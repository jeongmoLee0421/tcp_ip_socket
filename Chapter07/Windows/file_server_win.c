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

		// BUF_SIZE보다 작은 수를 읽었다면
		// 파일의 끝을 의미한다.
		if (readCnt < BUF_SIZE) {

			// buf 자체가 char형이기 때문에 (char*)형으로 캐스팅해서 주소(&)를 전달하지 않고
			// buf(배열은 이름이 첫 주소)만 전달해도 동작된다.
			send(hClntSock, (char*)&buf, readCnt, 0);
			break;
		}

		// send()는 두번째 인자가 전송할 데이터를 저장하고 있는 버퍼의 주소 값인데
		// const char* 형이다.
		send(hClntSock, (char*)&buf, BUF_SIZE, 0);
	}

	// hClntSock의 출력스트림을 종료한다.
	shutdown(hClntSock, SD_SEND);

	// hClntSock의 입력스트림은 살아있는 상태이기 때문에
	// 상대로부터 전송된 메시지를 받을 수 있다.
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

리눅스에서 close(), 윈도우에서 closesocket()은 완전종료를 의미한다.
입력 스트립, 출력 스트림을 모두 종료하고 소켓을 메모리에서 해제하는 것이다.
위 함수가 실행되고 나면 더 이상 송/수신이 불가하다.

예를 하나 들어보자
클라이언트가 서버로부터 어떤 데이터를 받았고 그에 대한 응답을 해야하는 상황이다.

서버는 자신이 보낼 데이터를 모두 전송하고 closesocket()로 완전종료를 해버리면
클라이언트의 응답을 받을 수가 없다.

이 때 사용할 수 있는 것이 shutdown()이다.
위 함수는 두번째 인자로 종료할 스트림을 결정할 수 있는데
입력 스트림, 출력 스트림, 두 스트림 모두 이렇게 세가지가 있다.

서버가 데이터를 모두 전송하고서 출력 스트림만 닫고 입력스트림을 열어두고 있다면
클라이언트의 응답을 받을 수 있다.

받고나서 closesocket()으로 완전종료를 하면 된다.
*/