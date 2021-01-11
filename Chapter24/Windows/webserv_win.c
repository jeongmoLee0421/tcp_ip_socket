#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#define BUF_SIZE 2048
#define BUF_SMALL 100

unsigned WINAPI RequestHandler(void* arg);
char* ContentType(char* file);
void SendData(SOCKET sock, char* ct, char* fileName);
void SendErrorMSG(SOCKET sock);
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;

	HANDLE hThread;
	DWORD dwThreadID;
	int clntAddrSize;

	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	// ��û �� ����
	while (1) {
		clntAddrSize = sizeof(clntAddr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (hClntSock == INVALID_SOCKET)
			ErrorHandling("accept() error");
		printf("Connection Request: %s : %d\n", inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));

		// request�� �����ϱ� ���� ������ ���� �� ���� �帧 �̵�
		hThread = (HANDLE)_beginthreadex(NULL, 0, RequestHandler, (void*)hClntSock, 0, (unsigned*)&dwThreadID);
	}

	closesocket(hServSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI RequestHandler(void* arg) {
	SOCKET hClntSock = (SOCKET)arg;
	char buf[BUF_SIZE];
	char method[BUF_SMALL];
	char ct[BUF_SMALL];
	char fileName[BUF_SMALL];

	// request �޽��� ����
	recv(hClntSock, buf, BUF_SIZE, 0);

	// HTTP ��û���� Ȯ��
	if (strstr(buf, "HTTP/") == NULL) {
		SendErrorMSG(hClntSock);
		closesocket(hClntSock);
		return 1;
	}

	// ���� ������ " /" �����ڸ� ã�Ƽ� �ڸ� �Ŀ� method�� ����
	strcpy(method, strtok(buf, " /"));

	// GET ��� ��û���� Ȯ��
	if (strcmp(method, "GET"))
		SendErrorMSG(hClntSock);

	// ���� ���ڿ��� �ڸ��� ���ؼ� strtok �Լ��� ù��° ���ڷ� NULL�� �����ϸ� �ȴ�.
	strcpy(fileName, strtok(NULL, " /")); // ��û ���� �̸� Ȯ��
	strcpy(ct, ContentType(fileName)); // Content-type Ȯ��
	SendData(hClntSock, ct, fileName); // ����
	
	return 0;
}

void SendData(SOCKET sock, char* ct, char* fileName) {
	char protocol[] = "HTTP/1.0 200 OK\r\n";
	char servName[] = "Server:simple web server\r\n";
	char cntLen[] = "Content-length:2048\r\n";
	char cntType[BUF_SMALL];
	char buf[BUF_SIZE];
	FILE* sendFile;

	sprintf(cntType, "Content-type:%s\r\n\r\n", ct);
	if ((sendFile = fopen(fileName, "r")) == NULL) { // ���� ���� ����
		SendErrorMSG(sock);
		return;
	}

	// ��� ���� ����
	send(sock, protocol, strlen(protocol), 0);
	send(sock, servName, strlen(servName), 0);
	send(sock, cntLen, strlen(cntLen), 0);
	send(sock, cntType, strlen(cntType), 0);

	// ��û ������ ����
	while (fgets(buf, BUF_SIZE, sendFile) != NULL)
		send(sock, buf, strlen(buf), 0);

	// HTTP �������ݿ� ���ؼ� ���� �� ����
	closesocket(sock);
}

void SendErrorMSG(SOCKET sock) {
	char protocol[] = "HTTP/1.0 400 Bad Request\r\n";
	char servName[] = "Server:simple web server\r\n";
	char cntLen[] = "Content-length:2048\r\n";
	char cntType[] = "Content-type:text/html\r\n\r\n";
	char content[] = "<html><head><title>NETWORK</title></head>""<body><font size = +5><br>���� �߻�! ��û ���ϸ� �� ��û ��� Ȯ��!""</font></body></html>";

	send(sock, protocol, strlen(protocol), 0);
	send(sock, servName, strlen(servName), 0);
	send(sock, cntLen, strlen(cntLen), 0);
	send(sock, cntType, strlen(cntType), 0);
	send(sock, content, strlen(content), 0);
	
	closesocket(sock);
}

char* ContentType(char* file) { // Content-Type ����
	char extension[BUF_SMALL];
	char fileName[BUF_SMALL];
	
	strcpy(fileName, file); // index.html
	strtok(fileName, "."); // index
	strcpy(extension, strtok(NULL, ".")); // html

	if (!strcmp(extension, "html") || !strcmp(extension, "htm"))
		return "text/html";
	else
		return "text/plain";
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}