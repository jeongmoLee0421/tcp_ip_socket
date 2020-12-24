#include <stdio.h>
#define BUF_SIZE 1024
void int_to_char_memcpy(void* dest, const void* src, int len);

int main(void) {
	char message[BUF_SIZE];
	char letter[] = "Hello?";
	int letter_size = 0;

	// �����ϱ� ���� 1�� �ʱ�ȭ
	for (int i = 0; i < BUF_SIZE; i++) message[i] = 1;
	
	// letter�� ����Ʈ �� ���
	int i = 0;
	while (letter[i]) {
		letter_size++;
		i++;
	}

	// letter�� ũ�Ⱑ ��� ����(letter_size)�� ����Ű�� ������
	// int*������ 4byte�� ����Ų��.
	// letter_size = 257;
	int* letter_size_ptr = &letter_size;
	int_to_char_memcpy(message, letter_size_ptr, sizeof(int));

	for (int i = 0; i < 8; i++) printf("%d ", message[i]);
}

// int�� ������ char�� �迭�� ���� �ִ� �Լ�
void int_to_char_memcpy(void* dest, const void* src, int len) {
	char* _dest = (char*)dest;
	// int*�� 4byte�� ����Ű�� ���� char*�� ĳ�����ؼ� 1byte�� ����Ű�� �Ѵ�.
	char* _src = (char*)src;

	// _dest, _src ��� 1byte�� ����Ű�� ������ �ѹ���Ʈ�� ����
	for (int i = 0; i < len; i++) _dest[i] = _src[i];
}