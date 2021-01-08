#include <stdio.h>
#include <windows.h>
#include <process.h>
#define STR_LEN 100

unsigned WINAPI NumberOfA(void* arg);
unsigned WINAPI NumberOfOthers(void* arg);

static char str[STR_LEN];
static HANDLE hEvent;

int main(int argc, char* argv[]) {
	HANDLE hThread1, hThread2;

	// Event 오브젝트 생성
	// HANDLE CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName);
	// 두번째 매개변수의 인자로 TURE 전달 시 manual-reset모드로 FALSE 전달 시 auto-reset 모드로 생성된다.
	// manual-reset모드는 WaitForSingleObject 함수가 반환한다고 해서 non-signaled 상태로 되돌려지지 않는다.
	// 세번째 매개변수의 인자로 TRUE 전달 시 signaled 상태, FALSE 전달시 non-signaled 상태로 생성
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	fputs("Input string: ", stdout);
	fgets(str, STR_LEN, stdin);

	// BOOL SetEvent(HANDLE hEvent); // signaled 상태로 변경
	SetEvent(hEvent);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	// BOOL ResetEvent(HANDLE hEvent) // non-signaled 상태로 변경
	ResetEvent(hEvent);
	CloseHandle(hEvent);
}

unsigned WINAPI NumberOfA(void* arg) {
	int i, cnt = 0;

	WaitForSingleObject(hEvent, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] == 'A')
			cnt++;
	}
	printf("Num of A: %d\n", cnt);
	return 0;
}

unsigned WINAPI NumberOfOthers(void* arg) {
	int i, cnt = 0;
	WaitForSingleObject(hEvent, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		//printf("%c\n", str[i]);
		if (str[i] != 'A')
			cnt++;
	}
	printf("Num of Others: %d\n", cnt - 1); // 개행문자 빼기
	return 0;
}

// 사용자로부터 문자열을 입력받고 나서
// SetEvent 함수에 의해 hEvent 오브젝트가 signaled 상태로 변경되고
// 변경되고 나면 두 함수 모두 진행 된다.