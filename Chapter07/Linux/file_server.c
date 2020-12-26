#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    FILE* fp;
    char buf[BUF_SIZE];
    int read_cnt;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    
    if (argc != 2){
        printf("Usage %s <ip>", argv[0]);
        exit(1);
    }
    
    fp = fopen("file_server.c", "rb");
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
        
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");
        
    clnt_addr_size = sizeof(clnt_addr_size);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accpet() error");
        
    while(1){
        // read BUF_SIZE with a file pointer and store it in a buf
        read_cnt = fread((void*)buf, 1, BUF_SIZE, fp);
        
        printf("read_cnt: %d\n", read_cnt);
        
        // if you read less than BUF_SIZE
        // this is end of the file
        if (read_cnt < BUF_SIZE){
            write(clnt_sock, buf, read_cnt);
            break;
        }
        
        // if you read as much as BUF_SIZE
        write(clnt_sock, buf, BUF_SIZE);
    }
    
    // Half-close
    // shudown output stream
    shutdown(clnt_sock, SHUT_WR);
    
    read(clnt_sock, buf, BUF_SIZE);
    printf("Message from client: %s\n", buf);
    
    fclose(fp);
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
