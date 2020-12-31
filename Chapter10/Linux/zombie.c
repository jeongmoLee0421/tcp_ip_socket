#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    pid_t pid = fork();
    
    if (pid == 0){
        puts("Hi, I am a child process");
    }
    else{
        printf("Child Process ID: %d\n", pid);
        sleep(30);
    }
    
    if (pid == 0)
        puts("End child process");
    else
        puts("End parent process");
    return 0;
}

/*
When will the zombie process will be destroyed?

the value of the argument of exit function or the return value of the return statement
must be passed to the parent process that created the child process.
*/
