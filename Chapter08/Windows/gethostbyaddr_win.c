#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	struct hostent* host;
	SOCKADDR_IN addr;
	char** pAlias;
	char** pAddrList;

	if (argc != 2) {
		printf("Usage %s <ip>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	host = gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET);
	if (!host)
		printf("gethostbyaddr() error");

	printf("official hostname: %s\n", host->h_name);
	
	pAlias = host->h_aliases;
	printf("aliases\n");

	// pAlias의 값(*pAlias)이 null이 아니라면 alias 문자열을 가리키고 있는 것이다.
	// 가리킨다면 가리키는 값을 출력하고 다음으로 넘어가고
	// null이어서 아무것도 가리키지 않는다면 반복문을 나간다.
	while (*pAlias) {
		printf("%s\n", *pAlias);
		pAlias++;
	}

	printf("addrtype: %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
	printf("IP addr length: %d\n", host->h_length);

	pAddrList = host->h_addr_list;
	printf("IP addr\n");
	while (*pAddrList) {
		// h_addr_list가 가리키는 것은 문자열포인터이고
		// 각각의 문자열 포인터들을 문자열의 주소값이 아니라 in_addr 구조체 변수의 주소 값이다.

		// pAddrList에 struct in_addr* 형으로 캐스팅하고 pAddrList가 in_addr 구조체 변수의 주소값을 가지고 있기 때문에
		// 실제 값을 얻기 위해서 *(포인터)를 붙여주고
		// inet_ntoa()로 보기 쉬운 문자열 ip주소로 변환한다.
		printf("%s\n", inet_ntoa(*(struct in_addr*)pAddrList));
		pAddrList++;
	}

	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}