#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "winsock2.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <ctime>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

// �ļ����� content-type
struct doc_type {
	const char* suffix;
	const char* type;
};
struct doc_type fileType[] = {{"html", "text/html"},
							  {"gif", "imag/gif"},
							  {"jpeg", "image/jpeg"},
							  {"jpg", "image/jpeg"},
							  {"ico", "image/x-icon"},
							  {"mp4", "video/mp4"},
							  {"flv", "video/x-flv"},
							  {NULL, NULL} };

//ȫ�ֱ���
char IP[20];
char Home[256];
int Port = 0;
//������Ӧ�ײ�
char http_res_200_ok[] ="HTTP/1.1 200 OK \r\nDate:%s\r\n"
"Server:YoungPhone's Server<0.1>\r\nAccept-Ranges:bytes\r\n"
"Content-Length:%d\r\nConnection:close\r\n"
"Content-Type:%s\r\n\r\n";

char http_res_404_notfound[] = "HTTP/1.1 404 NOT FOUND\r\nDate:%s\r\n"
"Server:YoungPhone's Server<0.1>\r\nAccept-Ranges:bytes\r\n"
"Content-Length:%d\r\nConnection:close\r\n"
"Content-Type:%s\r\n\r\n";

char http_res_400_badrequest[] = " HTTP / 1.1 404 NOT FOUND\r\nDate: % s\r\n"
"Server:YoungPhone's Server<0.1>\r\n\r\n";

//��������
void parsing_http_request(char* buf, int len, char* content_type, char* filename);	//����������
int send_http_response(SOCKET soc, char* buf, int len);		//������Ӧ����
int send_http_file(SOCKET soc, FILE* fp, char* head, char* content_type);	//����������ļ�
void getcfg();

//��ȡ�����ļ�
void getcfg()
{
	fstream fp;
	fp.open("config.txt", ios::in);
	string str, key, value;
	regex pattern("([\\S]*)(\\s|)=(\\s|)([\\S]*)");
	smatch result;
	while (getline(fp, str)) 
	{
		if (regex_search(str, result, pattern)) 
		{
			key = result.str(1);
			value = result.str(4);
			if (key == "ip") 
			{
				strcpy(IP, value.c_str());
			}
			else if (key == "port") 
			{
				Port = stoi(value);
			}
			else if (key == "home")
			{
				strcpy(Home, value.c_str());
			}
		};
	}
}

//ͨ��������ʽ�����������Ի���ļ�����content_type
void parsing_http_request(char* buf, int len, char* content_type, char* filename) 
{
	string temp_buf = buf;
	string PATH = "Web\\";
	regex pattern("GET /(.*) ");
	smatch result;
	// �õ��ļ���
	if (regex_search(temp_buf, result, pattern)) {
		string temp_filename = result.str(1);
		// ��������ļ���Ϊ��,��ȡ�ļ���index.html
		if (temp_filename == "")
			temp_filename = Home;
		temp_filename = PATH + temp_filename;
		strcpy(filename, temp_filename.c_str());

		// �õ���׺
		string suffix = temp_filename.substr(temp_filename.find_last_of('.') + 1);
		int i = 0;
		while (fileType[i].suffix != NULL) {
			if (fileType[i].suffix == suffix) {
				strcpy(content_type, fileType[i].type);
				break;
			}
			i++;
		}
		if (fileType[i].suffix == NULL) {
			content_type = NULL;
		}
	}
}

//������Ӧ����
int send_http_response(SOCKET soc, char* buf, int len)
{
	char filename[256];
	char content_type[100];
	FILE* res_file;

	memset(filename, '\0', 256);
	memset(content_type, '\0', 100);
	//�������Ļ���ļ�����content_type
	parsing_http_request(buf, len, content_type, filename);

	int state = 0;
	//���ļ�
	res_file = fopen(filename, "rb");
	if (!res_file)
	{
		cout << "file: " << filename << " not found!" << endl;
		//����404
		memset(filename, '\0', 256);
		memset(content_type, '\0', 100);
		strcpy(filename, "Web\\err404.html");
		strcpy(content_type, "text/html");
		res_file = fopen(filename, "rb");
		state = send_http_file(soc, res_file, http_res_404_notfound, content_type);
		fclose(res_file);
		if (state == -1)
			return -1;
		return 0;
	}
	state = send_http_file(soc, res_file, http_res_200_ok, content_type);
	fclose(res_file);
	return state;
}

