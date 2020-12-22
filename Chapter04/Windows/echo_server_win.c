#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	char message[BUF_SIZE];
	int strLen;

	SOCKADDR_IN servAddr, clntAddr;
	int clntAddrSize;

	if (argc != 2) {
		printf("Usage %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error\n");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error\n");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error\n");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error\n");

	clntAddrSize = sizeof(clntAddr);

	for (int i = 0; i < 5; i++) {
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (hClntSock == INVALID_SOCKET)
			ErrorHandling("accept() error\n");
		else
			printf("Connected client %d\n", i + 1);

		// fault
		// when the string is long
		// the server split string and send diveded packets
		// the client does not receive some of data
		// because client recv() call only once
		while ((strLen = recv(hClntSock, message, BUF_SIZE, 0)) != 0)
			send(hClntSock, message, strLen, 0);

		closesocket(hClntSock);
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}