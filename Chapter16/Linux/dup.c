#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    int cfd1, cfd2;
    char str1[] = "Hello\n";
    char str2[] = "friend\n";
    
    // int dup(int fildes);
    // duplicate stdout(1) file descriptor
    cfd1 = dup(1);
    
    // int dup2(int fildes, int fildes2);
    // duplicate cfd1 and set 7
    cfd2 = dup2(cfd1, 7);
    
    printf("fd1 = %d, fd2 = %d\n", cfd1, cfd2);
    write(cfd1, str1, sizeof(str1));
    write(cfd2, str2, sizeof(str2));
    
    close(cfd1);
    close(cfd2);
    
    // output
    write(1, str1, sizeof(str1));
    close(1);
    
    // not output
    // close all fd(cfd1, cfd2, 1(stdout))
    write(1, str2, sizeof(str2));
    return 0;
}
