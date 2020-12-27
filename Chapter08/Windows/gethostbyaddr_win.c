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

	// pAlias�� ��(*pAlias)�� null�� �ƴ϶�� alias ���ڿ��� ����Ű�� �ִ� ���̴�.
	// ����Ų�ٸ� ����Ű�� ���� ����ϰ� �������� �Ѿ��
	// null�̾ �ƹ��͵� ����Ű�� �ʴ´ٸ� �ݺ����� ������.
	while (*pAlias) {
		printf("%s\n", *pAlias);
		pAlias++;
	}

	printf("addrtype: %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
	printf("IP addr length: %d\n", host->h_length);

	pAddrList = host->h_addr_list;
	printf("IP addr\n");
	while (*pAddrList) {
		// h_addr_list�� ����Ű�� ���� ���ڿ��������̰�
		// ������ ���ڿ� �����͵��� ���ڿ��� �ּҰ��� �ƴ϶� in_addr ����ü ������ �ּ� ���̴�.

		// pAddrList�� struct in_addr* ������ ĳ�����ϰ� pAddrList�� in_addr ����ü ������ �ּҰ��� ������ �ֱ� ������
		// ���� ���� ��� ���ؼ� *(������)�� �ٿ��ְ�
		// inet_ntoa()�� ���� ���� ���ڿ� ip�ּҷ� ��ȯ�Ѵ�.
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