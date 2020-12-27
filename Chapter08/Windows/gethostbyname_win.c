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

	// ���� ������ �̸� �ܿ� �ٸ� ������ �̸��� Ȯ���� �� �ִ�.
	// char** h_aliases
	// h_aliases�� i��° ������ ����Ű�� ���ڿ��� ����Ѵ�.
	for (int i = 0; host->h_aliases[i]; i++)
		printf("alias %d: %s\n", i + 1, host->h_aliases[i]);

	printf("addrtype: %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
	printf("IP addr length: %d\n", host->h_length);

	// ip�ּҵ� �������� �� �ִ�.(���� �л�)
	// char** h_addr_list
	// h_addr_list�� i��° ������ ����Ű�� ������ in_addr ����ü ������ �ּҰ��� �ִ�.
	// �̰� (struct in_addr*)�� ĳ�����ؼ� ����Ű�� �ϰ�
	// �� ����Ű�� ���� �ּ��̹Ƿ� �� �ּ��� ���� �˱����ؼ� *(������)�� �ٿ��ش�.
	// �׷� 32��Ʈ ipv4 ���� �ּҸ� �˰Եǰ�
	// inet_ntoa()�� ����ؼ� �츮�� ���� �ν��ϴ� ���ڿ� ���·� ��ȯ���ش�.
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

DNS ������ ������ �̸����� ip�ּҸ� ���� �� ���Ǵ� �����̴�.

���� ������������ ��� �������� index.html������ ���������� �Ҷ�
�ּ�â�� �ּҸ� ���ԵǸ�
���������� http ������Ʈ �޽����� ����� �̸� ������ �ϴµ�
������ ip�ּҸ� �𸣱� ������
������ ������ ����ؼ� ip�ּҸ� �˾ƿ;� �Ѵ�.

�̶� �������� DNS������ ���Ǹ� �Ѵ�.
����Ʈ DNS ������ �ش��ϴ� ������ �̸��� ���ٸ� ���� DNS ������ �̵��ϸ鼭 �ᱹ ip�ּҸ� ã�Ƽ� �˷��ش�.
*/