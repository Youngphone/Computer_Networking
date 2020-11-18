#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"

extern ofstream outfile3;

class TCPRdtSender :public RdtSender
{
private:
	int base;						//�����
	int nextseqnum;					//��һ�����
	const int TotalSequenceNumber;	//��ŷ�Χ 
	const int N;					//���ڴ�С
	bool waitingState;				//�Ƿ��ڵȴ�Ack��״̬
	Packet* packetWaitingAck;		//�ѷ��Ͳ��ȴ�Ack�����ݰ�
	int lastAck;
	int dupTimes;

public:

	bool getWaitingState();
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����
	void printwindows();								//��ӡ��������

public:
	TCPRdtSender();
	virtual ~TCPRdtSender();
};

#endif