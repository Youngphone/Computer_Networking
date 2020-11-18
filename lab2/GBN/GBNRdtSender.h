#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"

extern ofstream outfile;

class GBNRdtSender :public RdtSender
{
private:
	int base;						//基序号
	int nextseqnum;					//下一个序号
	const int TotalSequenceNumber;	//序号范围 
	const int N;					//窗口大小
	bool waitingState;				//是否处于等待Ack的状态
	Packet* packetWaitingAck;		//已发送并等待Ack的数据包

public:

	bool getWaitingState();
	bool send(const Message& message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet& ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用
	void printwindows();

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif