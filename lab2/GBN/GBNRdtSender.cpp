#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"

constexpr int SEQUENCESIZE = 8;
constexpr int WINDOWSIZE = 4;

GBNRdtSender::GBNRdtSender() :
	base(0), nextseqnum(0), N(WINDOWSIZE), TotalSequenceNumber(SEQUENCESIZE), waitingState(false), packetWaitingAck(new Packet[TotalSequenceNumber])
{
}


GBNRdtSender::~GBNRdtSender()
{
	delete[] packetWaitingAck;
}



bool GBNRdtSender::getWaitingState() {
	this->waitingState = !((nextseqnum + TotalSequenceNumber - base) % TotalSequenceNumber < N);
	return this->waitingState;
}




bool GBNRdtSender::send(const Message& message) {
	if (getWaitingState()) { //���ͷ����ڵȴ�ȷ��״̬
		cout << "���ͷ���������" << endl;
		return false;
	}
	
	this->packetWaitingAck[nextseqnum].acknum = -1; //���Ը��ֶ�
	this->packetWaitingAck[nextseqnum].seqnum = this->nextseqnum;
	this->packetWaitingAck[nextseqnum].checksum = 0;
	memcpy(this->packetWaitingAck[nextseqnum].payload, message.data, sizeof(message.data));
	this->packetWaitingAck[nextseqnum].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[nextseqnum]);
	cout << endl;
	cout << "����ǰ���ͷ����� �� ";
	outfile << endl;
	outfile << "����ǰ���ͷ����� �� ";
	printwindows();
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[nextseqnum]);
	if(nextseqnum == base)
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[nextseqnum].seqnum);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[nextseqnum]);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	this->nextseqnum = (this->nextseqnum + 1) % TotalSequenceNumber;
	cout << endl;
	cout << "���ͺ��ͷ����� �� ";
	outfile << endl;
	outfile << "���ͺ��ͷ����� �� ";
	printwindows();
	return true;
}

void GBNRdtSender::receive(const Packet& ackPkt) {
		//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		if (checkSum != ackPkt.checksum)
			pUtils->printPacket("����У��Ͳ���ȣ����յ���ack��", ackPkt);
		else if(ackPkt.acknum != (this->base + TotalSequenceNumber - 1) % TotalSequenceNumber)
		{
			
			this->base = (ackPkt.acknum + 1) % TotalSequenceNumber;
			if (this->base == this->nextseqnum)
				pns->stopTimer(SENDER, (this->base + TotalSequenceNumber - 1) % TotalSequenceNumber);
			else
			{
				pns->stopTimer(SENDER, (this->base + TotalSequenceNumber - 1) % TotalSequenceNumber);
				pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);

			}
			cout << endl;
			cout << "�յ�ack��" << ackPkt.acknum << "��������";
			outfile << endl;
			outfile << "�յ�ack��" << ackPkt.acknum << "��������";
			printwindows();
		}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	if (this->base == this->nextseqnum)
		return;
	cout << "���ͳ�ʱ���ش������ѷ��͵�δȷ�ϵķ���" << endl;
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	for (int i = this->base; i != nextseqnum; i = (i + 1) % TotalSequenceNumber)
	{
		pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[i]);
		pUtils->printPacket("��Ϊ��ʱ��Ҫ�ش��ķ���", packetWaitingAck[i]);
	}
}

void GBNRdtSender::printwindows()
{
	for (int i = 0; i < SEQUENCESIZE; i++)
	{
		if (i == this->base)
		{
			cout << '[';
			outfile << '[';
		}
		if (i == this->nextseqnum)
		{
			cout << '|';
			outfile << '|';
		}
		cout << i << ' ';
		outfile << i << ' ';
		if (i == (this->base + N) % SEQUENCESIZE - 1)
		{
			cout << ']';
			outfile << ']';
		}
	}
	cout << endl;
	outfile << endl;
}