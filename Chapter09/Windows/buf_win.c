#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

void ErrorHandling(char* message);
void ShowSocketBufSize(SOCKET sock);

int main(int argc, char* argv[]) {
	SOCKET hSock;
	WSADATA wsaData;
	int sndBuf, rcvBuf, state;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hSock == INVALID_SOCKET)
		ErrorHandling("socket() error");
	ShowSocketBufSize(hSock);

	sndBuf = 1024 * 3, rcvBuf = 1024 * 3;
	
	// setsockopt()
	// sndBuf값을 참조해서 소켓의 출력버퍼값을 변경한다.
	state = setsockopt(hSock, SOL_SOCKET, SO_SNDBUF, (char*)&sndBuf, sizeof(sndBuf));
	if (state == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	state = setsockopt(hSock, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuf, sizeof(rcvBuf));
	if (state == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	ShowSocketBufSize(hSock);
	closesocket(hSock);
	WSACleanup();
	return 0;
}

void ShowSocketBufSize(SOCKET sock) {
	int sndBuf, rcvBuf, state, len;

	// getsockopt()
	// 소켓의 출력버퍼 값을 참조하여 sndBuf에 값을 저장한다.
	len = sizeof(sndBuf);
	state = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sndBuf, &len);
	if (state == SOCKET_ERROR)
		ErrorHandling("getsockopt() error");

	len = sizeof(rcvBuf);
	state = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuf, &len);
	if (state == SOCKET_ERROR)
		ErrorHandling("getsockopt() error");

	printf("Output buffer size(send): %d\n", sndBuf);
	printf("Input buffer size(receive): %d\n", rcvBuf);
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}