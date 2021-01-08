#include <stdio.h>
#include <windows.h>
#include <process.h> // _beginthreadex, _endthreadex
unsigned WINAPI ThreadFunc(void* arg);

int main(int argc, char* argv[]) {
	HANDLE hThread;
	unsigned threadID; // unsigned == int
	int param = 5;

	/*
	uintptr_t _beginthreades(
	void* security,
	unsigned stack_size,
	unsigned (*start_address)(void*),
	void* arglist,
	unsigned initflag,
	unsigned* thrdaddr
	);
	*/
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)&param, 0, &threadID);
	if (hThread == 0) {
		puts("_beginthreadex() error");
		return -1;
	}

	Sleep(3000);
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