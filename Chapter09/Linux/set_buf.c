#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
void error_handling(char* message);

int main(int argc, char* argv[]){
    int sock;
    int snd_buf = 1024 * 3, rcv_buf = 1024 * 3; // 3Mbyte
    int state;
    socklen_t len;
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");
        
    // get the value from the snd_buf and set the socket option
    state = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void*)&snd_buf, sizeof(snd_buf));
    if (state)
        error_handling("getsockopt() error");
        
    state = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&rcv_buf, sizeof(rcv_buf));
    if (state)
        error_handling("setsockopt() error");
        
    // get the value from the socket option and save to snd_buf
    len = sizeof(snd_buf);
    state = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void*)&snd_buf, &len);
    if (state)
        error_handling("getsockopt() error");
    printf("Output buffer size: %d\n", snd_buf);
    
    len = sizeof(rcv_buf);
    state = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&rcv_buf, &len);
    if (state)
        error_handling("setsockopt() error");
    printf("Input buffer size: %d\n", rcv_buf);
    
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
