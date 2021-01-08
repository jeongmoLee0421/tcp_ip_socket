#include <stdio.h>
#include <Windows.h>
#include <process.h>

#define NUM_THREAD 50
unsigned WINAPI threadInc(void* arg);
unsigned WINAPI threadDes(void* arg);
long long num = 0;

int main(int argc, char* argv[]) {
	HANDLE tHandles[NUM_THREAD];
	int i;

	printf("sizeof long long: %d\n", sizeof(long long));
	
	for (i = 0; i < NUM_THREAD; i++) {
		if (i % 2)
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadInc, NULL, 0, NULL);
		else
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
	}

	// 세번째 인자로 TRUE 전달 시 모든 검사대상이 signaled 상태가 되어야 반환
	// FALSE 전달 시 검사대상 중 하나라도 signaled 상태가 되면 반환
	WaitForMultipleObjects(NUM_THREAD, tHandles, TRUE, INFINITE);
	
	printf("result: %lld\n", num);
	return 0;
}

unsigned WINAPI threadInc(void* arg) {
	int i;
	for (i = 0; i < 50000000; i++)
		num += 1;
	return 0;
}

unsigned WINAPI threadDes(void* arg) {
	int i;
	for (i = 0; i < 50000000; i++)
		num -= 1;
	return 0;
}