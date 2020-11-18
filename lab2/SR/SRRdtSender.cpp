#include "stdafx.h"
#include "Global.h"
#include "SRRdtSender.h"

constexpr int SEQUENCESIZE = 8;
constexpr int WINDOWSIZE = 4;

SRRdtSender::SRRdtSender() :
	base(0), nextseqnum(0), N(WINDOWSIZE), TotalSequenceNumber(SEQUENCESIZE), waitingState(false), 
	packetWaitingAck(new Packet[TotalSequenceNumber]), packetstate(new int[SEQUENCESIZE])
{
	memset(packetstate, 0, sizeof(int) * SEQUENCESIZE);
}


SRRdtSender::~SRRdtSender()
{
	delete[] packetWaitingAck;
	delete[] packetstate;
}

bool SRRdtSender::getWaitingState() {
	this->waitingState = !((nextseqnum + TotalSequenceNumber - base) % TotalSequenceNumber < N);
	return this->waitingState;
}




bool SRRdtSender::send(const Message& message) {
	if (getWaitingState()) { //���ͷ����ڵȴ�ȷ��״̬
		cout << "���ͷ���������!\n";
		return false;
	}

	this->packetWaitingAck[nextseqnum].acknum = -1; //���Ը��ֶ�
	this->packetWaitingAck[nextseqnum].seqnum = this->nextseqnum;
	this->packetWaitingAck[nextseqnum].checksum = 0;
	memcpy(this->packetWaitingAck[nextseqnum].payload, message.data, sizeof(message.data));
	this->packetWaitingAck[nextseqnum].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[nextseqnum]);
	cout << endl;
	cout << "����ǰ���ͷ����� �� ";
	outfile1 << endl;
	outfile1 << "����ǰ���ͷ����� �� ";
	printwindows();
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[nextseqnum]);
	pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[nextseqnum].seqnum);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[nextseqnum]);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	this->nextseqnum = (this->nextseqnum + 1) % TotalSequenceNumber;
	cout << endl;
	cout << "���ͺ��ͷ����� �� ";
	outfile1 << endl;
	outfile1 << "���ͺ��ͷ����� �� ";
	printwindows();
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum != ackPkt.checksum)
		pUtils->printPacket("����У��Ͳ���ȣ����յ���ack��", ackPkt);
	else
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
		bool IsInSendingWindow;
		if (this->base < (this->base + N) % SEQUENCESIZE)
			IsInSendingWindow = (ackPkt.acknum >= this->base) && (ackPkt.acknum < (this->base + N) % SEQUENCESIZE);
		else
			IsInSendingWindow = (ackPkt.acknum >= this->base) || (ackPkt.acknum < (this->base + N) % SEQUENCESIZE);
		if (IsInSendingWindow)
		{
			packetstate[ackPkt.acknum] = 1;
			while (packetstate[this->base] == 1)
			{
				packetstate[this->base] = 0;
				this->base = (this->base + 1) % SEQUENCESIZE;	//��������
			}
			cout << endl;
			cout << "���յ�ACK��" << ackPkt.acknum << "��Ĵ�����ʾ�� ";
			outfile1 << endl;
			outfile1 << "���յ�ACK��" << ackPkt.acknum << "��Ĵ�����ʾ�� ";
			printwindows();
		}
	}
}

void SRRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	if (this->base == this->nextseqnum)
		return;
	cout << "���ͳ�ʱ���ش���ʱ����" << endl;
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[this->base]);
	pUtils->printPacket("��Ϊ��ʱ��Ҫ�ش��ķ���", packetWaitingAck[this->base]);
}

void SRRdtSender::printwindows()
{
	for (int i = 0; i < SEQUENCESIZE; i++)
	{
		if (i == this->base)
		{
			cout << '[';
			outfile1 << '[';
		}
		if (i == this->nextseqnum)
		{
			cout << '|';
			outfile1 << '|';
		}
		cout << i << ' ';
		outfile1 << i << ' ';
		if (i == (this->base + N) % SEQUENCESIZE - 1)
		{
			cout << ']';
			outfile1 << ']';
		}
	}
	cout << endl;
	outfile1 << endl;
}