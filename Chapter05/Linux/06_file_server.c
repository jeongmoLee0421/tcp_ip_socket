#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#define BUF_SIZE 1024
#define FILE_NAME_SIZE 128
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    int fd;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    char file_contents[BUF_SIZE];
    char file_name[FILE_NAME_SIZE];
    int i, recv_len, recv_cnt, str_len;
    
    if (argc != 2){
        printf("Usage %s <port>", argv[0]);
        exit(1);
    }
    
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");
        
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
        
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");
        
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");
    
    // read file_name
    read(clnt_sock, &file_name, FILE_NAME_SIZE - 1);
    
    // open file
    fd = open(file_name, O_RDONLY);
    if (fd == -1)
        error_handling("open() error");
    
    // read file_contents
    if (read(fd, &file_contents, BUF_SIZE - 1) == -1)
        error_handling("read() error");
        
    //printf("%s", file_contents);
    
    // write file_contents
    if (write(clnt_sock, &file_contents, sizeof(file_contents)) == -1)
        error_handling("write() error");
    
    close(fd);
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
