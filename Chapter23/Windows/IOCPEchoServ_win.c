#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>

#define BUF_SIZE 100
#define READ 3
#define WRITE 5

typedef struct { // socket info
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// PER_IO_DATA 구조체 변수의 주소값은 구조체 첫번째 멤버(overlapped)의 주소값과 일치한다.
typedef struct { // buffer info
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode; // read, write
}PER_IO_DATA, *LPPER_IO_DATA;

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);
void ErrorHandling(char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;

	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	int recvBytes, i, flags = 0;

	if (argc != 2) {
		printf("Usage %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// Completion Port 오브젝트 생성한다.
	// 마지막인자가 0이기 때문에 코어의 수만큼(CPU 수만큼) 쓰레드가 CP 오브젝트에 할당될 수 있다.
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	
	// 현재 실행중인 시스템 정보를 얻는다.
	GetSystemInfo(&sysInfo);

	// dwNumberOfProcessors에는 CPU의 수가 저장된다. -> CPU 수만큼 쓰레드 생성
	// 쓰레드 생성시 CP 오브젝트의 핸들을 전달한다. -> 쓰레드는 이 핸들을 대상으로 CP 오브젝트에 접근한다.
	// 쓰레드는 이 핸들로 인해(hComPort) CP 오브젝트에 할당이 이뤄진다.
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

	// Overlapped 속성 부여
	hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	while (1) {
		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int addrLen = sizeof(clntAddr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrLen);
		if (hClntSock == INVALID_SOCKET)
			ErrorHandling("accept() error");

		// PER_HANDLE_DATA 구조체 변수를 동적할당 한 후에 클라이언트와 연결된 소켓, 클라이언트의 주소 정보를 담는다.
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAddr), &clntAddr, addrLen);

		// CP 오브젝트와 소켓 연결
		// 이 소켓을 기반으로 하는 Overlapped IO가 완료될 때, 연결된 CP 오브젝트에 완료에대한 정보가 삽입되고, 이로 인해서 GetQueuedCompletionStatus 함수는 반환이 된다.
		// 세번째 인자 역시 GetQueuedCompletionStatus 함수가 반환을 하면서 얻게 된다.
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;

		// IOCP는 입력의 완료와 출력의 완료를 구분 지어주지 않는다. 다만 입력이건 출력이건 완료되었다는 사실만 인식을 시켜준다.
		// 따라서 입력을 진행한 것인지, 아니면 출력을 진행한 것인지에 대한 정보를 별도로 기록해둬야 한다.
		ioInfo->rwMode = READ;

		// WSARecv 함수를 호출하면서 여섯번째 인자로 OVERLAPPED 구조체 변수의 주소값을 전달하였다. 이 값은 GetQueuedCompletionStatus 함수가 반환을 하면서 얻을 수 있다.
		// 그런데 구조체 변수의 주소값은 첫번째 멤버의 주소값과 동일하므로 PER_IO_DATA 구조체 변수의 주소값을 전달한 것과 같다.
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
	}
	return 0;
}

DWORD WINAPI EchoThreadMain(LPVOID pComPort) {
	HANDLE hComPort = (HANDLE)pComPort;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	while (1) {
		// GetQueuedCompletionStatus 함수는 IO가 완료되고, 이에 대한 정보가 등록되었을 때 반환한다. (마지막 인자가 INFINITE 이므로)
		// 반환할 때 세번째, 네번째 인자를 통해서 두가지 정보를 얻게 된다.
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);

		sock = handleInfo->hClntSock;

		// ioInfo에 저장된 값은 OVERLAPPED 구조체 변수의 주소 값이지만 PER_IO_DATA구조체 변수의 주소값이기도 하다. (구조체변수의 주소값은 첫번째 멤버의 주소값과 일치)
		// 따라서 rwMode에 접근 가능하다.
		if (ioInfo->rwMode == READ) {
			puts("message received");
			if (bytesTrans == 0) { // 수신된 데이터가 0이라는 것은 EOF
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				continue;
			}

			// echo
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->rwMode = WRITE;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else { // 데이터 전송이 완료됨
			puts("message sent");
			free(ioInfo);
		}
	}
	return 0;
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
IOCP
이 소켓을 기반으로 진해되는 IO의 완료 상황은 저 CP 오브젝트에 등록해주세요

1. Completion Port의 생성
HANDLE CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
CP오브젝트 생성시에는 마지막 매개변수만 의미를 가지며 CP오브젝트에 할당되어 완료된 IO를 처리할 쓰레드의 수를 전달한다.

2. Completion Port 오브젝트와 소켓의 연결
HANDLE CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
FileHandle에 전달된 핸들의 소켓을 ExistingCompletionPort에 전달된 CP오브젝트에 연결시킨다.

3. Completion Port의 완료된 IO확인과 쓰레드의 IO처리
BOOL GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytes, PULONG_PTR lpCompletionKey, LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds);

GetQueuedCompletionStatus 함수의 세번째 인자를 통해서 얻게 되는 것은 소켓과 CP오브젝트의 연결을 목적으로 CreateIoCompletionPort 함수가 호출될 때 전달되는 세번째 인자 값이다.
GetQueuedCompletionStatus 함수의 네번째 인자를 통해서 얻게 되는 것은 WSASend, WSARecv 함수 호출시 전달되는 WSAOVERLAPPED 구조체 변수의 주소 값이다.

IOCP의 장점(vs select)
non-blocking 방식으로 IO가 진행되기 때문에, IO 작업으로 인한 시간의 지연이 발생하지 않는다.
IO가 완료된 핸들을 찾기 위해서 반목문을 구성할 필요가 없다.
IO의 진행대상인 소켓의 핸들을 배열에 저장해 놓고 관리할 필요가 없다.
IO의 처리를 위한 쓰레드의 수를 조절할 수 있다. 실험적 결과를 토대로 적절한 쓰레드 수를 지정할 수 있다.
*/