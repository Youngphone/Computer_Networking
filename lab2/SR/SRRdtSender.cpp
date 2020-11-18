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
	if (getWaitingState()) { //发送方处于等待确认状态
		cout << "发送方窗口已满!\n";
		return false;
	}

	this->packetWaitingAck[nextseqnum].acknum = -1; //忽略该字段
	this->packetWaitingAck[nextseqnum].seqnum = this->nextseqnum;
	this->packetWaitingAck[nextseqnum].checksum = 0;
	memcpy(this->packetWaitingAck[nextseqnum].payload, message.data, sizeof(message.data));
	this->packetWaitingAck[nextseqnum].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[nextseqnum]);
	cout << endl;
	cout << "发送前发送方窗口 ： ";
	outfile1 << endl;
	outfile1 << "发送前发送方窗口 ： ";
	printwindows();
	pUtils->printPacket("发送方发送报文", this->packetWaitingAck[nextseqnum]);
	pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[nextseqnum].seqnum);			//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[nextseqnum]);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	this->nextseqnum = (this->nextseqnum + 1) % TotalSequenceNumber;
	cout << endl;
	cout << "发送后发送方窗口 ： ";
	outfile1 << endl;
	outfile1 << "发送后发送方窗口 ： ";
	printwindows();
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum != ackPkt.checksum)
		pUtils->printPacket("数据校验和不相等，接收到的ack损坏", ackPkt);
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
				this->base = (this->base + 1) % SEQUENCESIZE;	//滑动窗口
			}
			cout << endl;
			cout << "接收到ACK：" << ackPkt.acknum << "后的窗口所示： ";
			outfile1 << endl;
			outfile1 << "接收到ACK：" << ackPkt.acknum << "后的窗口所示： ";
			printwindows();
		}
	}
}

void SRRdtSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	if (this->base == this->nextseqnum)
		return;
	cout << "发送超时，重传超时分组" << endl;
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[this->base]);
	pUtils->printPacket("因为超时将要重传的分组", packetWaitingAck[this->base]);
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