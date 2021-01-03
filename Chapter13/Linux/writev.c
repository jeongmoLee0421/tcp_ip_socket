#include <stdio.h>
#include <sys/uio.h>

int main(int argc, char* argv[]){
    /*
    struct iovec{
        void* iov_base; // buf address
        size_t iov_len // buf size
    }
    */
    struct iovec vec[2];
    char buf1[] = "ABCDEFG";
    char buf2[] = "1234567";
    int str_len;
    
    // point to buf1 and read 3 characters
    vec[0].iov_base = buf1;
    vec[0].iov_len = 3;
    vec[1].iov_base = buf2;
    vec[1].iov_len = 4;
    
    // ssize_t writev(int filedes, const struct iovec* iov, int iovcnt);
    // collect and transmit data(improve efficiency)
    str_len = writev(1, vec, 2);
    puts("");
    printf("Write bytes: %d\n", str_len);
    return 0;
}

// readv & writev
// it is important to be able to reduce the amount of transmitted packets
// because they collect and send, collect and receive
