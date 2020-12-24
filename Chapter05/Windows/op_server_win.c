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

		// TCP특성상 입력버퍼에 데이터가 얼마나 있던지 사용자 임의대로 읽을 수 있다.
		// 입력버퍼에 있는 데이터중에 가장 선두에 1byte가 피연산자 개수이기 때문에
		// 1byte만 받아서 opndCnt(int형)에 저장
		recv(hClntSock, &opndCnt, 1, 0);

		recvLen = 0;

		// 총 받은 길이는 피연산자 byte수 + 연산자(1byte)
		while ((opndCnt * OPSZ + 1) > recvLen) {
			recvCnt = recv(hClntSock, &opinfo[recvLen], BUF_SIZE - 1, 0);
			recvLen += recvCnt;
		}

		// opinfo는 char형 배열이지만 (int*)로 캐스팅하게 되면
		// 4byte씩 묶어서 정수형 배열처럼 사용
		// 받은 데이터의 총 길이(recvLen) - 1이 마지막 인덱스고 이는 연산자를 의미한다.
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
TCP의 오류제어

TCP 소켓에서 데이터를 주고 받을 때는
내가 데이터를 어디까지 받았고 어디서부터 보내면 된다는 응답을 항상 한다.

SEQ를 1000으로 하고 100byte를 전송했을 때
수신자는 전부 받았다면 1000 + 100(전송되 바이트 크기) + 1 byte로
ACK를 1101로 응답한다.
그럼 송신자는 제대로 갔다고 확인하고 다음 패킷을 전송하게 된다.

만약 중간에 문제가 생겨서 패킷이 도착하지 못해서
일정 시간을 기다려도 송신자가 응답을 받지 못한다면
문제가 생겼다고 판단하고 동일한 패킷을 다시 보낸다.
(일정 시간이라는 것은 빠른 경로면 짧게, 느린 경로면 길게 잡는데 이것은 실제로 측정해서 설정한다.)

이 때문에 TCP는 신뢰성을 보장할 수 있는 것이다.
*/