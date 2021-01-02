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

	FD_ZERO(&reads); // fd_set형 변수 초기화
	FD_SET(hServSock, &reads); // fd_set형 변수에 ServSock 등록

	while (1) {
		// select 함수가 호출되고 나면 변경사항이 있는 소켓핸들은 1을 유지하지만 변경사항이 없는 소켓핸들은 0으로 변경된다.
		// 0으로 변경된 fd_set형 변수를 그대로 사용하게 된다면 이후에 추적이 불가능하다.
		// 때문에 매번 원본을 복사해서 사용하는 것이다.
		cpyReads = reads;

		// select 함수가 호출되고 나면 TIMEVAL 구조체의 멤버들은 타임아웃이 발생하기까지 남았던 시간으로 바뀐다.(찍어보면 0, 0 출력)
		// 때문에 매번 타임아웃시간을 초기화 해줘야 한다.
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
			break;

		if (fdNum == 0) // 관찰 대상중에 변환된 것이 없고 타임아웃 시간이 지나서 발생하면 0을 반환
			continue;

		for (i = 0; i < reads.fd_count; i++) { // 에러도 발생하지 않았고 타임아웃도 발생하지 않았다면 어디선가 변화가 있었다는 것
			if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
				if (reads.fd_array[i] == hServSock) { // hServSock에 수신된 데이터가 있다는 것은 클라이언트에서 연결요청이 있었다는 뜻
					addrSize = sizeof(clntAddr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrSize);
					if (hClntSock == INVALID_SOCKET)
						ErrorHandling("accept() error");

					FD_SET(hClntSock, &reads); // 새로 연결된 클라이언트 소켓을 추적하기 위해서 fd_set형 변수에 등록
					printf("connected client: %d\n", hClntSock);
				}
				else { // 서버소켓이 아니면 클라이언트 소켓을 의미하고 데이터를 에코해준다.
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE, 0);
					if (strLen == 0) { // 연결종료를 의미하는 EOF라면
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
<멀티프로세스 서버의 단점>

연결요청이 들어올 때 마다 새로운 프로세스를 생성하는 것은 상당히 많이 비용이 든다.
많은 양의 연산이 요구되고 필요한 메모리 공간도 비교적 크다.

또한 프로세스마다 별도의 메모리 공간을 유지하기 때문에 데이터를 주고받음에 있어서 복잡하다.



<멀티플렉싱>
하나의 통신채널을 통해서 둘 이상의 데이터(시그널)를 전송하는데 사용되는 기술

멀티프로세스는 클라이언트마다 자식프로세스가 필요했지만
멀티플렉싱은 부모 프로세스 하나만 존재하며 select()함수로 소켓핸들의 변화를 추적하면서 클라이언트에 대응한다.
*/