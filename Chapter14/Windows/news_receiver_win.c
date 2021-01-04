#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h> // for struct ip_mreq

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hRecvSock;
	SOCKADDR_IN addr;
	struct ip_mreq joinAddr;
	char buf[BUF_SIZE];
	int strLen;

	if (argc != 3) {
		printf("Usage: %s <group IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hRecvSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (hRecvSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[2]));

	// ���Ͽ� ip�ּҿ� port��ȣ ���
	if (bind(hRecvSock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	joinAddr.imr_multiaddr.s_addr = inet_addr(argv[1]);
	joinAddr.imr_interface.s_addr = htonl(INADDR_ANY);

	// ���Ͽ� �׷� ip ����
	if (setsockopt(hRecvSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&joinAddr, sizeof(joinAddr)) == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	while (1) {
		strLen = recvfrom(hRecvSock, buf, BUF_SIZE - 1, 0, NULL, 0);
		if (strLen < 0)
			break;

		buf[strLen] = 0;
		fputs(buf, stdout);
	}

	closesocket(hRecvSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
��Ƽĳ��Ʈ�� ��Ʈ��ũ �޶� �׷쿡 ���ԵǾ� �ִٸ� �����͸� �޴´�.
��ε�ĳ��Ʈ�� ������ ��Ʈ��ũ�� �ִ� ��� ȣ��Ʈ�� �����͸� �޴´�.

Directed ��ε�ĳ��Ʈ�� Ư�� ��Ʈ��ũ�� ���ؼ� �����͸� ������ �� �ִµ� ��Ʈ��ũ �ڿ� 255�� �ٿ��ָ� �ȴ�.
���� �� vmnet8���� ��Ʈ��ũ �ּҰ� 192.168.217.1�ε� 192.168.217.255�� �����ϸ� �ȴ�.

Local ��ε�ĳ��Ʈ�� �����ϴ� ȣ��Ʈ�� ��Ʈ��ũ�� �ִ� ��� ȣ��Ʈ�� �����͸� �޴´�.
255.255.255.255 ��� ��ӵ� ip�ּҸ� ����Ѵ�.

��Ƽĳ��Ʈ�� Ʈ���� ���鿡���� �������ε�
tcp�� ��� 1000���� �����͸� �����ٰ� ���� �� ��ΰ� ������ �ٸ����� ������� �ʰ� 1000���� �������Ѵ�.
������ udp��� ��Ƽĳ��Ʈ�� �ѹ��� �����ϸ� ����Ͱ� �����͸� �����ؼ� ���� ��ο� �����ϴ� ȣ��Ʈ�鿡�� �����ϱ� ������ ȿ�����̴�.
*/