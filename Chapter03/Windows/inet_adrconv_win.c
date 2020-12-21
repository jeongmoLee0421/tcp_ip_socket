#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	// inet_addr
	// success, return converted 32bit int(big endian)
	char* addr = "127.212.124.78";
	unsigned long conv_addr = inet_addr(addr);
	if (conv_addr == INADDR_NONE)
		printf("Error occured!\n");
	else
		printf("Network ordered integer addr: %#lx\n", conv_addr);

	// inet_pton
	// success, information is assigned to the address
	SOCKADDR_IN addr_inet;
	memset(&addr_inet, 0, sizeof(addr_inet));
	if (!inet_pton(AF_INET, addr, &addr_inet.sin_addr))
		ErrorHandling("inet_pton() error!\n");
	else
		printf("Network ordered integer addr: %#x\n", addr_inet.sin_addr.s_addr);

	// inet_ntoa
	// success, return address value of converted string
	struct sockaddr_in addr_in;
	char* strPtr;
	char strArr[20];

	addr_in.sin_addr.s_addr = htonl(0x1020304);
	strPtr = inet_ntoa(addr_in.sin_addr);
	strcpy_s(strArr, sizeof(strArr), strPtr);
	printf("Dotted-Decimal notation3 %s\n", strArr);

	// inet_ntop
	// success, converted string is assigned to buf
	if (inet_ntop(AF_INET, &addr_in.sin_addr.s_addr, strArr, sizeof(strArr)) != NULL)
		printf("%s\n", strArr);
	else
		ErrorHandling("inet_ntop() error!\n");

	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}