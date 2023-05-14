#define _GNU_SOURCE
#include <dirent.h> 
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_FILENAME_SIZE 10
#define MIN_FILENAME_SIZE 5
#define PAGE_SIZE (4 * 1024)
#define TOTAL_FILES 1000
#define FILE_SIZE (PAGE_SIZE)

enum opration_mode {
    OPERATION_MODE_READ,
    OPERATION_MODE_WRITE,
};

enum cache_mode {
    CACHE_MODE_DIRECT, 
    CACHE_MODE_CACHE,
};

int random_number(int min, int max) {
    return (rand() % (max - min)) + min;
}

void random_filename(char *buff) {
    int filename_size = random_number(MIN_FILENAME_SIZE, MAX_FILENAME_SIZE);
    for (int i = 0 ; i < filename_size; i++)
        buff[i] = random_number('a', 'z');
    buff[filename_size] = '\0';
}

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
    int alloc_status = posix_memalign((void**)&buffer, PAGE_SIZE, FILE_SIZE);
    if (alloc_status != 0) {
        perror("cannot allocate buffer");
        exit(1);
    }
    int read_count = read(fd, buffer, FILE_SIZE);
    if (read_count <= 0) {
        printf("error on file %s\n", filename);
        perror("cannot read file");
        exit(1);
    }
    // Close the file
    close(fd);
    free(buffer);
}

void write_file(const char *filename, bool direct) {
    int flags = O_CREAT | O_WRONLY | O_TRUNC;
    if (direct) {
        flags |= O_DIRECT | O_SYNC;
    }
    // Open the file
    int fd = open(filename, flags, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("cannot open file");
        exit(1);
    }
    // Create a buffer
    char *buffer;
    int alloc_status = posix_memalign((void**)&buffer, PAGE_SIZE, FILE_SIZE);
    if (alloc_status != 0) {
        perror("cannot allocate buffer");
        exit(1);
    }
    for (int i = 0; i < FILE_SIZE; i++)
        buffer[i] = rand();
    // Write to file size
    if(write(fd, buffer, FILE_SIZE) < 0) {
        perror("cannot write");
        exit(1);
    }
    // Close the file
    close(fd);
    free(buffer);
}

void read_files(const char *root, bool direct) {
    size_t root_length = strlen(root);
    // Create path buffer and prefill it
    char path_buffer[1024];
    strcpy(path_buffer, root);
    // Open directory
    struct dirent *dir;
    DIR *d = opendir(root);
    if (d == NULL) {
        perror("cannot open folder");
        exit(1);
    }
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') // skip same folder
            continue;
        strcpy(path_buffer + root_length, dir->d_name);
        read_file(path_buffer, direct);
    }
    closedir(d);
}

void write_files(const char *root, bool direct) {
    size_t root_length = strlen(root);
    // Create path buffer and prefill it
    char path_buffer[1024];
    strcpy(path_buffer, root);
    // Create files
    for (int i = 0; i < TOTAL_FILES; i++) {
        random_filename(path_buffer + root_length); // fill the random buffer
        write_file(path_buffer, direct);
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));
    // Parse arguments
    if (argc < 4) {
        printf("Program usage:\n%s <PATH TO FOLDER> [read|write] [direct|cache]\n", argv[0]);
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
    // Do the thing
    if (op_mode == OPERATION_MODE_READ)
        read_files(argv[1], c_mode == CACHE_MODE_DIRECT);
    else
        write_files(argv[1], c_mode == CACHE_MODE_DIRECT);
}