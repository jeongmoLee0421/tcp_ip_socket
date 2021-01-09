#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void CALLBACK CompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* msg);

WSABUF dataBuf;
char buf[BUF_SIZE];
int recvBytes = 0;

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAddr, recvAddr;

	WSAOVERLAPPED overlapped;
	WSAEVENT evObj;

	int idx, recvAddrSize, flags = 0;

	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// Overlapped IO 소켓 생성
	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hLisnSock == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&lisnAddr, 0, sizeof(lisnAddr));
	lisnAddr.sin_family = AF_INET;
	lisnAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hLisnSock, (SOCKADDR*)&lisnAddr, sizeof(lisnAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAddrSize = sizeof(recvAddr);
	hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAddr, &recvAddrSize);
	if (hRecvSock == INVALID_SOCKET)
		ErrorHandling("accpet() error");

	memset(&overlapped, 0, sizeof(overlapped));
	dataBuf.len = BUF_SIZE;
	dataBuf.buf = buf;
	evObj = WSACreateEvent(); // 사용되지 않는 이벤트 오브젝트(Completion Routine함수 사용하기 때문)

	// Completion Routine을 이용하더라도 6번째 인자로 WSAOVERLAPPED 구조체 변수의 주소값은 넘겨야 한다.
	// 하지만 Event 오브젝트를 생성해서 넣을 필요는 없다.
	if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, CompRoutine) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING)
			puts("Background data receive");
	}

	// WSAWaitForMultipleEvents 함수를 호출하면 alertable wait상태에 놓이게 되고 운영체제가 Completion Routine을 호출하게 된다.
	// 어차피 Event 오브젝트는 실제로 사용되지 않기 때문에 signaled 상태로 변경될 일이 없고 WSA_INFINITE기 때문에 무한 대기를 하지만
	// Completion Routine이 실행되면서 이 함수는 WAIT_IO_COMPLETION을 반환하면서 빠져나온다.
	idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE);
	if (idx == WAIT_IO_COMPLETION) // IO의 정상완료를 의미
		puts("Overlapped I/O Completed");
	else
		ErrorHandling("WSARecv() error");

	WSACloseEvent(evObj);
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

void CALLBACK CompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	// 첫번째 매개변수 = 오류정보
	// 두번째 매개변수 = 완료된 데이터의 입출력 크기 정보
	// 세번째 매개변수 = WSASend, WSARecv 함수의 매개변수 lpOverlapped로 전달된 값
	// 마지막 매개변수 = 입출력 함수호출시 전달된 특성정보 또는 0이 전달
	if (dwError != 0)
		ErrorHandling("CompRoutine error");
	else {
		recvBytes = szRecvBytes;
		printf("Received message: %s\n", buf);
	}
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
<WSASend, WSARecv 함수의 마지막 전달인자를 통해서 등록되는, Completion Routine이라 불리는 함수를 통해서 입출력 완료 확인>
Pending된 IO가 완료되면, 이 함수를 호출해 달라고 운영체제에게 요청한다.

중요한 작업을 진행중에는 Completion Routine이 호출되면 프로그램의 흐름을 망칠 수 있기 때문에
운영체제는 IO를 요청한 쓰레드가 alerable wait 상태에 놓여있을 때만 Completion Routine을 호출한다.

alertable wait 상태는 운영체제가 전달하는 메시지의 수신을 대기하는 쓰레드의 상태를 뜻한다.
WaitForSingleObjectEx
WaitForMultipleOvjectsEx
WSAWaitForMultipleEvents
SleepEx

Completion Routine이 실행되면 위 함수들은 모두 WAIT_IO_COMPLETION을 반환하면서 함수를 빠져 나온다.
그리고 그 다음부터 실행을 이어 나간다.
*/

/*
비동기 Notification IO 모델의 경우에는 IO관련 이벤트의 발생을 알리는 과정이 비동기로 처리된다.
Overlapped IO 모델의 경우는 IO가 완료된 상황을 확인하는 과정이 비동기로 처리된다.

비동기 IO는 IO가 완료된 상황을 확인하는 과정이 비동기로 처리됨을 의미한다.
이러한 유형으로 IO를 진행하기 위해서는 IO가 non-blocking 모드로 동작해야 하며
IO가 non-blocking 모드로 동작해서 비동기로 IO를 진행하게 되면, 이를 바탕으로 IO를 중첩시킨 형태인 Overlapped IO가 가능해진다.
*/