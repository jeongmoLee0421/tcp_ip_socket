#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
void ErrorHandling(char* msg);

/*
<여섯번째 매개변수 Event 오브젝트 기반 입출력 완료 확인>
IO가 완료되면 WSAOVERLAPPED 구조체 변수가 참조하는 Event 오브젝트가 signaled 상태가 된다.
IO의 완료 및 결과를 확인하려면 WSAGetOverLappedResult 함수를 사용한다.
*/

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN sendAddr;

	/*
	typedef struct __WSABUF{
		u_long len; // 전송할 데이터의 크기
		char FAR* buf; // 버퍼의 주소 값
	} WSABUF, *LPWSABUF
	*/
	WSABUF dataBuf;
	char msg[] = "hello my friend!";
	int sendBytes = 0;

	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;
	/*
	typedef struct _WSAOVERLAPPED{
		DWORD Internal;
		DWORD InternalHigh;
		DWORD Offset;
		DWORD OffsetHigh;
		WSAEVENT hEvent;
	} WSAOVERLAPPED, *LPWSAOVERLAPPED
	*/
	// Overlapped IO를 진행하려면 WSASend 함수의 매개변수 lpOverlapped에는 항상 NULL이 아닌, 유효한 구조체 변수의 주소 값을 전달해야 한다.(NULL전달 시 블로킹 모드로 동작하는 일반적인 소켓으로 간주)
	// WSASend 함수호출을 통해서 동시에 둘 이상의 영역으로 데이터를 전송하는 경우에는 여섯번째 인자로 전달되는 WSAOVERLAPPED 구조체 변수를 각각 별도로 구성해야 한다.

	if (argc != 3) {
		printf("Usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// Overlapped IO를 위한 소켓 생성
	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&sendAddr, 0, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(argv[1]);
	sendAddr.sin_port = htons(atoi(argv[2]));

	if (connect(hSocket, (SOCKADDR*)&sendAddr, sizeof(sendAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	// Event 오브젝트 생성
	evObj = WSACreateEvent();
	
	// 모든 비트 0으로 초기화
	memset(&overlapped, 0, sizeof(overlapped));

	overlapped.hEvent = evObj;
	dataBuf.len = strlen(msg) + 1;
	dataBuf.buf = msg;

	// WSASend함수가 반환을 한 이후에도 계속해서 데이터의 전송이 이뤄지는 상황이라면 SOCKET_ERROR를 반환하고 오류코드로는 WSA_IO_PENDING이 등록된다.
	// 반환했을 때 데이터 전송이 완료된 경우라면 sendBytes에 바로 전송된 데이터 크기가 담긴다.
	if (WSASend(hSocket, &dataBuf, 1, &sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			puts("Background data receive");

			// 데이터 전송이 완료되면 Event 오브젝트가 signaled 상태가 되는데
			// signaled 상태가 되어서 반환될 때까지 대기한다.
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);

			// 데이터 전송 결과를 확인한다.
			WSAGetOverlappedResult(hSocket, &overlapped, &sendBytes, FALSE, NULL);
		}
		else {
			ErrorHandling("WSASend() error");
		}
	}

	printf("Send data size: %d\n", sendBytes);
	WSACloseEvent(evObj);
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
Overlapped(중첩된) IO

하나의 스레드 내에서 동시에 둘 이상의 영역으로 데이터를 전송(또는 수신)함으로 인해서, 입출력이 중첩되는 상황을 가리켜 'IO의 중첩'이라 한다.
이러한 일이 가능하려면 호출된 입출력함수가 바로 반환해야 한다. 그래야 두번째, 세번째 데이터를 연속으로 보낼 수 있다.
결과적으로 비동기 IO가 가능해야 하고, 비동기 IO가 가능하려면 입출력함수는 non-blocking 모드로 동작해야 된다.

Overlapped IO 소켓 생성
SOCKET WSASocket(int af, int type, int protocol, LPWSAPROTOCOL_INFO lpProtocolInfo, GROUP g, DWORD dwFlags);
마지막 매개변수에 WSA_FLAG_OVERLAPPED를 전달해서 Overlapped IO가 가능하도록 속성 부여

WSASend 함수
int WSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutime);

데이터 전송, 수신 결과 확인 함수
BOOL WSAGetOverLappedResult(SOCKET s, LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer, BOOL fWait, LPDWORD lpdwFlags);

WSARecv 함수
int WSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
*/