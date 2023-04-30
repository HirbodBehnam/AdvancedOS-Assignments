#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

#define FILEPATH "dummy.dat"
#define FILE_SIZE (1024 * 1024)

// The file to memory map it
char *memory_mapped_file = NULL;

void *writer_thread(void *args) {
    for (int i = 0; i < FILE_SIZE; i++) {
        memory_mapped_file[i] = rand();
        msync(memory_mapped_file, FILE_SIZE, MS_SYNC);
    }
    return NULL;
}

void create_disk_file() {
    srand(time(NULL));
    // Remove the last dummy file
    remove(FILEPATH);
    // Create a dummy file
    FILE* file = fopen(FILEPATH, "wb");
    for (int i = 0; i < FILE_SIZE; i++)
        fputc('\0', file);
    fclose(file);
}

int main(int argc, char** argv) {
    // Parse number of arguments
    int thread_count = 1;
    if (argc > 1)
        thread_count = atoi(argv[1]);
    printf("Running with %d threads\n", thread_count);
    // Create the dummy file
    srand(time(NULL));
    create_disk_file();
    // Memory map the file
    int file_map_fd = open(FILEPATH, O_RDONLY);
    if (file_map_fd <= 0) {
        perror("cannot open file");
        exit(1);
    }
    memory_mapped_file = mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_PRIVATE, file_map_fd, 0);
    if (memory_mapped_file == (void *) -1) {
        perror("cannot memory map the file");
        exit(1);
    }
    // Create threads
    pthread_t threads[thread_count];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    cpu_set_t cpus;
    for (int i = 1; i <= thread_count; i++) {    
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus); // This will assign CPU cores except zero'th core
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        pthread_create(&threads[i - 1], &attr, writer_thread, NULL);
    }
    // Wait for threads
    for (int i = 0; i < thread_count; i++)
        pthread_join(threads[i], NULL);
    // Unmap the file
    if (munmap(memory_mapped_file, FILE_SIZE) < 0) {
        perror("cannot unmap the file");
        exit(1);
    }
    // Close the file
    close(file_map_fd);
    return 0;
}