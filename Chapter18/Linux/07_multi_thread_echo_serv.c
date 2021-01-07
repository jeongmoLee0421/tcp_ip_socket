#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define BUF_SIZE 100
#define MAX_CLNT 256

void* handle_clnt(void* arg);
void error_handling(char* msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
char buf[BUF_SIZE]; // share buf
pthread_mutex_t mutex;

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    int clnt_addr_size;
    pthread_t t_id;
    
    if (argc != 2){
        printf("Usage %s <port>\n", argv[0]);
        exit(1);
    }
    
    pthread_mutex_init(&mutex, NULL);
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
        
    while(1){
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
            error_handling("accept() error");
            
        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf("Connected client IP: %s\n", inet_ntoa(clnt_addr.sin_addr));
    }
    close(serv_sock);
    pthread_mutex_destroy(&mutex);
    return 0;
}

void* handle_clnt(void* arg){
    int clnt_sock = *((int*)arg);
    int str_len = 0, i;
    
    // lock
    pthread_mutex_lock(&mutex);
    while((str_len = read(clnt_sock, buf, BUF_SIZE)) != 0) // read() blocking
        write(clnt_sock, buf, str_len);
    // unlock
    pthread_mutex_unlock(&mutex);
    
    close(clnt_sock);
    return NULL;
}

void error_handling(char* msg){
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
