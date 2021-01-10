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

	// Overlapped IO ���� ����
	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hLisnSock == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	// non-blocking ��� ���� ����
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
		SleepEx(100, TRUE); // Completion Routine�� �����ϱ� ���� alertable wait ����

		// non-blocking �Ӽ��� ������ ���� ���� �����Ǵ� ���ϵ� non-blocking �Ӽ��� ������.
		hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAddr, &recvAddrSize);
		if (hRecvSock == INVALID_SOCKET) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) // Ŭ���̾�Ʈ �����û�� �������� �ʴ� ���¿��� accept�Լ� ȣ��� INVALID_SOCKET�� �ٷ� ��ȯ�Ǹ�, WSAEWOULDBLOCK�� ��ȯ�ȴ�.
				continue;
			else // accept �Լ� ȣ�� ������
				ErrorHandling("accpet() error");
		}
		puts("client connected....");

		// Ŭ���̾�Ʈ �ϳ��� WSAOVERPALLED ����ü ������ �ϳ��� �Ҵ��ؾ� �ϱ� ����
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		hbInfo->hClntSock = (DWORD)hRecvSock;
		(hbInfo->wsaBuf).buf = hbInfo->buf;
		(hbInfo->wsaBuf).len = BUF_SIZE;

		// Completion Routine ����� Overlapped IO������ Event ������Ʈ�� ���ʿ��ϱ� ������ hEvent�� �ʿ��� �ٸ� ������ ä���� �ȴ�.
		// ���⼭�� hbInfo�� ä���µ� �̴� Completion Routine �Լ� ���η� ���޵ȴ�(������° �Ű����� WSAOVERLAPPED ����ü�� Completion Routine �Լ��� ����° �Ű������� ����)
		lpOvLp->hEvent = (HANDLE)hbInfo;

		// ������� �Ϸ�Ǹ� ReadCompRoutine �Լ��� ȣ��
		// ������° �Ű����� WSAOVERLAPPED ����ü ������ �ּҰ��� Completion Routine�� ����° �Ű������� ����
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

	if (szRecvBytes == 0) { // ��������(EOF)
		closesocket(hSock);
		
		// �����Ҵ��� lpOvLp�� hbInfo(hEvent�� ����Ǿ� �Ѿ��)�� �޸𸮸� �����Ѵ�.
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

	// non-blcoking ���� �����Ͱ� ���ŵǱ⸦ ��ٸ���.
	WSARecv(hSock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
1. Ŭ���̾�Ʈ�� ����Ǹ� WSARecv �Լ��� ȣ���ϸ鼭 non-blocking ���� �����Ͱ� ���ŵǰ� �ϰ�, ������ �Ϸ�Ǹ� ReadCompRoutine �Լ��� ȣ��ǰ� �Ѵ�.
2. ReadCompRoutine �Լ��� ȣ��Ǹ� WSASend �Լ��� ȣ���ϸ鼭 non-blocking ���� �����Ͱ� �۽ŵǰ� �ϰ�(echo), �۽��� �Ϸ�Ǹ� WriteCompRoutine �Լ��� ȣ��ǰ� �Ѵ�.
3. WirteCompRoutine �Լ��� ȣ��Ǹ� WSARecv �Լ��� ȣ���ϸ鼭 non-blocking ���� �������� ������ ��ٸ���.

ReadCompRoutine �Լ��� WriteCompRoutine �Լ��� ������ ȣ��Ǹ鼭 �������� ����, �۽��� �ݺ��Ѵ�.



Overlapped IO ���� ���� ������ ����
non-blocking ����� accept�Լ��� alertable wait ���·��� ������ ���� SleepEx �Լ��� ������ ���� �ݺ� ȣ��Ǵ� ���� ���ɿ� ������ ��ĥ �� �ִ�.

�ٸ� ���
accept �Լ��� ȣ���� main�����尡 ó���ϵ��� �ϰ�, ������ �����带 �߰��� �ϳ� �����ؼ� Ŭ���̾�Ʈ���� ������� ����ϰ� �Ѵ�.(IOCP)
IO�� �����ϴ� �����带 ������ ����


Overlapped IO�� IOCP�� ���� �߿��� ��
1. �Է°� ����� non-blocking ���� �����ϴ°�?
2. non-blocking ���� ����� �Է°� ����� �Ϸ�� ��� Ȯ���ϴ°�?
*/