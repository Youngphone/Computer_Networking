// StopWait.cpp : 定义控制台应用程序的入口点。
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
	pns->setRunMode(0);  //VERBOS模式
	//pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("input.txt");
	pns->setOutputFile("output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	outfile1.close();
	outfile2.close();

	return 0;
}