#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h> // IPPROTO_TCP
#include <netinet/tcp.h> // TCP_NODELAY

#define TRUE 1
#define FALSE 0
void error_handling(char* message);

int main(int argc, char* argv[]){
    int sock;
    int nagle, state;
    socklen_t len;
    
    // nagle algorithm
    // the nagle algorithm transmits the next data after receiving an ACK message for the previously transmitted data.
    // strength: reduced load on network traffic
    // weakness: the data transfer rate is not very fast.
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");
        
    len = sizeof(nagle);
    state = getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&nagle, &len);
    if (state)
        error_handling("getsockopt() error");
    printf("nagle algorithm: %d\n", nagle);
    
    nagle = TRUE;
    state = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&nagle, sizeof(nagle));
    if (state)
        error_handling("setsockopt() error");
    printf("nagle algorithm: %d\n", nagle);
    
    close(sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
