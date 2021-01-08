#include <stdio.h>
#include <windows.h>
#include <process.h>

#define NUM_THREAD 50
unsigned WINAPI threadInc(void* arg);
unsigned WINAPI threadDes(void* arg);

long long num = 0;
// 유저모드 동기화
// 속도가 빠르지만 기능이 제한적이다.
CRITICAL_SECTION cs;

int main(int argc, char* argv[]) {
	HANDLE tHandles[NUM_THREAD];
	int i;

	// 오브젝트 생성
	// void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	InitializeCriticalSection(&cs);

	for (i = 0; i < NUM_THREAD; i++) {
		if (i % 2)
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadInc, NULL, 0, NULL);
		else
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
	}

	// wait thread
	WaitForMultipleObjects(NUM_THREAD, tHandles, TRUE, INFINITE);

	// 오브젝트 소멸
	// void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	DeleteCriticalSection(&cs);
	
	printf("result: %lld\n", num);
	return 0;
}

unsigned WINAPI threadInc(void* arg) {
	int i;

	// 열쇠 획득
	// void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	EnterCriticalSection(&cs);

	for (i = 0; i < 50000000; i++)
		num += 1;

	// 열쇠 반납
	// void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	LeaveCriticalSection(&cs);
	
	return 0;
}

unsigned WINAPI threadDes(void* arg) {
	int i;
	
	EnterCriticalSection(&cs);
	for (i = 0; i < 50000000; i++)
		num -= 1;
	LeaveCriticalSection(&cs);

	return 0;
}