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
	WSANETWORKEVENTS netEvents; // 발생한 이벤트의 유형정보와 오류정보로 채워지는 구조체

	int numOfClntSock = 0;
	int strLen, i;
	int posInfo, startIdx; // 이벤트 발생유무 확인을 위한 변수
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

	// hServSock에 연결 요청이 있는지(FD_ACCEPT) 추적하기 위해 newEvent 연결(관찰 명령)
	newEvent = WSACreateEvent();
	if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
		ErrorHandling("WSAEventSelect() error");

	// 소켓 그리고 소켓과 연결된 이벤트를 참조하기 위해서 동일한 배열 순서로 저장
	hSockArr[numOfClntSock] = hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while (1) {
		// 이벤트가 발생했는지 확인
		// 이벤트 오브젝트가 signaled 상태이면 반환
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);

		// 반환된 정수 값(posInfo)에서 WSA_WAIT_EVENT_0를 빼면 두 번째 매개변수로 전달된 배열을 기준으로 signaled 상태가 된 Event 오브젝트의 핸들이 저장된 인덱스가 계산된다.
		// signaled 상태가 된 Event 오브젝트가 여러개라면 가장 작은 인덱스 값이 계산(가장 앞의 인덱스)
		startIdx = posInfo - WSA_WAIT_EVENT_0;

		for (i = startIdx; i < numOfClntSock; i++) {
			// 해당 이벤트가 signaled 상태인지 아닌지 확인해서 성공시 이벤트 발생 오브젝트 관련정보 반환
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);

			// 함수 실패 또는 타임 아웃이 되면 continue
			if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT)) {
				continue;
			}
			else { // 오류가 발생하지 않았다면 어떤 이벤트가 발생했다는 뜻
				// 이벤트가 발생한 인덱스를 저장
				sigEventIdx = i;

				// 소켓과 연결된 이벤트에 어떤 이유로 signaled 상태가 되었는지 newEvents 구조체에 저장
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);

				if (netEvents.lNetworkEvents & FD_ACCEPT) { // 연결 요청시
					if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0) { // 에러가 발생하면 0 이외의 값이 저장된다.
						puts("Accept error");
						break;
					}

					clntAddrLen = sizeof(clntAddr);
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAddr, &clntAddrLen);
					if (hClntSock == INVALID_SOCKET)
						ErrorHandling("accept() error");

					// 수신할 데이터가 존재하는지?(FD_READ), 연결의 종료가 요청되었는지?(FD_CLOSE)를 추적하기 위해
					// 클라이언트 소켓과 이벤트 오브젝트를 연결
					newEvent = WSACreateEvent();
					WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

					hEventArr[numOfClntSock] = newEvent;
					hSockArr[numOfClntSock] = hClntSock;
					numOfClntSock++;
					puts("connected new client...");
				}

				if (netEvents.lNetworkEvents & FD_READ) { // 데이터 수신시
					if (netEvents.iErrorCode[FD_READ_BIT] != 0) {
						puts("read error");
						break;
					}

					strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);
					send(hSockArr[sigEventIdx], msg, strLen, 0);
				}

				if (netEvents.lNetworkEvents & FD_CLOSE) { // 종료 요청시
					if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
						puts("close error");
						break;
					}

					// 이벤트 해제
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);

					numOfClntSock--;

					// 삭제된 소켓과 이벤트 정보를 지우고 배열 순서를 갱신
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
동기: 함수를 호출하고 나서 반환할 때 까지 대기
비동기: 함수를 호출하고 나서 바로 다음 작업을 실행

비동기는 cpu를 좀 더 효율적으로 사용 가능



비동기 Notification IO 모델
IO의 상태에 상관없이 반환이 이뤄지는 방식이 '비동기 Notification IO모델'이다.

WSAEventSelect 함수를 호출하고 운영체제에 소켓과 이벤트 오브젝트를 등록하고 나서 바로 반환을 하는데
변화가 있는지 없는지는 나중에 확인해도 되고 지금 당장은 다음 작업으로 넘어가기 때문에 비동기이다.

그러나 select 함수의 반환 시점은 IO가 필요한, 또는 가능항 상황이 되는 시점이기 때문에
해당 부분에서 블로킹되어서 기다려야 한다. 이것이 '동기 Notification IO 모델'이다.


1. 임의의 소켓을 대상으로 이벤트 발생여부의 관찰을 명령
int WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents);
소켓 s에서 lNetworkEvents에 전달된 이벤트 중 하나가 발생하면 hEventObject를 signaled 상태로 바꾼다.
"Event 오브젝트와 소켓을 연결하는 함수"
이벤트 발생 유뮤와 상관 없이 바로 반환

manual-reset 모드이면서 non-signaled 상태인 Event 오브젝트 생성하는 함수
WSAEVENT WSACreateEvent(void);
Event 오브젝트 종료
WSAEVENT WSACloseEvent(WSAEVENT hEvent);

2. 이벤트 발생유무의 확인
DWORD WSAWaitForMultipleEvents(DWORD cEvents, const WSAEVENT* lphEvents, BOOL fWaitAll, DWORD dwTimeout, BOOL fAlertable);
소켓의 이벤트 발생에 의해서 Event 오브젝트가 signaled 상태가 되어야 반환하는 함수이다.

3. 이벤트 종류의 구분(Event 오브젝트가 signaled 상태가 된 원인 파악)
int WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);
manual-reset 모드의 Event 오브젝트를 non-signaled 상태로 되돌리기 때문에 ResetEvent 함수 호출이 필요하지 않다.
*/