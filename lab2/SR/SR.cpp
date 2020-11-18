// StopWait.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "SRRdtSender.h"
#include "SRRdtReceiver.h"

ofstream outfile1, outfile2;

int main3(int argc, char* argv[])
{
	RdtSender* ps = new SRRdtSender();
	RdtReceiver* pr = new SRRdtReceiver();
	outfile1.open("../SRSendWindos.txt", ios::out | ios::trunc);
	outfile2.open("../SRRecvWindos.txt", ios::out | ios::trunc);
	pns->setRunMode(0);  //VERBOSģʽ
	//pns->setRunMode(1);  //����ģʽ
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("input.txt");
	pns->setOutputFile("output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	outfile1.close();
	outfile2.close();

	return 0;
}