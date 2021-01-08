#include <stdio.h>
#include <windows.h>
#include <process.h>

#define NUM_THREAD 50
unsigned WINAPI threadInc(void* arg);
unsigned WINAPI threadDes(void* arg);

long long num = 0;
// 커널모드 동기화
// 기능이 많지만 성능에 영향을 준다.
HANDLE hMutex;

int main(int argc, char* argv[]) {
	HANDLE tHandles[NUM_THREAD];
	int i;

	// mutex 오브젝트 생성
	// HANDLE CreateMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCTSTR lpName);
	hMutex = CreateMutex(NULL, FALSE, NULL);

	for (i = 0; i < NUM_THREAD; i++) {
		if (i % 2)
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadInc, NULL, 0, NULL);
		else
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
	}

	WaitForMultipleObjects(NUM_THREAD, tHandles, TRUE, INFINITE);

	// mutex 오브젝트 소멸
	// BOOL CloseHandle(HANDEL hObject);
	CloseHandle(hMutex);
	
	printf("result: %lld\n", num);
	return 0;
}

unsigned WINAPI threadInc(void* arg) {
	int i;

	// mutex는 WaitForSingleObject 함수가 반환될 때, 자동으로 non-signaled 상태가 되는 auto-reset모드 커널 오브젝트이다.
	// 따라서 WaitForSingleObject 함수가 mutex를 소유할 때(non-signaled 상태로 변경) 호출하는 함수가 된다.
	WaitForSingleObject(hMutex, INFINITE);

	for (i = 0; i < 50000000; i++)
		num += 1;

	// mutex 반납 -> signaled상태(소유자가 없다)로 변경
	ReleaseMutex(hMutex);
	
	return 0;
}

unsigned WINAPI threadDes(void* arg) {
	int i;
	
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < 50000000; i++)
		num -= 1;
	ReleaseMutex(hMutex);

	return 0;
}