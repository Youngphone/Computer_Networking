#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"

extern ofstream outfile2;

class SRRdtReceiver :public RdtReceiver
{
private:
	int base;						//接收方基序号
	const int N;					//窗口大小
	int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	const int TotalSequenceNumber;	//序号范围
	Packet lastAckPkt;				//上次发送的确认报文
	Packet* packetcache;			//接收方缓存区
	int* packetstate;				//接收方窗口报文状态

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:

	void receive(const Packet& packet);	//接收报文，将被NetworkService调用
	void printwindows();				//打印接收方窗口
};

#endif