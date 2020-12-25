#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET servSock;
	char message[BUF_SIZE];
	int strLen;
	int clntAddrSize;
	SOCKADDR_IN servAddr, clntAddr;

	if (argc != 2) {
		printf("Usage %s <port>", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// UDP socket = SOCK_DGRAM
	servSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (servSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	while (1) {
		clntAddrSize = sizeof(clntAddr);
		
		// �����͸� ���� �� �۽��� ���� �ּ� ������ ����
		strLen = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (strLen == SOCKET_ERROR)
			ErrorHandling("recvfrom() error");

		// ������ ������ ������� �����͸� ������
		// ������ ���� UDP ���Ͽ��� ������ ������ ����
		// sendto()�� ���������ڴ� ������ �ּ� ���� ����ü ���� ũ��
		strLen = sendto(servSock, message, strLen, 0, (SOCKADDR*)&clntAddr, sizeof(clntAddr));
		if (strLen == SOCKET_ERROR)
			ErrorHandling("sendto() error");
	}
	closesocket(servSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
UDP

UDP�� TCP�� �ٸ���
���ἳ�� �� ���������� ����
������ �ŷڼ��� �����ϱ� ���� �帧��� ����.
������ �������� ũ�Ⱑ �۰�, �ŷڵ��� �߿伺�� ���� �������� �ȴٸ� UDP�� ������.

�������� ���� ���� �����Ͱ� �ٲ������� ������ �ٲ� ����� �ֱ� ������ TCP�� ����ؾ� ������
����������, ���� ������ �����Ͱ� �ణ �սǵǾ ��� ȭ���� �����̰ų�, ������ ���� ���̱� ������ ��뿡 ������
���������� ������ �����͸� �����;� �ϱ� ������ UDP�� ����.

TCP�� ����ϰ��� �ϴ� ���� 1��1�� ������ �����ؾ� ������
UDP�� ������ �ϳ��� �־ ���� ��ǻ�Ϳ� ����� �����ϴ�.

recvfrom() �Լ� ȣ��ÿ� ������ ������ �޾Ƽ� ����ϰ�
sendto()�Լ� ȣ��ÿ� ��ϵ� ������ ������� �����͸� �����ϰ� ������ �����Ѵ�.

*/