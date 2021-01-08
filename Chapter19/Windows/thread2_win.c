#include <stdio.h>
#include <windows.h>
#include <process.h>
unsigned WINAPI ThreadFunc(void* arg);

int main(int argc, char* argv[]) {
	HANDLE hThread;
	DWORD wr;
	unsigned threadID;
	int param = 5;

	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)&param, 0, &threadID);
	if (hThread == 0) {
		puts("_beginthreadex() error");
		return -1;
	}

	// DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
	// 두번째 인자에 INFINITE 전달시 커널 오브젝트가 signaled 상태(쓰레드가 종료된 상태)가 되기 전에는 반환하지 않는다.
	if ((wr = WaitForSingleObject(hThread, INFINITE)) == WAIT_FAILED) {
		puts("thread wait error");
		return -1;
	}

	printf("wait result: %s\n", (wr == WAIT_OBJECT_0) ? "signaled" : "time-out");
	puts("end of main");
	return 0;
}

unsigned WINAPI ThreadFunc(void* arg) {
	int i;
	int cnt = *((int*)arg);
	for (i = 0; i < cnt; i++) {
		Sleep(1000);
		puts("running thread");
	}
	return 0;
}