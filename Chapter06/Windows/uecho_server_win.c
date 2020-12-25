#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET servSock;
	char message[BUF_SIZE];
	int strLen;
	int clntAddrSize;
	SOCKADDR_IN servAddr, clntAddr;

	if (argc != 2) {
		printf("Usage %s <port>", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// UDP socket = SOCK_DGRAM
	servSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (servSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	while (1) {
		clntAddrSize = sizeof(clntAddr);
		
		// 데이터를 받을 때 송신한 곳의 주소 정보를 저장
		strLen = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (strLen == SOCKET_ERROR)
			ErrorHandling("recvfrom() error");

		// 저장한 정보를 기반으로 데이터를 보내고
		// 보내고 나면 UDP 소켓에서 목적지 정보를 삭제
		// sendto()의 마지막인자는 목적지 주소 값의 구조체 변수 크기
		strLen = sendto(servSock, message, strLen, 0, (SOCKADDR*)&clntAddr, sizeof(clntAddr));
		if (strLen == SOCKET_ERROR)
			ErrorHandling("sendto() error");
	}
	closesocket(servSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
UDP

UDP는 TCP와 다르게
연결설정 및 해제과정이 없고
데이터 신뢰성을 보장하기 위한 흐름제어가 없다.
하지만 데이터의 크기가 작고, 신뢰도가 중요성이 조금 떨어져도 된다면 UDP가 빠르다.

압축파일 같은 경우는 데이터가 바뀌어버리면 내용이 바뀔 우려가 있기 때문에 TCP를 사용해야 하지만
동영상파일, 음악 파일은 데이터가 약간 손실되어도 잠깐 화면이 버벅이거나, 잡음이 나는 것이기 때문에 사용에 괜찮고
지속적으로 빠르게 데이터를 가져와야 하기 때문에 UDP가 좋다.

TCP는 통신하고자 하는 상대와 1대1로 소켓을 생성해야 했지만
UDP는 소켓이 하나만 있어도 여러 컴퓨터와 통신이 가능하다.

recvfrom() 함수 호출시에 상대방의 정보를 받아서 등록하고
sendto()함수 호출시에 등록된 정보를 기반으로 데이터를 전송하고 정보를 삭제한다.

*/