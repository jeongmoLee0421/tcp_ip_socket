#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    char message[BUF_SIZE];
    int str_len, i, recv_len, recv_cnt;
    
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
        error_handling("accpet() error");
        
    char letter1[] = "Hello?";
    
    i = 0;
    str_len = 0;
    while(letter1[i]){
        str_len++;
        i++;
    }
    
    // write string length
    write(clnt_sock, &str_len, sizeof(str_len));
    // write data
    write(clnt_sock, &letter1, sizeof(letter1) - 1);
    
    // read string length
    read(clnt_sock, &str_len, sizeof(str_len));
    recv_len = 0;
    recv_cnt = 0;
    while(str_len > recv_len){
        recv_cnt = read(clnt_sock, &message, BUF_SIZE - 1);
        recv_len += recv_cnt;
    }
    printf("Message from client: %s\n", message);
    
    char letter2[] = "thank you!";
    
    i = 0;
    str_len = 0;
    while(letter2[i]){
        str_len++;
        i++;
    }
    
    write(clnt_sock, &str_len, sizeof(str_len));
    write(clnt_sock, &letter2, sizeof(letter2) - 1);
    
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
