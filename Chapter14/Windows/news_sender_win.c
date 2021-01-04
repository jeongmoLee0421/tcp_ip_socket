#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h> // for IP_MULTICAST_TTL option

#define TTL 64
#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSendSock;
	SOCKADDR_IN mulAddr;
	int timeLive = TTL;
	FILE* fp;
	char buf[BUF_SIZE];
	int state;

	if (argc != 3) {
		printf("Usage: %s <group IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hSendSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (hSendSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&mulAddr, 0, sizeof(mulAddr));
	mulAddr.sin_family = AF_INET;
	mulAddr.sin_addr.s_addr = inet_addr(argv[1]);
	mulAddr.sin_port = htons(atoi(argv[2]));

	// 멀티캐스트 설정(Time to live)
	state = setsockopt(hSendSock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&timeLive, sizeof(timeLive));
	if (state == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	if ((fp = fopen("news.txt", "r")) == NULL)
		ErrorHandling("fopen() error");

	while (1) {
		if (fgets(buf, BUF_SIZE, fp) == NULL)
			break;

		sendto(hSendSock, buf, strlen(buf), 0, (SOCKADDR*)&mulAddr, sizeof(mulAddr));
		Sleep(2000);
	}

	fclose(fp);
	closesocket(hSendSock);
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}