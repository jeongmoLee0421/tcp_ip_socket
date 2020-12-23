#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024
#define ARR_SIZE 100
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    int operandCnt;
    int operandArr[ARR_SIZE];
    char operator;
    int result;
    
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
    
    // first, read operand count    
    if (read(clnt_sock, &operandCnt, sizeof(operandCnt)) == -1)
        error_handling("read() error");
        
    // second, read operand 
    for (int i=0; i<operandCnt; i++){
        if (read(clnt_sock, &operandArr[i], sizeof(operandArr[i])) == -1)
            error_handling("read() error");
    }
    
    // third, read operator
    if (read(clnt_sock, &operator, sizeof(operator)) == -1)
        error_handling("read() error");
        
    switch(operator){
        case '+':
            result = 0;
            for(int i=0; i<operandCnt; i++){
                result += operandArr[i];
            }
            break;
            
        case '-':
            result = operandArr[0];
            for(int i=1; i<operandCnt; i++){
                result -= operandArr[i];
            }
            break;
            
        case '*':
            result = 1;
            for(int i=0; i<operandCnt; i++){
                result *= operandArr[i];
            }
    }
    
    if (write(clnt_sock, &result, sizeof(result)) == -1)
        error_handling("write() error");
    
    close(serv_sock);
    close(clnt_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
