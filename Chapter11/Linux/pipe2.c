#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char* argv[]){
    int fds[2];
    char str1[] = "Who are you?";
    char str2[] = "jeongmolee";
    char buf[BUF_SIZE];
    pid_t pid;
    
    pipe(fds);
    pid = fork();
    
    // if 'sleep(2)' code does not run
    // child process write and receive own data
    // then parent process blocking state in 'read' code
    
    // when data is passed to the pipe,
    // it is passed to the process that takes it first
    // even if it is a provider process
    
    if (pid == 0){
        write(fds[1], str1, sizeof(str1));
        sleep(2); // if does not run
        read(fds[0], buf, BUF_SIZE);
        printf("Child proc output: %s\n", buf);
    }
    else{
        read(fds[0], buf, BUF_SIZE); // blocking state
        printf("Parent proc output: %s\n", buf);
        write(fds[1], str2, sizeof(str2));
        sleep(3);
    }
    return 0;
}
