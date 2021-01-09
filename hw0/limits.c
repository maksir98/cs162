#include <stdio.h>
#include <sys/resource.h>
#include <stdlib.h>


int main() {
    struct rlimit* lim_stack = (struct rlimit*) malloc(sizeof(struct rlimit));
    struct rlimit* lim_process_limit = (struct rlimit*) malloc(sizeof(struct rlimit));
    struct rlimit* lim_descriptors = (struct rlimit*) malloc(sizeof(struct rlimit));

    if (getrlimit(RLIMIT_STACK, lim_stack) < 0)
    {
        printf("fail\n");
        exit(1);
    }
    if (getrlimit(RLIMIT_NPROC, lim_process_limit) < 0)
    {
        printf("fail\n");
        exit(1);
    }
    if (getrlimit(RLIMIT_NOFILE, lim_descriptors) < 0)
    {
        printf("fail\n");
        exit(1);
    }
    
    printf("stack size: %d\n", (int) lim_stack->rlim_cur);
    printf("process limit: %d\n", (int) lim_process_limit->rlim_cur);
    printf("max file descriptors: %d\n", (int) lim_descriptors->rlim_cur);
    return 0;
}
