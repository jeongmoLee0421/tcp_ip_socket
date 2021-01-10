#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* msg);

typedef struct {
	SOCKET hClntSock;
	char buf[BUF_SIZE];
	WSABUF wsaBuf;
}PER_IO_DATA, *LPPER_IO_DATA;

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAddr, recvAddr;
	LPWSAOVERLAPPED lpOvLp;
	DWORD recvBytes;
	LPPER_IO_DATA hbInfo;
	int mode = 1, recvAddrSize, flagInfo = 0;

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

	// non-blocking 모드 소켓 설정
	ioctlsocket(hLisnSock, FIONBIO, &mode);

	memset(&lisnAddr, 0, sizeof(lisnAddr));
	lisnAddr.sin_family = AF_INET;
	lisnAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hLisnSock, (SOCKADDR*)&lisnAddr, sizeof(lisnAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAddrSize = sizeof(recvAddr);
	while (1) {
		SleepEx(100, TRUE); // Completion Routine을 실행하기 위한 alertable wait 상태

		// non-blocking 속성의 소켓을 통해 새로 생성되는 소켓도 non-blocking 속성을 가진다.
		hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAddr, &recvAddrSize);
		if (hRecvSock == INVALID_SOCKET) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) // 클라이언트 연결요청이 존재하지 않는 상태에서 accept함수 호출시 INVALID_SOCKET이 바로 반환되며, WSAEWOULDBLOCK이 반환된다.
				continue;
			else // accept 함수 호출 오류시
				ErrorHandling("accpet() error");
		}
		puts("client connected....");

		// 클라이언트 하나당 WSAOVERPALLED 구조체 변수를 하나씩 할당해야 하기 때문
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		hbInfo->hClntSock = (DWORD)hRecvSock;
		(hbInfo->wsaBuf).buf = hbInfo->buf;
		(hbInfo->wsaBuf).len = BUF_SIZE;

		// Completion Routine 기반의 Overlapped IO에서는 Event 오브젝트가 불필요하기 때문에 hEvent에 필요한 다른 정보를 채워도 된다.
		// 여기서는 hbInfo로 채웠는데 이는 Completion Routine 함수 내부로 전달된다(여섯번째 매개변수 WSAOVERLAPPED 구조체가 Completion Routine 함수의 세번째 매개변수로 전달)
		lpOvLp->hEvent = (HANDLE)hbInfo;

		// 입출력이 완료되면 ReadCompRoutine 함수를 호출
		// 여섯번째 매개변수 WSAOVERLAPPED 구조체 변수의 주소값은 Completion Routine의 세번째 매개변수로 전달
		WSARecv(hRecvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, lpOvLp, ReadCompRoutine);
	}

	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

void CALLBACK ReadCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD sentBytes;

	if (szRecvBytes == 0) { // 연결종료(EOF)
		closesocket(hSock);
		
		// 동적할당한 lpOvLp와 hbInfo(hEvent에 저장되어 넘어온)의 메모리를 해제한다.
		free(lpOverlapped->hEvent);
		free(lpOverlapped);
		puts("client disconnected...");
	}
	else { // echo
		bufInfo->len = szRecvBytes;
		WSASend(hSock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine);
	}
}

void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD recvBytes;
	int flagInfo = 0;

	// non-blcoking 모드로 데이터가 수신되기를 기다린다.
	WSARecv(hSock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
1. 클라이언트가 연결되면 WSARecv 함수를 호출하면서 non-blocking 모드로 데이터가 수신되게 하고, 수신이 완료되면 ReadCompRoutine 함수가 호출되게 한다.
2. ReadCompRoutine 함수가 호출되면 WSASend 함수를 호출하면서 non-blocking 모드로 데이터가 송신되게 하고(echo), 송신이 완료되면 WriteCompRoutine 함수가 호출되게 한다.
3. WirteCompRoutine 함수가 호출되면 WSARecv 함수를 호출하면서 non-blocking 모드로 데이터의 수신을 기다린다.

ReadCompRoutine 함수와 WriteCompRoutine 함수가 번갈아 호출되면서 데이터의 수신, 송신을 반복한다.



Overlapped IO 모델의 에코 서버의 단점
non-blocking 모드의 accept함수와 alertable wait 상태로의 진입을 위한 SleepEx 함수가 번갈아 가며 반복 호출되는 것은 성능에 영향을 미칠 수 있다.

다른 방안
accept 함수의 호출은 main쓰레드가 처리하도록 하고, 별도의 쓰레드를 추가로 하나 생성해서 클라이언트와의 입출력을 담당하게 한다.(IOCP)
IO를 전담하는 쓰레드를 별도로 생성


Overlapped IO나 IOCP나 가장 중요한 점
1. 입력과 출력은 non-blocking 모드로 동작하는가?
2. non-blocking 모드로 진행된 입력과 출력의 완료는 어떻게 확인하는가?
*/