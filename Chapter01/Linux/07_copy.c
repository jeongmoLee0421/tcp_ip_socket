#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 100
void error_handling(char* message);

int main(void){
    int read_fd, write_fd;
    char buf[BUF_SIZE];
    
    read_fd = open("data.txt", O_RDONLY);
    write_fd = open("data_copy.txt", O_CREAT|O_TRUNC|O_WRONLY);
    
    if (read_fd == -1 || write_fd == -1)
        error_handling("open() error!");
        
    if (read(read_fd, buf, sizeof(buf)) == -1)
        error_handling("read() error!");
        
    if (write(write_fd, buf, sizeof(buf)) == -1)
        error_handling("write() error!");
        
    close(read_fd);
    close(write_fd);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
