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
    FILE* readfp;
    FILE* writefp;
    
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    char buf[BUF_SIZE] = {0, };
    
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
        
    // fd(clnt_sock) -> fp
    readfp = fdopen(clnt_sock, "r");
    
    // duplicate fd(clnt_sock)
    writefp = fdopen(dup(clnt_sock), "w");
    
    fputs("From server: hi client?\n", writefp);
    fputs("Nice to meet you\n", writefp);
    fputs("You are awesome!\n", writefp);
    fflush(writefp);
    
    // shutdown write stream (read stream is alive)
    // half-close and send EOF
    shutdown(fileno(writefp), SHUT_WR);
    
    // close writefp -> close dup(clnt_sock)
    // alive(readfp -> clnt_sock)
    // socket is alive
    // because socket are also destroyed only when all file descriptors are destroyed
    fclose(writefp);
    
    fgets(buf, sizeof(buf), readfp);
    fputs(buf, stdout);
    
    // socket is destroyed
    fclose(readfp);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
