#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"

extern ofstream outfile2;

class SRRdtReceiver :public RdtReceiver
{
private:
	int base;						//���շ������
	const int N;					//���ڴ�С
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	const int TotalSequenceNumber;	//��ŷ�Χ
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���
	Packet* packetcache;			//���շ�������
	int* packetstate;				//���շ����ڱ���״̬

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:

	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����
	void printwindows();				//��ӡ���շ�����
};

#endif