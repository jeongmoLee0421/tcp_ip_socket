#include <stdio.h>
#include <windows.h>
#include <process.h>
#define STR_LEN 100

unsigned WINAPI NumberOfA(void* arg);
unsigned WINAPI NumberOfOthers(void* arg);

static char str[STR_LEN];
static HANDLE sema;

int main(int argc, char* argv[]) {
	HANDLE hThread1, hThread2;
	sema = CreateSemaphore(NULL, 0, 2, NULL); // 세마포어 초기 자원 0개, 최대 자원 2개(스레드 2개가 동시에 접근 가능)

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	fputs("Input string: ", stdout);
	fgets(str, STR_LEN, stdin);

	ReleaseSemaphore(sema, 2, NULL); // 자원 두개 반납
	// 어차피 str변수를 읽기 동작만 하지 쓰기 동작은 하지 않기 때문에 두 쓰레드가 접근해도 괜찮다

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	CloseHandle(sema);
	return 0;
}

unsigned WINAPI NumberOfA(void* arg) {
	int i, cnt = 0;

	WaitForSingleObject(sema, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] == 'A')
			cnt++;
	}
	printf("Num of A: %d\n", cnt);

	ReleaseSemaphore(sema, 1, NULL);
	return 0;
}

unsigned WINAPI NumberOfOthers(void* arg) {
	int i, cnt = 0;

	WaitForSingleObject(sema, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] != 'A')
			cnt++;
	}
	printf("Num of Others: %d\n", cnt - 1);

	ReleaseSemaphore(sema, 1, NULL);
	return 0;
}