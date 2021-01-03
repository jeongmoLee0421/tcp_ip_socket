#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hAcptSock, hRecvSock;

	SOCKADDR_IN recvAddr, sendAddr;
	int sendAddrSize, strLen;
	char buf[BUF_SIZE];
	int result;

	fd_set read, except, readCopy, exceptCopy;
	struct timeval timeout;

	if (argc != 2) {
		printf("Usage %s <port>", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hAcptSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hAcptSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hAcptSock, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hAcptSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	sendAddrSize = sizeof(sendAddr);
	hRecvSock = accept(hAcptSock, (SOCKADDR*)&sendAddr, & sendAddrSize);
	if (hRecvSock == INVALID_SOCKET)
		ErrorHandling("accept() error");

	FD_ZERO(&read);
	FD_ZERO(&except);
	FD_SET(hRecvSock, &read);
	FD_SET(hRecvSock, &except);

	while (1) {
		readCopy = read;
		exceptCopy = except;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		result = select(0, &readCopy, 0, &exceptCopy, &timeout);

		if (result > 0) {
			
			// 예외사항이 발생한 소켓을 추적하면 MSG_OOB 데이터를 수신할 수 있다.
			if (FD_ISSET(hRecvSock, &exceptCopy)) {
				strLen = recv(hRecvSock, buf, BUF_SIZE - 1, MSG_OOB);
				buf[strLen] = 0;
				printf("Urgent message: %s\n", buf);
			}

			if (FD_ISSET(hRecvSock, &readCopy)) {
				strLen = recv(hRecvSock, buf, BUF_SIZE - 1, 0);
				if (strLen == 0) {
					break;
					closesocket(hRecvSock);
				}
				else {
					buf[strLen] = 0;
					puts(buf);
				}
			}
		}
	}

	closesocket(hAcptSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
윈도우는 리눅스처럼 시그널 핸들러 함수 등록이 불가능하다.

그래서 select 함수를 사용해서 문제를 해결할 수 있다.
1. 수신한 데이터를 지니고 있는 소켓이 존재하는가?
2. 블로킹되지 않고 데이터의 전송이 가능한 소켓은 무엇인가?
3. 예외상황이 발생한 소켓은 무엇인가?

이 중에 3번째 예외상황이 발생한 소켓은 무언인가?를 추적하면 된다.
out-of-band 데이터의 수신도 예외사항에 해당되기 때문이다.
*/

/*
MSG_OOB는 TCP의 기본 데이터 전송방식을 벗어나지 못한다.
즉 MSG_OOB 옵션이 설정되어도 전송순서는 그대로 유지된다.
하지만 MSG_OOB 옵션은 수신측에 긴급하게 데이터를 처리해 달라고 요청하는 용도로 사용된다.
*/