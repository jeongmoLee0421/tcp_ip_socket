#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;
	TIMEVAL timeout;
	fd_set reads, cpyReads;

	int addrSize;
	int strLen, fdNum, i;
	char buf[BUF_SIZE];

	if (argc != 2) {
		printf("Usage %s <port>\n", argv[0]);
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

	FD_ZERO(&reads); // fd_set�� ���� �ʱ�ȭ
	FD_SET(hServSock, &reads); // fd_set�� ������ ServSock ���

	while (1) {
		// select �Լ��� ȣ��ǰ� ���� ��������� �ִ� �����ڵ��� 1�� ���������� ��������� ���� �����ڵ��� 0���� ����ȴ�.
		// 0���� ����� fd_set�� ������ �״�� ����ϰ� �ȴٸ� ���Ŀ� ������ �Ұ����ϴ�.
		// ������ �Ź� ������ �����ؼ� ����ϴ� ���̴�.
		cpyReads = reads;

		// select �Լ��� ȣ��ǰ� ���� TIMEVAL ����ü�� ������� Ÿ�Ӿƿ��� �߻��ϱ���� ���Ҵ� �ð����� �ٲ��.(���� 0, 0 ���)
		// ������ �Ź� Ÿ�Ӿƿ��ð��� �ʱ�ȭ ����� �Ѵ�.
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
			break;

		if (fdNum == 0) // ���� ����߿� ��ȯ�� ���� ���� Ÿ�Ӿƿ� �ð��� ������ �߻��ϸ� 0�� ��ȯ
			continue;

		for (i = 0; i < reads.fd_count; i++) { // ������ �߻����� �ʾҰ� Ÿ�Ӿƿ��� �߻����� �ʾҴٸ� ��𼱰� ��ȭ�� �־��ٴ� ��
			if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
				if (reads.fd_array[i] == hServSock) { // hServSock�� ���ŵ� �����Ͱ� �ִٴ� ���� Ŭ���̾�Ʈ���� �����û�� �־��ٴ� ��
					addrSize = sizeof(clntAddr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrSize);
					if (hClntSock == INVALID_SOCKET)
						ErrorHandling("accept() error");

					FD_SET(hClntSock, &reads); // ���� ����� Ŭ���̾�Ʈ ������ �����ϱ� ���ؼ� fd_set�� ������ ���
					printf("connected client: %d\n", hClntSock);
				}
				else { // ���������� �ƴϸ� Ŭ���̾�Ʈ ������ �ǹ��ϰ� �����͸� �������ش�.
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE, 0);
					if (strLen == 0) { // �������Ḧ �ǹ��ϴ� EOF���
						FD_CLR(reads.fd_array[i], &reads);
						closesocket(cpyReads.fd_array[i]);
						printf("closed client: %d\n", cpyReads.fd_array[i]);
					}
					else {
						send(reads.fd_array[i], buf, strLen, 0); // echo
					}
				}
			}
		}
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

/*
<��Ƽ���μ��� ������ ����>

�����û�� ���� �� ���� ���ο� ���μ����� �����ϴ� ���� ����� ���� ����� ���.
���� ���� ������ �䱸�ǰ� �ʿ��� �޸� ������ ���� ũ��.

���� ���μ������� ������ �޸� ������ �����ϱ� ������ �����͸� �ְ������ �־ �����ϴ�.



<��Ƽ�÷���>
�ϳ��� ���ä���� ���ؼ� �� �̻��� ������(�ñ׳�)�� �����ϴµ� ���Ǵ� ���

��Ƽ���μ����� Ŭ���̾�Ʈ���� �ڽ����μ����� �ʿ�������
��Ƽ�÷����� �θ� ���μ��� �ϳ��� �����ϸ� select()�Լ��� �����ڵ��� ��ȭ�� �����ϸ鼭 Ŭ���̾�Ʈ�� �����Ѵ�.
*/