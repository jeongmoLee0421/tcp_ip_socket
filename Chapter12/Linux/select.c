#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#define BUF_SIZE 30

int main(int argc, char* argv[]){
    fd_set reads, temps;
    int result, str_len;
    char buf[BUF_SIZE];
    struct timeval timeout;
    
    FD_ZERO(&reads); // init all bit 0
    FD_SET(0, &reads); // register fd0 in reads. (fd0 is stdin)
    
    // timeout.tv_sec = 5;
    // timeout.tv_usec = 5000;
    
    while(1){
        temps = reads; // for the maintenance of the original
        
        // if it is not initialized each time,
        // it is changed to the time remaining until the timeout occurs
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        //printf("%ld %ld\n", timeout.tv_sec, timeout.tv_usec);
        
        // int select(int maxfd, fd_set* readset, fd_set* writeset, fd_set* exceptset, const struct timeval* timeout);
        result = select(1, &temps, 0, 0, &timeout);
        
        if (result == -1){
            puts("select() error");
            break;
        }
        else if (result == 0){
            puts("time out");
        }
        else{
            if(FD_ISSET(0, &temps)){
                str_len = read(0, buf, BUF_SIZE);
                buf[str_len] = 0;
                printf("message from console: %s\n", buf);
            }
        }
    }
    
    return 0;
}
