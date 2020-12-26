#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;
	char buf[BUF_SIZE];
	FILE* fp;
	int readCnt;
	int clntAddrSize;

	if (argc != 2) {
		printf("Usage %s <port>", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	fp = fopen("file_server_win.c", "rb");
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

	clntAddrSize = sizeof(clntAddr);
	hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);

	while (1) {
		readCnt = fread((void*)buf, 1, BUF_SIZE, fp);

		// BUF_SIZE���� ���� ���� �о��ٸ�
		// ������ ���� �ǹ��Ѵ�.
		if (readCnt < BUF_SIZE) {

			// buf ��ü�� char���̱� ������ (char*)������ ĳ�����ؼ� �ּ�(&)�� �������� �ʰ�
			// buf(�迭�� �̸��� ù �ּ�)�� �����ص� ���۵ȴ�.
			send(hClntSock, (char*)&buf, readCnt, 0);
			break;
		}

		// send()�� �ι�° ���ڰ� ������ �����͸� �����ϰ� �ִ� ������ �ּ� ���ε�
		// const char* ���̴�.
		send(hClntSock, (char*)&buf, BUF_SIZE, 0);
	}

	// hClntSock�� ��½�Ʈ���� �����Ѵ�.
	shutdown(hClntSock, SD_SEND);

	// hClntSock�� �Է½�Ʈ���� ����ִ� �����̱� ������
	// ���κ��� ���۵� �޽����� ���� �� �ִ�.
	recv(hClntSock, (char*)buf, BUF_SIZE, 0);
	printf("Message from client: %s\n", buf);

	fclose(fp);
	closesocket(hClntSock);
	closesocket(hServSock);
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
Half-close

���������� close(), �����쿡�� closesocket()�� �������Ḧ �ǹ��Ѵ�.
�Է� ��Ʈ��, ��� ��Ʈ���� ��� �����ϰ� ������ �޸𸮿��� �����ϴ� ���̴�.
�� �Լ��� ����ǰ� ���� �� �̻� ��/������ �Ұ��ϴ�.

���� �ϳ� ����
Ŭ���̾�Ʈ�� �����κ��� � �����͸� �޾Ұ� �׿� ���� ������ �ؾ��ϴ� ��Ȳ�̴�.

������ �ڽ��� ���� �����͸� ��� �����ϰ� closesocket()�� �������Ḧ �ع�����
Ŭ���̾�Ʈ�� ������ ���� ���� ����.

�� �� ����� �� �ִ� ���� shutdown()�̴�.
�� �Լ��� �ι�° ���ڷ� ������ ��Ʈ���� ������ �� �ִµ�
�Է� ��Ʈ��, ��� ��Ʈ��, �� ��Ʈ�� ��� �̷��� �������� �ִ�.

������ �����͸� ��� �����ϰ� ��� ��Ʈ���� �ݰ� �Է½�Ʈ���� ����ΰ� �ִٸ�
Ŭ���̾�Ʈ�� ������ ���� �� �ִ�.

�ް��� closesocket()���� �������Ḧ �ϸ� �ȴ�.
*/