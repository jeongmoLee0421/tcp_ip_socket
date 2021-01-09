#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 100

void CompressSockets(SOCKET hSockArr[], int idx, int total);
void CompressEvents(WSAEVENT hEventArr[], int idx, int total);
void ErrorHandling(char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;

	SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT newEvent;
	WSANETWORKEVENTS netEvents; // �߻��� �̺�Ʈ�� ���������� ���������� ä������ ����ü

	int numOfClntSock = 0;
	int strLen, i;
	int posInfo, startIdx; // �̺�Ʈ �߻����� Ȯ���� ���� ����
	int clntAddrLen;
	char msg[BUF_SIZE];

	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	// hServSock�� ���� ��û�� �ִ���(FD_ACCEPT) �����ϱ� ���� newEvent ����(���� ���)
	newEvent = WSACreateEvent();
	if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
		ErrorHandling("WSAEventSelect() error");

	// ���� �׸��� ���ϰ� ����� �̺�Ʈ�� �����ϱ� ���ؼ� ������ �迭 ������ ����
	hSockArr[numOfClntSock] = hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while (1) {
		// �̺�Ʈ�� �߻��ߴ��� Ȯ��
		// �̺�Ʈ ������Ʈ�� signaled �����̸� ��ȯ
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);

		// ��ȯ�� ���� ��(posInfo)���� WSA_WAIT_EVENT_0�� ���� �� ��° �Ű������� ���޵� �迭�� �������� signaled ���°� �� Event ������Ʈ�� �ڵ��� ����� �ε����� ���ȴ�.
		// signaled ���°� �� Event ������Ʈ�� ��������� ���� ���� �ε��� ���� ���(���� ���� �ε���)
		startIdx = posInfo - WSA_WAIT_EVENT_0;

		for (i = startIdx; i < numOfClntSock; i++) {
			// �ش� �̺�Ʈ�� signaled �������� �ƴ��� Ȯ���ؼ� ������ �̺�Ʈ �߻� ������Ʈ �������� ��ȯ
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);

			// �Լ� ���� �Ǵ� Ÿ�� �ƿ��� �Ǹ� continue
			if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT)) {
				continue;
			}
			else { // ������ �߻����� �ʾҴٸ� � �̺�Ʈ�� �߻��ߴٴ� ��
				// �̺�Ʈ�� �߻��� �ε����� ����
				sigEventIdx = i;

				// ���ϰ� ����� �̺�Ʈ�� � ������ signaled ���°� �Ǿ����� newEvents ����ü�� ����
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);

				if (netEvents.lNetworkEvents & FD_ACCEPT) { // ���� ��û��
					if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0) { // ������ �߻��ϸ� 0 �̿��� ���� ����ȴ�.
						puts("Accept error");
						break;
					}

					clntAddrLen = sizeof(clntAddr);
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAddr, &clntAddrLen);
					if (hClntSock == INVALID_SOCKET)
						ErrorHandling("accept() error");

					// ������ �����Ͱ� �����ϴ���?(FD_READ), ������ ���ᰡ ��û�Ǿ�����?(FD_CLOSE)�� �����ϱ� ����
					// Ŭ���̾�Ʈ ���ϰ� �̺�Ʈ ������Ʈ�� ����
					newEvent = WSACreateEvent();
					WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

					hEventArr[numOfClntSock] = newEvent;
					hSockArr[numOfClntSock] = hClntSock;
					numOfClntSock++;
					puts("connected new client...");
				}

				if (netEvents.lNetworkEvents & FD_READ) { // ������ ���Ž�
					if (netEvents.iErrorCode[FD_READ_BIT] != 0) {
						puts("read error");
						break;
					}

					strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);
					send(hSockArr[sigEventIdx], msg, strLen, 0);
				}

				if (netEvents.lNetworkEvents & FD_CLOSE) { // ���� ��û��
					if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
						puts("close error");
						break;
					}

					// �̺�Ʈ ����
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);

					numOfClntSock--;

					// ������ ���ϰ� �̺�Ʈ ������ ����� �迭 ������ ����
					CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
					CompressEvents(hEventArr, sigEventIdx, numOfClntSock);
				}
			}
		}
	}
	WSACleanup();
	return 0;
}

void CompressSockets(SOCKET hSockArr[], int idx, int total) {
	int i;
	for (i = idx; i < total; i++)
		hSockArr[i] = hSockArr[i + 1];
}

void CompressEvents(WSAEVENT hEventArr[], int idx, int total) {
	int i;
	for (i = idx; i < total; i++)
		hEventArr[i] = hEventArr[i + 1];
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
����: �Լ��� ȣ���ϰ� ���� ��ȯ�� �� ���� ���
�񵿱�: �Լ��� ȣ���ϰ� ���� �ٷ� ���� �۾��� ����

�񵿱�� cpu�� �� �� ȿ�������� ��� ����



�񵿱� Notification IO ��
IO�� ���¿� ������� ��ȯ�� �̷����� ����� '�񵿱� Notification IO��'�̴�.

WSAEventSelect �Լ��� ȣ���ϰ� �ü���� ���ϰ� �̺�Ʈ ������Ʈ�� ����ϰ� ���� �ٷ� ��ȯ�� �ϴµ�
��ȭ�� �ִ��� �������� ���߿� Ȯ���ص� �ǰ� ���� ������ ���� �۾����� �Ѿ�� ������ �񵿱��̴�.

�׷��� select �Լ��� ��ȯ ������ IO�� �ʿ���, �Ǵ� ������ ��Ȳ�� �Ǵ� �����̱� ������
�ش� �κп��� ���ŷ�Ǿ ��ٷ��� �Ѵ�. �̰��� '���� Notification IO ��'�̴�.


1. ������ ������ ������� �̺�Ʈ �߻������� ������ ���
int WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents);
���� s���� lNetworkEvents�� ���޵� �̺�Ʈ �� �ϳ��� �߻��ϸ� hEventObject�� signaled ���·� �ٲ۴�.
"Event ������Ʈ�� ������ �����ϴ� �Լ�"
�̺�Ʈ �߻� ���¿� ��� ���� �ٷ� ��ȯ

manual-reset ����̸鼭 non-signaled ������ Event ������Ʈ �����ϴ� �Լ�
WSAEVENT WSACreateEvent(void);
Event ������Ʈ ����
WSAEVENT WSACloseEvent(WSAEVENT hEvent);

2. �̺�Ʈ �߻������� Ȯ��
DWORD WSAWaitForMultipleEvents(DWORD cEvents, const WSAEVENT* lphEvents, BOOL fWaitAll, DWORD dwTimeout, BOOL fAlertable);
������ �̺�Ʈ �߻��� ���ؼ� Event ������Ʈ�� signaled ���°� �Ǿ�� ��ȯ�ϴ� �Լ��̴�.

3. �̺�Ʈ ������ ����(Event ������Ʈ�� signaled ���°� �� ���� �ľ�)
int WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);
manual-reset ����� Event ������Ʈ�� non-signaled ���·� �ǵ����� ������ ResetEvent �Լ� ȣ���� �ʿ����� �ʴ�.
*/