//����������ļ�
int send_http_file(SOCKET soc, FILE* fp, char* head, char* content_type)
{
	int file_len, hdr_len, send_len;
	char http_header[1024];
	char dt[26];
	memset(http_header, '\0', 1024);

	//�����ļ���С
	fseek(fp, 0, SEEK_END);
	file_len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// ���ڵ�ǰϵͳ�ĵ�ǰ����/ʱ��
	time_t now = time(0);
	// �� now ת��Ϊ�ַ�����ʽ
	strcpy(dt, ctime(&now));
	dt[strlen(ctime(&now)) - 1] = '\0';

	//������Ӧ����ͷ��
	hdr_len = sprintf(http_header, head, dt, file_len, content_type);

	//���ͱ���ͷ��
	send_len = send(soc, http_header, hdr_len, 0);
	if (send_len == SOCKET_ERROR)
	{
		cout << "send response failed, error:" << WSAGetLastError() << endl;
		return -1;
	}

	//����������ļ�
	char readbuf[1024];
	int readlen;
	do
	{
		readlen = fread(readbuf, 1, 1024, fp);
		if (readlen > 0)
		{
			send_len = send(soc, readbuf, readlen, 0);
			if (send_len == SOCKET_ERROR)
			{
				cout << "send response failed, error:" << WSAGetLastError() << endl;
				return -1;
			}
		}
	} while (readlen == 1024);
	return 1;
}

int main()
{
	WSADATA wsaData;
	if (WSAStartup(0x0202, &wsaData) != 0)
	{
		cout << "Winsock startup failed with error!\n" << endl;
		return 0;
	}
	if (wsaData.wVersion != 0x0202)
	{
		cout << "Winsock version is not correct!\n" << endl;
		WSACleanup();
		return 0;
	}
	cout << "Winsock startup OK!\n" << endl;


	//�����׽���
	SOCKET srvsocket = socket(AF_INET, SOCK_STREAM, 0);
	if (srvsocket == INVALID_SOCKET)
	{
		cout << "Socket create failed!\n" << endl;
		WSACleanup();
		return 0;
	}
	cout << "Socket create OK!\n" << endl;

	//��ȡ�����ļ�
	getcfg();
	//�󶨶˿ں�ip
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(Port);
	sin.sin_addr.S_un.S_addr = inet_addr(IP);
	int rtn = bind(srvsocket, (LPSOCKADDR)&sin, sizeof(sin));
	if (rtn == SOCKET_ERROR)
	{
		cout << "Socket bind error!\n" << endl;
		WSACleanup();
		return 0;
	}
	cout << "Socket bind OK!\n" << endl;

	//��ʼ����
	rtn = listen(srvsocket, 5);
	if (rtn == SOCKET_ERROR)
	{
		cout << "Socket listen error!\n" << endl;
		WSACleanup();
		return 0;
	}
	cout << "Socket listen OK!\n" << endl;


	SOCKET clientsocket;
	sockaddr_in clientAddr;
	int addrLen = sizeof(clientAddr);
	char recvBuf[4096];
	while (true)
	{
		cout << "Waiting for client connection and data..." << endl;
		clientsocket = accept(srvsocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientsocket == INVALID_SOCKET)
		{
			cout << "accept error!\n" << endl;
			continue;
		}
		cout << "\nSocket listen one client request from IP: " << inet_ntoa(clientAddr.sin_addr) << "	Port: " << ntohs(clientAddr.sin_port) << endl;
		memset(recvBuf, '\0', 4096);
		rtn = recv(clientsocket, recvBuf, 1024, 0);
		if (rtn > 0)
		{
			printf("Received %d bytes from client:\n%s", rtn, recvBuf);
		}
		else
		{
			cout << "Response: 400 Bad request!\n";
			closesocket(clientsocket);
			continue;
		}
		int res = send_http_response(clientsocket, recvBuf, rtn);
		if (res == 1)
			cout << "Response: 200 OK!" << endl;
		else if (res == 0)
			cout << "Response: 404 NOT FOUND!" << endl;
		closesocket(clientsocket);
	}
	closesocket(srvsocket);
	WSACleanup();
	return 1;
}