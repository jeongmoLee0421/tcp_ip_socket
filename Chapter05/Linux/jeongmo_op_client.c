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
    int operandCnt;
    char operator;
    int result;
    struct sockaddr_in serv_addr;
    
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
        printf("connected.....................\n");
        
    printf("Operand count: ");
    scanf("%d", &operandCnt);
    
    // first, write operand count
    if (write(sock, &operandCnt, sizeof(operandCnt)) == -1)
        error_handling("write() error");
    
    // second, write operand
    for (int i=0; i<operandCnt; i++){
        int num;
        printf("Operand %d: ", i + 1);
        scanf("%d", &num);
        
        if (write(sock, &num, sizeof(num)) == -1)
            error_handling("write() error");
    }
    
    // beware of newline characters remaining in the buffer
    // i received a space and characters together
    printf("Operator: ");
    scanf(" %c", &operator);
    
    if (write(sock, &operator, sizeof(operator)) == -1)
        error_handling("write() error");
    
    if (read(sock, &result, sizeof(result)) == -1)
        error_handling("read() error");
        
    printf("Operation result: %d\n", result); 
    
    close(sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
