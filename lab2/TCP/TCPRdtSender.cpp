#include "stdafx.h"
#include "Global.h"
#include "TCPRdtSender.h"

constexpr int SEQUENCESIZE = 8;
constexpr int WINDOWSIZE = 4;

TCPRdtSender::TCPRdtSender() :
	base(0), nextseqnum(0), N(WINDOWSIZE), TotalSequenceNumber(SEQUENCESIZE), waitingState(false), 
	packetWaitingAck(new Packet[TotalSequenceNumber]), lastAck(0), dupTimes(0)
{
}


TCPRdtSender::~TCPRdtSender()
{
	delete[] packetWaitingAck;
}



bool TCPRdtSender::getWaitingState() {
	this->waitingState = !((nextseqnum + TotalSequenceNumber - base) % TotalSequenceNumber < N);
	return this->waitingState;
}




bool TCPRdtSender::send(const Message& message) {
	if (getWaitingState()) { //���ͷ����ڵȴ�ȷ��״̬
		cout << "���ͷ���������" <<endl;
		return false;
	}

	this->packetWaitingAck[nextseqnum].acknum = -1; //���Ը��ֶ�
	this->packetWaitingAck[nextseqnum].seqnum = this->nextseqnum;
	this->packetWaitingAck[nextseqnum].checksum = 0;
	memcpy(this->packetWaitingAck[nextseqnum].payload, message.data, sizeof(message.data));
	this->packetWaitingAck[nextseqnum].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[nextseqnum]);
	cout << endl;
	cout << "����ǰ���ͷ����� �� ";
	outfile3 << endl;
	outfile3 << "����ǰ���ͷ����� �� ";
	printwindows();
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[nextseqnum]);
	if (nextseqnum == base)
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[nextseqnum].seqnum);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[nextseqnum]);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	this->nextseqnum = (this->nextseqnum + 1) % TotalSequenceNumber;
	cout << endl;
	cout << "���ͺ��ͷ����� �� ";
	outfile3 << endl;
	outfile3 << "���ͺ��ͷ����� �� ";
	printwindows();
	return true;
}

void TCPRdtSender::receive(const Packet& ackPkt) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum != ackPkt.checksum)
		pUtils->printPacket("����У��Ͳ���ȣ����յ���ack��", ackPkt);
	else
	{
		bool IsInSendingWindow;
		if (this->base < (this->base + N) % SEQUENCESIZE)
			IsInSendingWindow = (ackPkt.acknum >= this->base) && (ackPkt.acknum < (this->base + N) % SEQUENCESIZE);
		else
			IsInSendingWindow = (ackPkt.acknum >= this->base) || (ackPkt.acknum < (this->base + N) % SEQUENCESIZE);
		if (IsInSendingWindow)
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
			outfile3 << endl;
			outfile3 << "�յ�ack��" << ackPkt.acknum << "��������";
			printwindows();
		}
		else
		{
			//�����ش�
			if (this->lastAck != ackPkt.acknum)
			{
				this->lastAck = ackPkt.acknum;
				this->dupTimes = 1;
			}
			else
				this->dupTimes++;
			if (this->dupTimes == 3)	//�����յ���������ack�������ش�
			{
				this->dupTimes = 0;
				pUtils->printPacket("���յ����������ACK", ackPkt);
				cout << "���ͷ��յ�һ�����ĵ���������ack�������ش�" << endl;
				//������ʱ��
				pns->stopTimer(SENDER, this->base);
				pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
				pUtils->printPacket("�����ش��ķ���", packetWaitingAck[base]);
				pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[base]);
			}
		}
	}
}

void TCPRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	if (this->base == this->nextseqnum)
		return;
	cout << "���ͳ�ʱ���ش�base����" << endl;
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	pUtils->printPacket("��Ϊ��ʱ��Ҫ�ش��ķ���", packetWaitingAck[base]);
	pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[base]);
}

void TCPRdtSender::printwindows()
{
	for (int i = 0; i < SEQUENCESIZE; i++)
	{
		if (i == this->base)
		{
			cout << '[';
			outfile3 << '[';
		}
		if (i == this->nextseqnum)
		{
			cout << '|';
			outfile3 << '|';
		}
		cout << i << ' ';
		outfile3 << i << ' ';
		if (i == (this->base + N) % SEQUENCESIZE - 1)
		{
			cout << ']';
			outfile3 << ']';
		}
	}
	cout << endl;
	outfile3 << endl;
}