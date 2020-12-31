#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    int status;
    pid_t pid = fork();
    
    if (pid == 0)
        return 3;
    else{
        printf("Child PID: %d\n", pid);
        pid = fork();
        
        if (pid == 0)
            exit(7);
        else{
            printf("Child PID: %d\n", pid);
            wait(&status);
            if (WIFEXITED(status)) // if the child process terminated normally, return true
                printf("Child send one: %d\n", WEXITSTATUS(status)); // return the return value of the child process
                
            wait(&status);
            if (WIFEXITED(status))
                printf("Child send two: %d\n", WEXITSTATUS(status));
            sleep(30);
        }
    }
    return 0;
}

// if there are no child process that have been killed,
// it will be in a blocking state.
