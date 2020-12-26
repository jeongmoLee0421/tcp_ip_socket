#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	int hSocket;
	SOCKADDR_IN servAddr;
	int readCnt;
	FILE* fp;
	char buf[BUF_SIZE];

	if (argc != 3) {
		printf("Usage %s <ip> <port>", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	fp = fopen("receive.dat", "wb");
	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));

	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	while ((readCnt = recv(hSocket, buf, BUF_SIZE, 0)) != 0)
		fwrite((void*)buf, 1, readCnt, fp);

	puts("Received file data");

	// 상대방(서버)의 출력 스트림은 종료되었지만 입력 스트림은 살아있기 때문에
	// 데이터를 보내면 수신할 수 있다.
	send(hSocket, "Thank you", 10, 0);
	
	fclose(fp);
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}