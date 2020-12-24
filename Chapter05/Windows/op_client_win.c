#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
#define RLT_SIZE 4
#define OPSZ 4
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSocket;
	char opmsg[BUF_SIZE];
	int result, opndCnt, i;
	SOCKADDR_IN servAddr;

	if (argc != 3) {
		printf("Usage %s <ip> <port>", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));

	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");
	else
		puts("Connected................");

	fputs("Operand count: ", stdout);
	scanf("%d", &opndCnt);
	
	// char�� �迭 opmsg�� �ֱ� ���ؼ� int�� ������ (char)�� ĳ�����Ѵ�.
	// 4byte�� 1byte�� ĳ�����ϱ� ������ 32��Ʈ���� 8��Ʈ���� ���Եǰ�
	// -128���� 127������ ���� ��Ȯ�ϰ� ��ȯ�� �����ϴ�.
	// ������ ������ 8��Ʈ �̻��� ����ϱ� ������ (char)�� ĳ�����ϸ� 8��Ʈ�� ������ ��Ʈ���� �߸���.
	opmsg[0] = (char)opndCnt;

	for (i = 0; i < opndCnt; i++) {
		printf("Operand %d: ", i + 1);

		// 4byte ������ �Է¹޾Ƽ� (int*)�� ĳ�����ؼ� char�� �迭 opmsg�� 4byte��ŭ �����ϰ� �Ѵ�.
		// �ǿ����� ũ�Ⱑ 4����Ʈ�̰� �þ �� ���� i�� �����ָ�
		// 0�� �ڸ����� �ǿ������� ������ �� �ֱ� ������ +1
		scanf("%d", (int*)&opmsg[i * OPSZ + 1]);
	}

	// ���ۿ� �����ִ� '\n'�� ����ش�.
	fgetc(stdin);
	fputs("Operator: ", stdout);

	// 0������ �ǿ����� ����, ������ 4����Ʈ�� �ǿ�����, ������ 1����Ʈ�� �����ڸ� �Է� �޴´�.
	scanf("%c", &opmsg[opndCnt * OPSZ + 1]);

	// 1(�ǿ����� ����) + �ǿ�����(openCnd * OPSZ) + 1(������)
	send(hSocket, opmsg, opndCnt * OPSZ + 2, 0);

	// ��� ����� result�� ��´�.
	recv(hSocket, &result, RLT_SIZE, 0);

	printf("Operation result: %d\n", result);
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
TCP�� �帧����

TCP ���Ͽ��� �Է¹��ۿ� ��¹��۰� �ִ�.
write()�� ȣ���ϸ� ��¹��۷� �����͸� �̵���Ű��
read()�� ȣ���ϸ� �Է¹��ۿ��� �����͸� �д´�.

TCP�� �����̵� �������� ���������� �ִµ�
������ ������ ũ�⸦ ���� �����鼭
��밡 ������ �ִ� ��ŭ�� �����͸� �����Ѵ�.

�׷��� ������ ���۰� ���ļ� �����Ͱ� �Ҹ�Ǵ� ���� �߻����� �ʴ´�.
*/