#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TRUE 1
#define FALSE 0
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size, optlen;
    char message[30];
    int option, str_len, state;
    
    if (argc != 2){
        printf("Usage %s <port>\n", argv[0]);
        exit(1);
    }
    
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");
        
    // the port number for the time-wate stae socket is assigned to the newly started socket.
    optlen = sizeof(option);
    option = TRUE;
    state = setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);
        
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    // Time-wait
    // if host A send the last message to host B and destroys the socket immediately,
    // the ACK message is destroyed in the middle.
    // Eventually host B tries to retransmit, but since host A's socket is completely shut down,
    // it won't get an ACK message forever.
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
        
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");
        
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");
        
    while((str_len = read(clnt_sock, message, sizeof(message))) != 0){
        write(clnt_sock, message, str_len);
        
        // if fd is 1, Standard Output
        write(1, message, str_len);
    }
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
