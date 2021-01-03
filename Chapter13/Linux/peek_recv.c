#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[]){
    int acpt_sock, recv_sock;
    struct sockaddr_in acpt_addr, recv_addr;
    int str_len, state;
    socklen_t recv_addr_size;
    char buf[BUF_SIZE];
    
    if (argc != 2){
        printf("Usage %s <port>\n", argv[0]);
        exit(1);
    }
    
    acpt_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (acpt_sock == -1)
        error_handling("socket() error");
        
    memset(&acpt_addr, 0, sizeof(acpt_addr));
    acpt_addr.sin_family = AF_INET;
    acpt_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    acpt_addr.sin_port = htons(atoi(argv[1]));
    
    if (bind(acpt_sock, (struct sockaddr*)&acpt_addr, sizeof(acpt_addr)) == -1)
        error_handling("bind() error");
        
    if (listen(acpt_sock, 5) == -1)
        error_handling("listen() error");
        
    recv_addr_size = sizeof(recv_addr);
    recv_sock = accept(acpt_sock, (struct sockaddr*)&recv_addr, &recv_addr_size);
    if (recv_sock == -1)
        error_handling("accept() error");
        
    // MSG_PEEK: check if the received data exists in the input buffer.
    // when reading data from the input buffer, the data is not deleted from the input buffer.
    
    // MSG_DONTWAIT: even if there is no data received, it does not enter the block state.
    while(1){
        str_len = recv(recv_sock, buf, sizeof(buf) - 1, MSG_PEEK|MSG_DONTWAIT);
        if (str_len > 0)
            break;
    }
    
    buf[str_len] = 0;
    printf("Buffering %s\n", buf);
    
    // since i used MSG_PEEK above, the data remains in the buffer.
    str_len = recv(recv_sock, buf, sizeof(buf) - 1, 0);
    buf[str_len] = 0;
    printf("Read again: %s\n", buf);
    
    close(acpt_sock);
    close(recv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
