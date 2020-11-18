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
	if (getWaitingState()) { //发送方处于等待确认状态
		cout << "发送方窗口已满" <<endl;
		return false;
	}

	this->packetWaitingAck[nextseqnum].acknum = -1; //忽略该字段
	this->packetWaitingAck[nextseqnum].seqnum = this->nextseqnum;
	this->packetWaitingAck[nextseqnum].checksum = 0;
	memcpy(this->packetWaitingAck[nextseqnum].payload, message.data, sizeof(message.data));
	this->packetWaitingAck[nextseqnum].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[nextseqnum]);
	cout << endl;
	cout << "发送前发送方窗口 ： ";
	outfile3 << endl;
	outfile3 << "发送前发送方窗口 ： ";
	printwindows();
	pUtils->printPacket("发送方发送报文", this->packetWaitingAck[nextseqnum]);
	if (nextseqnum == base)
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[nextseqnum].seqnum);			//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[nextseqnum]);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	this->nextseqnum = (this->nextseqnum + 1) % TotalSequenceNumber;
	cout << endl;
	cout << "发送后发送方窗口 ： ";
	outfile3 << endl;
	outfile3 << "发送后发送方窗口 ： ";
	printwindows();
	return true;
}

void TCPRdtSender::receive(const Packet& ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum != ackPkt.checksum)
		pUtils->printPacket("数据校验和不相等，接收到的ack损坏", ackPkt);
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
			cout << "收到ack：" << ackPkt.acknum << "滑动窗口";
			outfile3 << endl;
			outfile3 << "收到ack：" << ackPkt.acknum << "滑动窗口";
			printwindows();
		}
		else
		{
			//快速重传
			if (this->lastAck != ackPkt.acknum)
			{
				this->lastAck = ackPkt.acknum;
				this->dupTimes = 1;
			}
			else
				this->dupTimes++;
			if (this->dupTimes == 3)	//连续收到三个冗余ack，快速重传
			{
				this->dupTimes = 0;
				pUtils->printPacket("接收到三个冗余的ACK", ackPkt);
				cout << "发送方收到一个报文的三个冗余ack，快速重传" << endl;
				//重启计时器
				pns->stopTimer(SENDER, this->base);
				pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
				pUtils->printPacket("快速重传的分组", packetWaitingAck[base]);
				pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[base]);
			}
		}
	}
}

void TCPRdtSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	if (this->base == this->nextseqnum)
		return;
	cout << "发送超时，重传base分组" << endl;
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	pUtils->printPacket("因为超时将要重传的分组", packetWaitingAck[base]);
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