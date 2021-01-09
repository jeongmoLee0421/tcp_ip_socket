#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
void ErrorHandling(char* msg);

/*
<������° �Ű����� Event ������Ʈ ��� ����� �Ϸ� Ȯ��>
IO�� �Ϸ�Ǹ� WSAOVERLAPPED ����ü ������ �����ϴ� Event ������Ʈ�� signaled ���°� �ȴ�.
IO�� �Ϸ� �� ����� Ȯ���Ϸ��� WSAGetOverLappedResult �Լ��� ����Ѵ�.
*/

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN sendAddr;

	/*
	typedef struct __WSABUF{
		u_long len; // ������ �������� ũ��
		char FAR* buf; // ������ �ּ� ��
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
	// Overlapped IO�� �����Ϸ��� WSASend �Լ��� �Ű����� lpOverlapped���� �׻� NULL�� �ƴ�, ��ȿ�� ����ü ������ �ּ� ���� �����ؾ� �Ѵ�.(NULL���� �� ���ŷ ���� �����ϴ� �Ϲ����� �������� ����)
	// WSASend �Լ�ȣ���� ���ؼ� ���ÿ� �� �̻��� �������� �����͸� �����ϴ� ��쿡�� ������° ���ڷ� ���޵Ǵ� WSAOVERLAPPED ����ü ������ ���� ������ �����ؾ� �Ѵ�.

	if (argc != 3) {
		printf("Usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// Overlapped IO�� ���� ���� ����
	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&sendAddr, 0, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(argv[1]);
	sendAddr.sin_port = htons(atoi(argv[2]));

	if (connect(hSocket, (SOCKADDR*)&sendAddr, sizeof(sendAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	// Event ������Ʈ ����
	evObj = WSACreateEvent();
	
	// ��� ��Ʈ 0���� �ʱ�ȭ
	memset(&overlapped, 0, sizeof(overlapped));

	overlapped.hEvent = evObj;
	dataBuf.len = strlen(msg) + 1;
	dataBuf.buf = msg;

	// WSASend�Լ��� ��ȯ�� �� ���Ŀ��� ����ؼ� �������� ������ �̷����� ��Ȳ�̶�� SOCKET_ERROR�� ��ȯ�ϰ� �����ڵ�δ� WSA_IO_PENDING�� ��ϵȴ�.
	// ��ȯ���� �� ������ ������ �Ϸ�� ����� sendBytes�� �ٷ� ���۵� ������ ũ�Ⱑ ����.
	if (WSASend(hSocket, &dataBuf, 1, &sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			puts("Background data receive");

			// ������ ������ �Ϸ�Ǹ� Event ������Ʈ�� signaled ���°� �Ǵµ�
			// signaled ���°� �Ǿ ��ȯ�� ������ ����Ѵ�.
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);

			// ������ ���� ����� Ȯ���Ѵ�.
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
Overlapped(��ø��) IO

�ϳ��� ������ ������ ���ÿ� �� �̻��� �������� �����͸� ����(�Ǵ� ����)������ ���ؼ�, ������� ��ø�Ǵ� ��Ȳ�� ������ 'IO�� ��ø'�̶� �Ѵ�.
�̷��� ���� �����Ϸ��� ȣ��� ������Լ��� �ٷ� ��ȯ�ؾ� �Ѵ�. �׷��� �ι�°, ����° �����͸� �������� ���� �� �ִ�.
��������� �񵿱� IO�� �����ؾ� �ϰ�, �񵿱� IO�� �����Ϸ��� ������Լ��� non-blocking ���� �����ؾ� �ȴ�.

Overlapped IO ���� ����
SOCKET WSASocket(int af, int type, int protocol, LPWSAPROTOCOL_INFO lpProtocolInfo, GROUP g, DWORD dwFlags);
������ �Ű������� WSA_FLAG_OVERLAPPED�� �����ؼ� Overlapped IO�� �����ϵ��� �Ӽ� �ο�

WSASend �Լ�
int WSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutime);

������ ����, ���� ��� Ȯ�� �Լ�
BOOL WSAGetOverLappedResult(SOCKET s, LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer, BOOL fWait, LPDWORD lpdwFlags);

WSARecv �Լ�
int WSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
*/