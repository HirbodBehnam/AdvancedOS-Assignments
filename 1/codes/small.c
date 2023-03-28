#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define SIZE 1024

int main() {
    long pagesize = sysconf(_SC_PAGE_SIZE);
    if (pagesize == -1) {
        perror("_SC_PAGE_SIZE");
        exit(1);
    }
    printf("got page size as: %ld\n", pagesize);
    puts("allocating");
    void *mem;
    int result = posix_memalign(&mem, pagesize, SIZE);
    if (result < 0) {
        perror("memalign");
        exit(1);
    }
    puts("protecting");
    result = mprotect(mem, SIZE, PROT_READ);
    if (result < 0) {
        perror("mprotect");
        exit(1);
    }
    char *mem2 = (char *)mem;
    puts("writing");
    fflush(stdout);
    for (int i = 0; i < SIZE; i++)
        mem2[i] = 'f';
    puts("reading");
    fflush(stdout);
    for (int i = 0; i < SIZE; i++)
        putchar(mem2[i]);
}