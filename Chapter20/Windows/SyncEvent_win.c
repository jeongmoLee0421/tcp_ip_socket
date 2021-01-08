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

	// Event ������Ʈ ����
	// HANDLE CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName);
	// �ι�° �Ű������� ���ڷ� TURE ���� �� manual-reset���� FALSE ���� �� auto-reset ���� �����ȴ�.
	// manual-reset���� WaitForSingleObject �Լ��� ��ȯ�Ѵٰ� �ؼ� non-signaled ���·� �ǵ������� �ʴ´�.
	// ����° �Ű������� ���ڷ� TRUE ���� �� signaled ����, FALSE ���޽� non-signaled ���·� ����
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	fputs("Input string: ", stdout);
	fgets(str, STR_LEN, stdin);

	// BOOL SetEvent(HANDLE hEvent); // signaled ���·� ����
	SetEvent(hEvent);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	// BOOL ResetEvent(HANDLE hEvent) // non-signaled ���·� ����
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
	printf("Num of Others: %d\n", cnt - 1); // ���๮�� ����
	return 0;
}

// ����ڷκ��� ���ڿ��� �Է¹ް� ����
// SetEvent �Լ��� ���� hEvent ������Ʈ�� signaled ���·� ����ǰ�
// ����ǰ� ���� �� �Լ� ��� ���� �ȴ�.