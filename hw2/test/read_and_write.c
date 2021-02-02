#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


const char* file_path_name = "./test/result.txt";

int main(int argc, char const *argv[])
{
    int pipefd[2];
    char *read_buffer = (char *)malloc(1000);
    int file_descritor = open(file_path_name, O_WRONLY);

    ssize_t size_read = read(STDIN_FILENO, (void*)read_buffer, 1000);
    printf("read size: %ld\n", size_read);
    // perror("read failed");

    if (file_descritor == -1)
    {
        perror("open failed(read_and_write)");
    }
    
    if (write(file_descritor, (void*)read_buffer, size_read) == -1)
    {
        perror("write failed(read_and_write)");
    }
    if (fsync(file_descritor) == -1)
    {
        perror("fsync failed(read_and_write)");
    }
    close(file_descritor);

    return 0;
}
