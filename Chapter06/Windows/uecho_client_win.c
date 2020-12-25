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

	// �Ϲ����� UDP = unconnected UDP
	// ������ sendto()ȣ��ÿ� �ü�� Ŀ�ο� ���ؼ� �ڵ����� ip�� port��ȣ�� �Ҵ�ް� ������ ������ �Ѵ�.
	// ������ ���� �Ŀ� UDP���Ͽ� ��ϵ� ������ �����.

	// connected UDP
	// ������ connect() �Լ� ȣ��� sock�� ���� �ּ������� �����ϰ� �Ǹ�
	// ������ ���� ����, ������ ����ؾ� �ϴ� ��쿡 ȿ�����̴�.
	// ������ �ּ� ������ �˰� �ֱ� ������ ���� ������ ���, ������ �Ź� ���� �ʾƵ� �Ǳ� �����̴�.
	connect(sock, (SOCKADDR*)&servAddr, sizeof(servAddr));

	// unconnected UDP��� sendto(), recvfrom() �Լ��� ���������
	// connected UDP��� send(), recv() �Լ��� ����� �� �ִ�.
	// ��? �̹� ���Ͽ� ������ �ּҰ� �Ҵ�Ǿ� �ֱ� �����̴�.

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