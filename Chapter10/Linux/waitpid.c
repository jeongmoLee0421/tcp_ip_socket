#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    int status;
    pid_t pid = fork();
    
    if (pid == 0){
        sleep(15);
        return 24;
    }
    else{
        while(!waitpid(-1, &status, WNOHANG)){
        // WNOHANG
        // Even if ther are no child process that have been killed,
        // it is not in blocking, it return 0 and terminates the function.
            sleep(3);
            puts("sleep 3sec.");
        }
        
        if (WIFEXITED(status))
            printf("Child send %d\n", WEXITSTATUS(status));
    }
    return 0;
}
