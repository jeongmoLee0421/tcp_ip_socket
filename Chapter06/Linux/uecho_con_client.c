#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char* message);

// connected UDP socket
// when sending and receiving data to one host
int main(int argc, char* argv[]){
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_addr;
    
    if (argc != 3){
        printf("Usage %s <ip> <port>", argv[0]);
        exit(1);
    }
    
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("socket() error");
        
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    // if you use the connect function
    // you can use write and read function
    
    // when calling the connect function
    // destination ip and port number are registered in udp socket
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    while(1){
        fputs("Insert message(q to quit): ", stdout);
        fgets(message, sizeof(message), stdin);
        
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;
        
        // destination information has already been assigned to the socket
        write(sock, message, strlen(message));
        
        str_len = read(sock, message, sizeof(message) - 1);
        
        message[str_len] = 0;
        printf("Message from server: %s", message);
    }
    close(sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
