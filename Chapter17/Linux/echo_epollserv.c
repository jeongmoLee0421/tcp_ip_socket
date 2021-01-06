#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 100
#define EPOLL_SIZE 50
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
    
    if (argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
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

    // creating a repository of file descriptors        
    epfd = epoll_create(EPOLL_SIZE);
    
    // dynamic allocation
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    
    /*
    struct epoll_event{
        __uint32_t events;
        epoll_data_t data;
    }
    */
    
    event.events = EPOLLIN; // there is data to be received
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event); // register serv_sock in epfd for event observation purpose
    
    while(1){
        // The select function must deliver all the observation target information to the operating system every time,
        // but the epoll function only delivers changes to the observation target information.
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
                    
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event); // register clnt_sock in epfd for event observation purpose
                printf("connected client: %d\n", clnt_sock);
            }
            else{
                str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                if (str_len == 0){ // close request
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL); // cancel registration
                    close(ep_events[i].data.fd); // close fd(socket)
                    printf("closed client: %d\n", ep_events[i].data.fd);
                }
                else{
                    write(ep_events[i].data.fd, buf, str_len); // echo
                }
            }
        }
    }
    close(serv_sock);
    close(epfd); // close epoll fd
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

/*
epoll <-> select

To check the state change, there is no need for a loop over the entire file descriptor. (check only the changes)

When calling the epoll_wait function, there is no need to deliver the information of the observation object every time. (only transfer changes)
*/

/*
int epoll_create(int size);

int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);

int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);
*/
