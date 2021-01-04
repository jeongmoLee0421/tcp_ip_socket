#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[]){
    int send_sock;
    struct sockaddr_in broad_addr;
    FILE* fp;
    char buf[BUF_SIZE];
    int so_brd = 1;
    int state;
    
    if (argc != 3){
        printf("Usage: %s <broadcast IP> <port>\n", argv[0]);
        exit(1);
    }
    
    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (send_sock == -1)
        error_handling("socket() error");
        
    memset(&broad_addr, 0, sizeof(broad_addr));
    broad_addr.sin_family = AF_INET;
    broad_addr.sin_addr.s_addr = inet_addr(argv[1]);
    broad_addr.sin_port = htons(atoi(argv[2]));
    
    // broadcast setting
    state = setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST, (void*)&so_brd, sizeof(so_brd));
    if (state == -1)
        error_handling("setsockopt() error");
        
    if ((fp = fopen("news.txt", "r")) == NULL)
        error_handling("fopen() error");
        
     while(1){
         if (fgets(buf, BUF_SIZE, fp) == NULL)
             break;
             
         sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&broad_addr, sizeof(broad_addr));
         sleep(2);
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

/*
multicast <-> broadcast

it is similar in that it sends data to multiple hosts at once

In multicast, even if the host is on a different network, joining a group can receive data.
On the other hand, broadcast can only be received by hosts connected to the same network.
*/

/*
Directed broadcast <-> Local broadcast

Directed broadcast
Data can be transmitted to a network in a specific area.
vmware network8 = 192.168.217.1
use 192.168.217.255

Local broadcast
It can be transmitted on the same network as the sender.
use 255.255.255.255
*/
