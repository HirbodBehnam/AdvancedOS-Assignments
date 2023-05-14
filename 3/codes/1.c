#define _GNU_SOURCE
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TESTS 10
#define BUFFER_SIZE (32 * 1024)
#define FILE_SIZE (1024ll * 1024ll * 1024ll * 10ll)
#define PAGE_SIZE (4 * 1024)

enum opration_mode {
    OPERATION_MODE_READ,
    OPERATION_MODE_WRITE,
};

enum cache_mode {
    CACHE_MODE_DIRECT, 
    CACHE_MODE_CACHE,
};

void read_file(const char *filename, bool direct) {
    int flags = O_RDONLY;
    if (direct) {
        flags |= O_DIRECT | O_SYNC;
    }
    // Open the file
    int fd = open(filename, flags);
    if (fd < 0) {
        perror("cannot open file");
        exit(1);
    }
    // Read the file until the end
    char *buffer;
    int alloc_status = posix_memalign((void**)&buffer, PAGE_SIZE, BUFFER_SIZE);
    if (alloc_status != 0) {
        perror("cannot allocate buffer");
        exit(1);
    }
    while (1) {
        int read_count = read(fd, buffer, BUFFER_SIZE);
        if (read_count <= 0)
            break;
    }
    // Close the file
    close(fd);
    free(buffer);
}

void write_file(const char *filename, bool direct) {
    int flags = O_WRONLY | O_TRUNC;
    if (direct) {
        flags |= O_DIRECT | O_SYNC;
    }
    // Open the file
    int fd = open(filename, flags);
    if (fd < 0) {
        perror("cannot open file");
        exit(1);
    }
    // Create a buffer
    char *buffer;
    int alloc_status = posix_memalign((void**)&buffer, PAGE_SIZE, BUFFER_SIZE);
    if (alloc_status != 0) {
        perror("cannot allocate buffer");
        exit(1);
    }
    for (int i = 0; i < BUFFER_SIZE; i++)
        buffer[i] = rand();
    // Write to file size
    for (long long i = 0; i < FILE_SIZE / BUFFER_SIZE; i++)
        if(write(fd, buffer, BUFFER_SIZE) < 0) {
            perror("cannot write");
            exit(1);
        }
    // Close the file
    close(fd);
    free(buffer);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    // Parse arguments
    if (argc < 4) {
        printf("Program usage:\n%s <PATH TO FILE> [read|write] [direct|cache]\n", argv[0]);
        exit(1);
    }
    enum opration_mode op_mode;
    if (strcmp(argv[2], "write") == 0) {
        op_mode = OPERATION_MODE_WRITE;
        puts("Using write mode");
    } else if (strcmp(argv[2], "read") == 0) {
        op_mode = OPERATION_MODE_READ;
        puts("Using read mode");
    } else {
        puts("Invalid operation mode");
        exit(1);
    }
    enum cache_mode c_mode;
    if (strcmp(argv[3], "direct") == 0) {
        c_mode = CACHE_MODE_DIRECT;
        puts("Using direct mode");
    } else if (strcmp(argv[3], "cache") == 0) {
        c_mode = CACHE_MODE_CACHE;
        puts("Using cache mode");
    } else {
        puts("Invalid cache mode");
        exit(1);
    }
    // Do the tests
    for (int i = 0; i < TESTS; i++) {
        if (op_mode == OPERATION_MODE_READ)
            read_file(argv[1], c_mode == CACHE_MODE_DIRECT);
        else
            write_file(argv[1], c_mode == CACHE_MODE_DIRECT);
    }
}