#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

void error_handling(char* message);
void end_program(int sig);

int main(int argc, char* argv[]){
    struct sigaction act;
    act.sa_handler = end_program;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    
    sigaction(SIGINT, &act, 0); // register
    
    while(1){
        printf("hello friend\n");
        sleep(1);
    }
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void end_program(int sig){
    char c;
    
    if (sig == SIGINT){
        printf("really end? ");
        scanf("%c", &c);
        
        if ((c == 'Y') || (c == 'y'))
            exit(0);
        else return;
    }
}
