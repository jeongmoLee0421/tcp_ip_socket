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
	semOne = CreateSemaphore(NULL, 0, 1, NULL); // �ڿ����� �ִ� 1�� non-sinaled ����(0)�� ����
	semTwo = CreateSemaphore(NULL, 1, 1, NULL); // �ڿ����� �ִ� 1�� signaled ����(1)�� ����

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

		// scanf �Լ��� ���� ���ð��� �����ɸ� ���� �ִ� ��쿡 �������� �ڿ��� �����ϰ� �������� �ݳ����� �ʰԵǸ�
		// ���α׷��� �������� ������� ���� �� �ִ�.
		// ������ scnaf�� ���� ������� �Է��� �޴�(��� �ð��� ���� �ɸ����� �ִ�)���� �Ӱ迵���� ���� �ʴ� ���� ����.
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
semTwo �ڿ��� �����ϸ鼭 ���� �Է¹ް� semOne �ڿ��� �ݳ��Ѵ�.
�̿� ����
semOne �ڿ��� �����ϸ鼭 ���� �����ϰ� semTwo �ڿ��� �ݳ��Ѵ�.
*/