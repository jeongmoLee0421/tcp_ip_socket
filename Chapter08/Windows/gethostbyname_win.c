#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	struct hostent* host;

	if (argc != 2) {
		printf("Usage %s <addr>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	host = gethostbyname(argv[1]);
	if (!host)
		ErrorHandling("gethostbyname() error");

	printf("official name: %s\n", host->h_name);

	// 공식 도메인 이름 외에 다른 도메인 이름도 확인할 수 있다.
	// char** h_aliases
	// h_aliases의 i번째 공간이 가리키는 문자열을 출력한다.
	for (int i = 0; host->h_aliases[i]; i++)
		printf("alias %d: %s\n", i + 1, host->h_aliases[i]);

	printf("addrtype: %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
	printf("IP addr length: %d\n", host->h_length);

	// ip주소도 여러개일 수 있다.(부하 분산)
	// char** h_addr_list
	// h_addr_list의 i번째 공간이 가리키는 곳에는 in_addr 구조체 변수의 주소값이 있다.
	// 이걸 (struct in_addr*)로 캐스팅해서 가리키게 하고
	// 그 가리키는 값이 주소이므로 그 주소의 값을 알기위해서 *(포인터)를 붙여준다.
	// 그럼 32비트 ipv4 정수 주소를 알게되고
	// inet_ntoa()를 사용해서 우리가 쉽게 인식하는 문자열 형태로 변환해준다.
	for (int i = 0; host->h_addr_list[i]; i++)
		printf("IP addr %d: %s\n", i + 1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));

	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
Domain Name System

DNS 서버란 도메인 이름으로 ip주소를 얻을 때 사용되는 서버이다.

내가 웹브라우저에서 어느 웹서버의 index.html문서를 가져오려고 할때
주소창에 주소를 적게되면
웹브라우저는 http 리퀘스트 메시지를 만들고 이를 보내야 하는데
상대방의 ip주소를 모르기 때문에
도메인 네임을 사용해서 ip주소를 알아와야 한다.

이때 리졸버로 DNS서버에 질의를 한다.
디폴트 DNS 서버에 해당하는 도메인 이름이 없다면 상위 DNS 서버로 이동하면서 결국 ip주소를 찾아서 알려준다.
*/