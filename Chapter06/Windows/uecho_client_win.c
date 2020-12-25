#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN servAddr;
	int strLen;
	char message[BUF_SIZE];

	if (argc != 3) {
		printf("Usage %s <ip> <port>", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");
	
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
		ErrorHandling("UDP socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));

	// 일반적인 UDP = unconnected UDP
	// 원래는 sendto()호출시에 운영체제 커널에 의해서 자동으로 ip와 port번호를 할당받고 데이터 전송을 한다.
	// 데이터 전송 후에 UDP소켓에 등록된 정보를 지운다.

	// connected UDP
	// 하지만 connect() 함수 호출로 sock에 서버 주소정보를 저장하게 되면
	// 동일한 상대와 오래, 여러번 통신해야 하는 경우에 효율적이다.
	// 상대방의 주소 정보를 알고 있기 때문에 상대방 정보의 등록, 삭제를 매번 하지 않아도 되기 때문이다.
	connect(sock, (SOCKADDR*)&servAddr, sizeof(servAddr));

	// unconnected UDP라면 sendto(), recvfrom() 함수를 사용하지만
	// connected UDP라면 send(), recv() 함수를 사용할 수 있다.
	// 왜? 이미 소켓에 목적지 주소가 할당되어 있기 때문이다.

	while (1) {
		fputs("Insert message(q to quit): ", stdout);
		fgets(message, sizeof(message), stdin);
		if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
			break;

		strLen = send(sock, message, strlen(message), 0);
		if (strLen == SOCKET_ERROR)
			ErrorHandling("send() error");

		strLen = recv(sock, message, BUF_SIZE - 1, 0);
		if (strLen == SOCKET_ERROR)
			ErrorHandling("recv() error");

		message[strLen] = 0;
		printf("Message from server: %s", message);
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}