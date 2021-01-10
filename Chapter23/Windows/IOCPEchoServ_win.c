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

// PER_IO_DATA ����ü ������ �ּҰ��� ����ü ù��° ���(overlapped)�� �ּҰ��� ��ġ�Ѵ�.
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

	// Completion Port ������Ʈ �����Ѵ�.
	// ���������ڰ� 0�̱� ������ �ھ��� ����ŭ(CPU ����ŭ) �����尡 CP ������Ʈ�� �Ҵ�� �� �ִ�.
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	
	// ���� �������� �ý��� ������ ��´�.
	GetSystemInfo(&sysInfo);

	// dwNumberOfProcessors���� CPU�� ���� ����ȴ�. -> CPU ����ŭ ������ ����
	// ������ ������ CP ������Ʈ�� �ڵ��� �����Ѵ�. -> ������� �� �ڵ��� ������� CP ������Ʈ�� �����Ѵ�.
	// ������� �� �ڵ�� ����(hComPort) CP ������Ʈ�� �Ҵ��� �̷�����.
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

	// Overlapped �Ӽ� �ο�
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

		// PER_HANDLE_DATA ����ü ������ �����Ҵ� �� �Ŀ� Ŭ���̾�Ʈ�� ����� ����, Ŭ���̾�Ʈ�� �ּ� ������ ��´�.
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAddr), &clntAddr, addrLen);

		// CP ������Ʈ�� ���� ����
		// �� ������ ������� �ϴ� Overlapped IO�� �Ϸ�� ��, ����� CP ������Ʈ�� �Ϸῡ���� ������ ���Եǰ�, �̷� ���ؼ� GetQueuedCompletionStatus �Լ��� ��ȯ�� �ȴ�.
		// ����° ���� ���� GetQueuedCompletionStatus �Լ��� ��ȯ�� �ϸ鼭 ��� �ȴ�.
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;

		// IOCP�� �Է��� �Ϸ�� ����� �ϷḦ ���� �������� �ʴ´�. �ٸ� �Է��̰� ����̰� �Ϸ�Ǿ��ٴ� ��Ǹ� �ν��� �����ش�.
		// ���� �Է��� ������ ������, �ƴϸ� ����� ������ �������� ���� ������ ������ ����ص־� �Ѵ�.
		ioInfo->rwMode = READ;

		// WSARecv �Լ��� ȣ���ϸ鼭 ������° ���ڷ� OVERLAPPED ����ü ������ �ּҰ��� �����Ͽ���. �� ���� GetQueuedCompletionStatus �Լ��� ��ȯ�� �ϸ鼭 ���� �� �ִ�.
		// �׷��� ����ü ������ �ּҰ��� ù��° ����� �ּҰ��� �����ϹǷ� PER_IO_DATA ����ü ������ �ּҰ��� ������ �Ͱ� ����.
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
		// GetQueuedCompletionStatus �Լ��� IO�� �Ϸ�ǰ�, �̿� ���� ������ ��ϵǾ��� �� ��ȯ�Ѵ�. (������ ���ڰ� INFINITE �̹Ƿ�)
		// ��ȯ�� �� ����°, �׹�° ���ڸ� ���ؼ� �ΰ��� ������ ��� �ȴ�.
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);

		sock = handleInfo->hClntSock;

		// ioInfo�� ����� ���� OVERLAPPED ����ü ������ �ּ� �������� PER_IO_DATA����ü ������ �ּҰ��̱⵵ �ϴ�. (����ü������ �ּҰ��� ù��° ����� �ּҰ��� ��ġ)
		// ���� rwMode�� ���� �����ϴ�.
		if (ioInfo->rwMode == READ) {
			puts("message received");
			if (bytesTrans == 0) { // ���ŵ� �����Ͱ� 0�̶�� ���� EOF
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
		else { // ������ ������ �Ϸ��
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
�� ������ ������� ���صǴ� IO�� �Ϸ� ��Ȳ�� �� CP ������Ʈ�� ������ּ���

1. Completion Port�� ����
HANDLE CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
CP������Ʈ �����ÿ��� ������ �Ű������� �ǹ̸� ������ CP������Ʈ�� �Ҵ�Ǿ� �Ϸ�� IO�� ó���� �������� ���� �����Ѵ�.

2. Completion Port ������Ʈ�� ������ ����
HANDLE CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
FileHandle�� ���޵� �ڵ��� ������ ExistingCompletionPort�� ���޵� CP������Ʈ�� �����Ų��.

3. Completion Port�� �Ϸ�� IOȮ�ΰ� �������� IOó��
BOOL GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytes, PULONG_PTR lpCompletionKey, LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds);

GetQueuedCompletionStatus �Լ��� ����° ���ڸ� ���ؼ� ��� �Ǵ� ���� ���ϰ� CP������Ʈ�� ������ �������� CreateIoCompletionPort �Լ��� ȣ��� �� ���޵Ǵ� ����° ���� ���̴�.
GetQueuedCompletionStatus �Լ��� �׹�° ���ڸ� ���ؼ� ��� �Ǵ� ���� WSASend, WSARecv �Լ� ȣ��� ���޵Ǵ� WSAOVERLAPPED ����ü ������ �ּ� ���̴�.

IOCP�� ����(vs select)
non-blocking ������� IO�� ����Ǳ� ������, IO �۾����� ���� �ð��� ������ �߻����� �ʴ´�.
IO�� �Ϸ�� �ڵ��� ã�� ���ؼ� �ݸ��� ������ �ʿ䰡 ����.
IO�� �������� ������ �ڵ��� �迭�� ������ ���� ������ �ʿ䰡 ����.
IO�� ó���� ���� �������� ���� ������ �� �ִ�. ������ ����� ���� ������ ������ ���� ������ �� �ִ�.
*/