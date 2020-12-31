#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig){
    if (sig == SIGALRM)
        puts("Time out!");
    alarm(2);
}

void keycontrol(int sig){
    if (sig == SIGINT)
        puts("CTRL+C pressed");
}

int main(int argc, char* argv[]){
    // Ask the operating system to call a specific function
    int i;
    signal(SIGALRM, timeout); // Request to call the timeout function after the registered time out through the alarm function call
    signal(SIGINT, keycontrol); // Request to call the keycontrol function after entering CTRL+C
    alarm(2);
    
    // When the signal occurs, the process that was in the blocking state wakes up
    for (i=0; i<3; i++){
        puts("wait...");
        sleep(100);
    }
    return 0;
}
