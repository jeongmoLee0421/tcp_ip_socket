#include <stdio.h>
#define BUF_SIZE 1024
void int_to_char_memcpy(void* dest, const void* src, int len);

int main(void) {
	char message[BUF_SIZE];
	char letter[] = "Hello?";
	int letter_size = 0;

	// 구분하기 좋게 1로 초기화
	for (int i = 0; i < BUF_SIZE; i++) message[i] = 1;
	
	// letter의 바이트 수 계산
	int i = 0;
	while (letter[i]) {
		letter_size++;
		i++;
	}

	// letter의 크기가 담긴 공간(letter_size)을 가리키는 포인터
	// int*형으로 4byte를 가리킨다.
	// letter_size = 257;
	int* letter_size_ptr = &letter_size;
	int_to_char_memcpy(message, letter_size_ptr, sizeof(int));

	for (int i = 0; i < 8; i++) printf("%d ", message[i]);
}

// int형 변수를 char형 배열에 집어 넣는 함수
void int_to_char_memcpy(void* dest, const void* src, int len) {
	char* _dest = (char*)dest;
	// int*로 4byte씩 가리키는 것을 char*로 캐스팅해서 1byte씩 가리키게 한다.
	char* _src = (char*)src;

	// _dest, _src 모두 1byte를 가리키기 때문에 한바이트씩 복사
	for (int i = 0; i < len; i++) _dest[i] = _src[i];
}