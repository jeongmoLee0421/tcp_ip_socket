#include <stdio.h>
#include <windows.h>
#include <process.h>
unsigned WINAPI Read(void* arg);
unsigned WINAPI Accu(void* arg);

static HANDLE semOne;
static HANDLE semTwo;
static int num;

int main(int argc, char* argv[]) {
	HANDLE hThread1, hThread2;

	// HANDLE CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCTSTR lpName);
	semOne = CreateSemaphore(NULL, 0, 1, NULL); // 자원수가 최대 1인 non-sinaled 상태(0)로 생성
	semTwo = CreateSemaphore(NULL, 1, 1, NULL); // 자원수가 최대 1인 signaled 상태(1)로 생성

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, Read, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, Accu, NULL, 0, NULL);

	// wait thread
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	CloseHandle(semOne);
	CloseHandle(semTwo);
}

unsigned WINAPI Read(void* arg) {
	int i, readData;

	for (i = 0; i < 5; i++) {
		/*fputs("Input num: ", stdout);
		WaitForSingleObject(semTwo, INFINITE);
		scanf("%d", &num);
		ReleaseSemaphore(semOne, 1, NULL);*/

		// scanf 함수와 같이 대기시간이 오래걸릴 수도 있는 경우에 세마포어 자원을 점유하고서 오랫동안 반납하지 않게되면
		// 프로그램이 오랫동안 진행되지 않을 수 있다.
		// 때문에 scnaf와 같이 사용자의 입력을 받는(대기 시간이 오래 걸릴수도 있는)경우는 임계영역에 넣지 않는 것이 좋다.
		fputs("Input num: ", stdout);
		scanf("%d", &readData); 
		WaitForSingleObject(semTwo, INFINITE);
		num = readData;
		ReleaseSemaphore(semOne, 1, NULL);
	}
	return 0;
}

unsigned WINAPI Accu(void* arg) {
	int sum = 0, i;

	for (i = 0; i < 5; i++) {
		WaitForSingleObject(semOne, INFINITE);
		sum += num;
		ReleaseSemaphore(semTwo, 1, NULL);
	}
	printf("Result: %d\n", sum);
	return 0;
}

/*
semTwo 자원을 점유하면서 수를 입력받고 semOne 자원을 반납한다.
이에 따라
semOne 자원을 점유하면서 합을 갱신하고 semTwo 자원을 반납한다.
*/