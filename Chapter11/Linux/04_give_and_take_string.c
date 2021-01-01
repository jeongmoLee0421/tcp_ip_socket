#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[]){
    // if i only have one pipe, i need to control the flow of execution with sleep(),
    // buf if i use multiple piples i don't have to think.
    // one only needs to transmit and the other only needs to receive.
    int fds1[2], fds2[2];
    char str1[] = "what's your name?";
    char str2[] = "jeongmolee";
    char str3[] = "thank you!";
    int state;
    pid_t pid;
    char buf[BUF_SIZE];
    
    state = pipe(fds1);
    if (state == -1)
        error_handling("pipe() error");
        
    state = pipe(fds2);
    if (state == -1)
        error_handling("pipe() error");
    
    // fds1 = child -> parent
    // fds2 = parent -> child
    pid = fork();
    if (pid == 0){
        write(fds1[1], str1, sizeof(str1));
        //sleep(2);
        read(fds2[0], buf, BUF_SIZE);
        printf("parent proc: %s\n", buf);
        write(fds1[1], str3, sizeof(str3));
    }
    else{
        read(fds1[0], buf, BUF_SIZE);
        printf("child proc: %s\n", buf);
        write(fds2[1], str2, sizeof(str2));
        //sleep(2);
        read(fds1[0], buf, BUF_SIZE);
        printf("child proc: %s\n", buf);
    }
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
