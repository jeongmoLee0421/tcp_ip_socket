#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in serv_addr, your_addr;
    socklen_t your_addr_size;
    char message[BUF_SIZE];
    int str_len;
    
    if (argc != 2){
        printf("Usage %s <port>", argv[0]);
        exit(1);
    }
    
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("socket() error");
        
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
        
    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
        
    while(1){           
        // init message
        memset(message, 0, sizeof(message));
        
        your_addr_size = sizeof(your_addr);
        
        // receive message
        str_len = recvfrom(sock, message, BUF_SIZE - 1, 0, (struct sockaddr*)&your_addr, &your_addr_size);
        if (str_len == -1)
            error_handling("recvfrom() error");
        else
            printf("Message from friend(client): %s\n", message);
            
        // init message
        memset(message, 0, sizeof(message));
        
        fputs("input message: ", stdout);
        fgets(message, sizeof(message), stdin);
        
        // send message
        str_len = sendto(sock, message, strlen(message), 0, (struct sockaddr*)&your_addr, your_addr_size);
        if (str_len == -1)
            error_handling("sendto() error");
    }
    close(sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
