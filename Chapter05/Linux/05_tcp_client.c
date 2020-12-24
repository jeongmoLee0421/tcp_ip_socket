#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024
void error_handling(char* message);

int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    int str_len, i, recv_len, recv_cnt;
    char message[BUF_SIZE];
    
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
    else
        puts("Connected.................");
        
    // read string length
    read(sock, &str_len, sizeof(str_len));
    
    recv_len = 0;
    recv_cnt = 0;
    while (str_len > recv_len){
        recv_cnt = read(sock, &message, BUF_SIZE - 1);
        recv_len += recv_cnt;
    }
    printf("Message from server: %s\n", message);
    
    char letter[] = "nice to meet you!";
    i = 0;
    str_len = 0;
    while(letter[i]){
        str_len++;
        i++;
    }
    
    // write string length
    write(sock, &str_len, sizeof(str_len));
    // write data
    write(sock, &letter, sizeof(letter) - 1);
    
    str_len = 0;
    read(sock, &str_len, sizeof(str_len));
    
    // must be initialize
    // it may contain garbage data.
    memset(&message, 0, sizeof(message));
    recv_len = 0;
    recv_cnt = 0;
    while(str_len > recv_len){
        recv_cnt = read(sock, &message, BUF_SIZE - 1);
        recv_len += recv_cnt;
    }
    printf("Message from server: %s\n", message);
    
    close(sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
