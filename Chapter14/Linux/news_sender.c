#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TTL 64
#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[]){
    int send_sock;
    struct sockaddr_in mul_addr;
    int time_live = TTL;
    FILE* fp;
    char buf[BUF_SIZE];
    int state;
    
    if (argc != 3){
        printf("Usage %s <groupIP> <port>\n", argv[0]);
        exit(1);
    }
    
    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (send_sock == -1)
        error_handling("socket() error");
        
    memset(&mul_addr, 0, sizeof(mul_addr));
    mul_addr.sin_family = AF_INET;
    mul_addr.sin_addr.s_addr = inet_addr(argv[1]); // Multicast IP
    mul_addr.sin_port = htons(atoi(argv[2])); // Multicast Port
    
    // multicast setting(TTL information)
    state = setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));
    if (state == -1)
        error_handling("setsockopt() error");
        
    if ((fp = fopen("news.txt", "r")) == NULL)
        error_handling("fopen() error");
        
    // feof() return nonzero value if a read operation has attempted to read past the end of the file, it returns 0 otherwise.
    // if it is EOF, return 0 and if it is read past the EOF, return nonzero(true)
    
    // fgets() return str(char*)
    // NULL is returned to indicate an error or end-of-file condition
    while (1){
        if (fgets(buf, BUF_SIZE, fp) == NULL)
            break;
        sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&mul_addr, sizeof(mul_addr));
        sleep(2); // make a time difference
    }
    
    fclose(fp);
    close(send_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
