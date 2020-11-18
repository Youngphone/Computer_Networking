#include "stdafx.h"
#include "Global.h"
#include "SRRdtReceiver.h"

constexpr int SEQUENCESIZE = 8;
constexpr int WINDOWSIZE = 4;

SRRdtReceiver::SRRdtReceiver() :base(0), N(WINDOWSIZE), expectSequenceNumberRcvd(0), TotalSequenceNumber(SEQUENCESIZE), 
								packetcache(new Packet[SEQUENCESIZE]),packetstate(new int [SEQUENCESIZE])
{
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
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
		pUtils->printPacket("���յ��ı��Ĵ�������У�����", packet);
		return;
	}

	bool IsInReceivingWindow;
	if (this->base < SEQUENCESIZE - N)
		IsInReceivingWindow = (packet.seqnum >= this->base) && (packet.seqnum < (this->base + N) % SEQUENCESIZE);
	else
		IsInReceivingWindow = (packet.seqnum >= this->base) || (packet.seqnum < (this->base + N) % SEQUENCESIZE);
	if (IsInReceivingWindow)
	{
		pUtils->printPacket("���շ��ɹ����շ��ͷ��İ�", packet);

		/*����ack*/
		packetcache[packet.seqnum] = packet;//�����յ��ķ���
		packetstate[packet.seqnum] = 1;
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.seqnum = -1;//���Ը��ֶ�
		memset(lastAckPkt.payload, '.', sizeof(lastAckPkt.payload));
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ack��", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);

		cout << endl;
		cout << "���շ����´���ǰ��";
		outfile2 << endl;
		outfile2 << "���շ����´���ǰ��";
		printwindows();
		while (packetstate[this->base] == 1)
		{
			Message msg;
			memcpy(msg.data, packetcache[this->base].payload, sizeof(packetcache[this->base].payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			pUtils->printPacket("������ȷ��ͨ��ģ�����绷���ύ��Ӧ�ò�", packetcache[this->base]);
			packetstate[this->base] = 0;
			this->base = (this->base + 1) % SEQUENCESIZE;//���ڻ���
		}
		cout << endl;
		cout << "���շ����´��ں�";
		outfile2 << endl;
		outfile2 << "���շ����´��ں�";
		printwindows();
	}
	else
	{
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.seqnum = -1;//���Ը��ֶ�
		memset(lastAckPkt.payload, '.', sizeof(lastAckPkt.payload));
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ��յ����մ�����ı��ģ�����ack��", lastAckPkt);
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