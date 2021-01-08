#include <stdio.h>
#include <windows.h>
#include <process.h>

#define NUM_THREAD 50
unsigned WINAPI threadInc(void* arg);
unsigned WINAPI threadDes(void* arg);

long long num = 0;
// ������� ����ȭ
// �ӵ��� �������� ����� �������̴�.
CRITICAL_SECTION cs;

int main(int argc, char* argv[]) {
	HANDLE tHandles[NUM_THREAD];
	int i;

	// ������Ʈ ����
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

	// ������Ʈ �Ҹ�
	// void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	DeleteCriticalSection(&cs);
	
	printf("result: %lld\n", num);
	return 0;
}

unsigned WINAPI threadInc(void* arg) {
	int i;

	// ���� ȹ��
	// void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	EnterCriticalSection(&cs);

	for (i = 0; i < 50000000; i++)
		num += 1;

	// ���� �ݳ�
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