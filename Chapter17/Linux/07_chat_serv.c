#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 256
#define MAX_CLNT_SIZE 128
#define EPOLL_SIZE 50
void setnonblockingmode(int fd);
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    int str_len, i;
    char buf[BUF_SIZE];
    
    struct epoll_event* ep_events;
    struct epoll_event event;
    int epfd, event_cnt;
    int clnt_sock_arr[MAX_CLNT_SIZE] = {0, };
    int clnt_cnt = 0;
    
    if (argc != 2){
        printf("Usage %s <port>\n", argv[0]);
        exit(1);
    }
    
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");
        
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
        
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");
        
    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    
    setnonblockingmode(serv_sock);
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);
    
    while(1){
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1){
            puts("epoll_wait() error");
            break;
        }
        
        for (i=0; i<event_cnt; i++){
            if (ep_events[i].data.fd == serv_sock){
                clnt_addr_size = sizeof(clnt_addr);
                clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
                if (clnt_sock == -1)
                    error_handling("accept() error");
                    
                // add client
                clnt_sock_arr[clnt_cnt] = clnt_sock;
                clnt_cnt++;
                    
                printf("client connect %d\n", clnt_sock);
                    
                setnonblockingmode(clnt_sock);
                event.events = EPOLLIN|EPOLLET;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
            }
            else{
                while(1){
                    str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                    if (str_len == 0){
                        epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                        
                        int j;
                        for (j = 0; j < clnt_cnt; j++){
                            if (clnt_sock_arr[j] == ep_events[i].data.fd)
                                break;
                        }
                        
                        for (int k = j; k < clnt_cnt - 1; k++){
                            clnt_sock_arr[k] = clnt_sock_arr[k+1];
                        }
                        
                        // delete client
                        clnt_cnt--;
                        
                        printf("client disconnect %d\n", ep_events[i].data.fd);
                        close(ep_events[i].data.fd);
                        break;
                    }
                    else if (str_len < 0){
                        if(errno == EAGAIN)
                            break;
                    }
                    else{
                        // send a message to all client connected to server.
                        for (int j=0; j<clnt_cnt; j++){
                            write(clnt_sock_arr[j], buf, str_len);
                        }
                    }
                }
            }
        }
    }
    close(serv_sock);
    close(epfd);
    return 0;
}

void setnonblockingmode(int fd){
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
