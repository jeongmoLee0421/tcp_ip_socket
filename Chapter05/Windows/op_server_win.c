#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
#define OPSZ 4
void ErrorHandling(char* message);
int calculate(int opnum, int opnds[], char operator);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;
	int clntAddrSize;
	int result, opndCnt, i;
	int recvCnt, recvLen;
	char opinfo[BUF_SIZE];

	if (argc != 2) {
		printf("Usage %s <port>", argv[0]);
		exit(1);
	}

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	hServSock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");
	
	clntAddrSize = sizeof(clntAddr);

	for (i = 0; i < 3; i++) {
		opndCnt = 0;
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (hClntSock == INVALID_SOCKET)
			ErrorHandling("accept() error");

		// TCPƯ���� �Է¹��ۿ� �����Ͱ� �󸶳� �ִ��� ����� ���Ǵ�� ���� �� �ִ�.
		// �Է¹��ۿ� �ִ� �������߿� ���� ���ο� 1byte�� �ǿ����� �����̱� ������
		// 1byte�� �޾Ƽ� opndCnt(int��)�� ����
		recv(hClntSock, &opndCnt, 1, 0);

		recvLen = 0;

		// �� ���� ���̴� �ǿ����� byte�� + ������(1byte)
		while ((opndCnt * OPSZ + 1) > recvLen) {
			recvCnt = recv(hClntSock, &opinfo[recvLen], BUF_SIZE - 1, 0);
			recvLen += recvCnt;
		}

		// opinfo�� char�� �迭������ (int*)�� ĳ�����ϰ� �Ǹ�
		// 4byte�� ��� ������ �迭ó�� ���
		// ���� �������� �� ����(recvLen) - 1�� ������ �ε����� �̴� �����ڸ� �ǹ��Ѵ�.
		result = calculate(opndCnt, (int*)opinfo, opinfo[recvLen - 1]);
		send(hClntSock, &result, sizeof(result), 0);
		closesocket(hClntSock);
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

int calculate(int opnum, int opnds[], char op) {
	int result = opnds[0], i;

	switch (op) {
	case '+':
		for (i = 1; i < opnum; i++) result += opnds[i];
		break;

	case '-':
		for (i = 1; i < opnum; i++) result -= opnds[i];
		break;

	case '*':
		for (i = 1; i < opnum; i++) result *= opnds[i];
		break;
	}
	return result;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
TCP�� ��������

TCP ���Ͽ��� �����͸� �ְ� ���� ����
���� �����͸� ������ �޾Ұ� ��𼭺��� ������ �ȴٴ� ������ �׻� �Ѵ�.

SEQ�� 1000���� �ϰ� 100byte�� �������� ��
�����ڴ� ���� �޾Ҵٸ� 1000 + 100(���۵� ����Ʈ ũ��) + 1 byte��
ACK�� 1101�� �����Ѵ�.
�׷� �۽��ڴ� ����� ���ٰ� Ȯ���ϰ� ���� ��Ŷ�� �����ϰ� �ȴ�.

���� �߰��� ������ ���ܼ� ��Ŷ�� �������� ���ؼ�
���� �ð��� ��ٷ��� �۽��ڰ� ������ ���� ���Ѵٸ�
������ ����ٰ� �Ǵ��ϰ� ������ ��Ŷ�� �ٽ� ������.
(���� �ð��̶�� ���� ���� ��θ� ª��, ���� ��θ� ��� ��µ� �̰��� ������ �����ؼ� �����Ѵ�.)

�� ������ TCP�� �ŷڼ��� ������ �� �ִ� ���̴�.
*/