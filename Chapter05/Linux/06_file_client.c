#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024
#define FILE_NAME_SIZE 128
void error_handling(char* message);

int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    char file_contents[BUF_SIZE];
    char file_name[FILE_NAME_SIZE];
    int i, recv_len, recv_cnt;
    
    if (argc != 3){
        printf("Usage %s <ip> <port>", argv[0]);
        exit(1);
    }
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");
        
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");
        
    printf("input file name: ");
    scanf("%s", file_name);
    
    // write file_name
    if (write(sock, &file_name, sizeof(file_name)) == -1)
        error_handling("write() error");
    
    // read file_contents
    if (read(sock, &file_contents, BUF_SIZE - 1) == -1)
        error_handling("read() error");
    
    printf("file from server: %s\n", file_contents);
    
    close(sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
