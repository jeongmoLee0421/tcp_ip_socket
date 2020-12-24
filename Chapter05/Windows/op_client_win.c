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
	
	// char형 배열 opmsg에 넣기 위해서 int형 변수를 (char)로 캐스팅한다.
	// 4byte를 1byte로 캐스팅하기 때문에 32비트에서 8비트만이 남게되고
	// -128에서 127까지의 수만 정확하게 변환이 가능하다.
	// 나머지 수들은 8비트 이상을 사용하기 때문에 (char)로 캐스팅하면 8비트를 제외한 비트들이 잘린다.
	opmsg[0] = (char)opndCnt;

	for (i = 0; i < opndCnt; i++) {
		printf("Operand %d: ", i + 1);

		// 4byte 정수를 입력받아서 (int*)로 캐스팅해서 char형 배열 opmsg에 4byte만큼 차지하게 한다.
		// 피연산자 크기가 4바이트이고 늘어갈 때 마다 i를 곱해주며
		// 0번 자리에는 피연산자의 개수가 들어가 있기 때문에 +1
		scanf("%d", (int*)&opmsg[i * OPSZ + 1]);
	}

	// 버퍼에 남아있는 '\n'을 비워준다.
	fgetc(stdin);
	fputs("Operator: ", stdout);

	// 0번에는 피연산자 개수, 나머지 4바이트는 피연산자, 마지막 1바이트에 연산자를 입력 받는다.
	scanf("%c", &opmsg[opndCnt * OPSZ + 1]);

	// 1(피연산자 개수) + 피연산자(openCnd * OPSZ) + 1(연산자)
	send(hSocket, opmsg, opndCnt * OPSZ + 2, 0);

	// 계산 결과를 result에 담는다.
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
TCP의 흐름제어

TCP 소켓에는 입력버퍼와 출력버퍼가 있다.
write()를 호출하면 출력버퍼로 데이터를 이동시키고
read()를 호출하면 입력버퍼에서 데이터를 읽는다.

TCP는 슬라이딩 윈도우라는 프로토콜이 있는데
상대방의 윈도우 크기를 통지 받으면서
상대가 받을수 있는 만큼의 데이터만 전송한다.

그렇기 때문에 버퍼가 넘쳐서 데이터가 소멸되는 일은 발생하지 않는다.
*/