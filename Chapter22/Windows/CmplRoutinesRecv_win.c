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

	// Overlapped IO ���� ����
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
	evObj = WSACreateEvent(); // ������ �ʴ� �̺�Ʈ ������Ʈ(Completion Routine�Լ� ����ϱ� ����)

	// Completion Routine�� �̿��ϴ��� 6��° ���ڷ� WSAOVERLAPPED ����ü ������ �ּҰ��� �Ѱܾ� �Ѵ�.
	// ������ Event ������Ʈ�� �����ؼ� ���� �ʿ�� ����.
	if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, CompRoutine) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING)
			puts("Background data receive");
	}

	// WSAWaitForMultipleEvents �Լ��� ȣ���ϸ� alertable wait���¿� ���̰� �ǰ� �ü���� Completion Routine�� ȣ���ϰ� �ȴ�.
	// ������ Event ������Ʈ�� ������ ������ �ʱ� ������ signaled ���·� ����� ���� ���� WSA_INFINITE�� ������ ���� ��⸦ ������
	// Completion Routine�� ����Ǹ鼭 �� �Լ��� WAIT_IO_COMPLETION�� ��ȯ�ϸ鼭 �������´�.
	idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE);
	if (idx == WAIT_IO_COMPLETION) // IO�� ����ϷḦ �ǹ�
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
	// ù��° �Ű����� = ��������
	// �ι�° �Ű����� = �Ϸ�� �������� ����� ũ�� ����
	// ����° �Ű����� = WSASend, WSARecv �Լ��� �Ű����� lpOverlapped�� ���޵� ��
	// ������ �Ű����� = ����� �Լ�ȣ��� ���޵� Ư������ �Ǵ� 0�� ����
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
<WSASend, WSARecv �Լ��� ������ �������ڸ� ���ؼ� ��ϵǴ�, Completion Routine�̶� �Ҹ��� �Լ��� ���ؼ� ����� �Ϸ� Ȯ��>
Pending�� IO�� �Ϸ�Ǹ�, �� �Լ��� ȣ���� �޶�� �ü������ ��û�Ѵ�.

�߿��� �۾��� �����߿��� Completion Routine�� ȣ��Ǹ� ���α׷��� �帧�� ��ĥ �� �ֱ� ������
�ü���� IO�� ��û�� �����尡 alerable wait ���¿� �������� ���� Completion Routine�� ȣ���Ѵ�.

alertable wait ���´� �ü���� �����ϴ� �޽����� ������ ����ϴ� �������� ���¸� ���Ѵ�.
WaitForSingleObjectEx
WaitForMultipleOvjectsEx
WSAWaitForMultipleEvents
SleepEx

Completion Routine�� ����Ǹ� �� �Լ����� ��� WAIT_IO_COMPLETION�� ��ȯ�ϸ鼭 �Լ��� ���� ���´�.
�׸��� �� �������� ������ �̾� ������.
*/

/*
�񵿱� Notification IO ���� ��쿡�� IO���� �̺�Ʈ�� �߻��� �˸��� ������ �񵿱�� ó���ȴ�.
Overlapped IO ���� ���� IO�� �Ϸ�� ��Ȳ�� Ȯ���ϴ� ������ �񵿱�� ó���ȴ�.

�񵿱� IO�� IO�� �Ϸ�� ��Ȳ�� Ȯ���ϴ� ������ �񵿱�� ó������ �ǹ��Ѵ�.
�̷��� �������� IO�� �����ϱ� ���ؼ��� IO�� non-blocking ���� �����ؾ� �ϸ�
IO�� non-blocking ���� �����ؼ� �񵿱�� IO�� �����ϰ� �Ǹ�, �̸� �������� IO�� ��ø��Ų ������ Overlapped IO�� ����������.
*/