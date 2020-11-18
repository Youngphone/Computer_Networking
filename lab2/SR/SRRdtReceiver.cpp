#include "stdafx.h"
#include "Global.h"
#include "SRRdtReceiver.h"

constexpr int SEQUENCESIZE = 8;
constexpr int WINDOWSIZE = 4;

SRRdtReceiver::SRRdtReceiver() :base(0), N(WINDOWSIZE), expectSequenceNumberRcvd(0), TotalSequenceNumber(SEQUENCESIZE), 
								packetcache(new Packet[SEQUENCESIZE]),packetstate(new int [SEQUENCESIZE])
{
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	memset(packetstate, 0, sizeof(int) * SEQUENCESIZE);
}


SRRdtReceiver::~SRRdtReceiver()
{
	delete[] packetcache;
	delete[] packetstate;
}

void SRRdtReceiver::receive(const Packet& packet)
{
	if (packet.checksum != pUtils->calculateCheckSum(packet))//
	{
		pUtils->printPacket("接收到的报文错误：数据校验错误", packet);
		return;
	}

	bool IsInReceivingWindow;
	if (this->base < SEQUENCESIZE - N)
		IsInReceivingWindow = (packet.seqnum >= this->base) && (packet.seqnum < (this->base + N) % SEQUENCESIZE);
	else
		IsInReceivingWindow = (packet.seqnum >= this->base) || (packet.seqnum < (this->base + N) % SEQUENCESIZE);
	if (IsInReceivingWindow)
	{
		pUtils->printPacket("接收方成功接收发送方的包", packet);

		/*发送ack*/
		packetcache[packet.seqnum] = packet;//缓存收到的分组
		packetstate[packet.seqnum] = 1;
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.seqnum = -1;//忽略该字段
		memset(lastAckPkt.payload, '.', sizeof(lastAckPkt.payload));
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送ack包", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);

		cout << endl;
		cout << "接收方更新窗口前：";
		outfile2 << endl;
		outfile2 << "接收方更新窗口前：";
		printwindows();
		while (packetstate[this->base] == 1)
		{
			Message msg;
			memcpy(msg.data, packetcache[this->base].payload, sizeof(packetcache[this->base].payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			pUtils->printPacket("报文正确并通过模拟网络环境提交给应用层", packetcache[this->base]);
			packetstate[this->base] = 0;
			this->base = (this->base + 1) % SEQUENCESIZE;//窗口滑动
		}
		cout << endl;
		cout << "接收方更新窗口后：";
		outfile2 << endl;
		outfile2 << "接收方更新窗口后：";
		printwindows();
	}
	else
	{
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.seqnum = -1;//忽略该字段
		memset(lastAckPkt.payload, '.', sizeof(lastAckPkt.payload));
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方收到接收窗口外的报文，发送ack包", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
	}
}

void SRRdtReceiver::printwindows()
{
	for (int i = 0; i < SEQUENCESIZE; i++)
	{
		if (i == this->base)
		{
			cout << '[';
			outfile2 << '[';
		}
		if (packetstate[i] == 0)
		{
			cout << '\'' << i << '\'' << "  ";
			outfile2 << '\'' << i << '\'' << "  ";
		}
		else
		{
			cout << i << "  ";
			outfile2 << i << "  ";
		}
		if (i == (this->base + N) % SEQUENCESIZE - 1)
		{
			cout << ']';
			outfile2 << ']';
		}
	}
	cout << endl;
}