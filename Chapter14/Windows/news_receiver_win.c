#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h> // for struct ip_mreq

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hRecvSock;
	SOCKADDR_IN addr;
	struct ip_mreq joinAddr;
	char buf[BUF_SIZE];
	int strLen;

	if (argc != 3) {
		printf("Usage: %s <group IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hRecvSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (hRecvSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[2]));

	// 소켓에 ip주소와 port번호 등록
	if (bind(hRecvSock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	joinAddr.imr_multiaddr.s_addr = inet_addr(argv[1]);
	joinAddr.imr_interface.s_addr = htonl(INADDR_ANY);

	// 소켓에 그룹 ip 가입
	if (setsockopt(hRecvSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&joinAddr, sizeof(joinAddr)) == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	while (1) {
		strLen = recvfrom(hRecvSock, buf, BUF_SIZE - 1, 0, NULL, 0);
		if (strLen < 0)
			break;

		buf[strLen] = 0;
		fputs(buf, stdout);
	}

	closesocket(hRecvSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
멀티캐스트는 네트워크 달라도 그룹에 가입되어 있다면 데이터를 받는다.
브로드캐스트는 동일한 네트워크에 있는 모든 호스트가 데이터를 받는다.

Directed 브로드캐스트는 특정 네트워크를 정해서 데이터를 전송할 수 있는데 네트워크 뒤에 255만 붙여주면 된다.
예를 들어서 vmnet8번의 네트워크 주소가 192.168.217.1인데 192.168.217.255로 지정하면 된다.

Local 브로드캐스트는 전송하는 호스트의 네트워크에 있는 모든 호스트가 데이터를 받는다.
255.255.255.255 라고 약속된 ip주소를 사용한다.

멀티캐스트는 트래픽 측면에서도 긍정적인데
tcp의 경우 1000명에게 데이터를 보낸다고 했을 때 경로가 같던지 다르던지 상관하지 않고 1000번을 보내야한다.
하지만 udp기반 멀티캐스트는 한번만 전송하면 라우터가 데이터를 복사해서 같은 경로에 존재하는 호스트들에게 전송하기 때문에 효율적이다.
*/