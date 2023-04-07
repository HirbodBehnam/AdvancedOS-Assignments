#define _GNU_SOURCE
#include <malloc.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

pthread_mutex_t exchanged_cond_var_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t exchanged_cond_var = PTHREAD_COND_INITIALIZER;
char *exchanged = NULL;
int exchanged_fd = -1;
#define EXCHANGED_SIZE 4096
#define EXCHANGED_FILENAME1 "dummy1.dat"

void *mapper_thread(void *args) {
    // Allocate memory and signal threads that we have allocated it
    pthread_mutex_lock(&exchanged_cond_var_lock);
    // Create the exchanged part
    exchanged = pvalloc(EXCHANGED_SIZE);
    if (exchanged == NULL) {
        perror("cannot allocate memory");
        exit(1);
    }
    exchanged_fd = open(EXCHANGED_FILENAME1, O_RDWR);
    if (exchanged_fd == -1) {
        perror("cannot open exchanged 1 file");
        exit(1);
    }
    exchanged = mmap(exchanged, EXCHANGED_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, exchanged_fd, 0);
    if (exchanged == MAP_FAILED) {
        perror("cannot map exchanged 1 file");
        exit(1);
    }
    pthread_cond_broadcast(&exchanged_cond_var);
    pthread_mutex_unlock(&exchanged_cond_var_lock);
    return NULL;
}

void *unmapper_thread(void *args) {
    // Wait for memory to be allocated
    pthread_mutex_lock(&exchanged_cond_var_lock);
    while (exchanged == NULL)
        pthread_cond_wait(&exchanged_cond_var, &exchanged_cond_var_lock);
    pthread_mutex_unlock(&exchanged_cond_var_lock);
    // Unmap the file and close the file descriptor
    if (munmap(exchanged, EXCHANGED_SIZE) < 0) {
        perror("cannot unmap the file");
        exit(1);
    }
    close(exchanged_fd);
    return NULL;
}

void *reader_thread(void *args) {
    // Wait for memory to be allocated
    pthread_mutex_lock(&exchanged_cond_var_lock);
    while (exchanged == NULL)
        pthread_cond_wait(&exchanged_cond_var, &exchanged_cond_var_lock);
    pthread_mutex_unlock(&exchanged_cond_var_lock);
    // Read data from memory and discard it
    int _temp = 0;
    for (int i = 0; i < EXCHANGED_SIZE; i++)
        _temp += exchanged[i];
    return (void *)_temp;
}

void *write_thread(void *args) {
    // Wait for memory to be allocated
    pthread_mutex_lock(&exchanged_cond_var_lock);
    while (exchanged == NULL)
        pthread_cond_wait(&exchanged_cond_var, &exchanged_cond_var_lock);
    pthread_mutex_unlock(&exchanged_cond_var_lock);
    // Write random data in memory
    for (int i = 0; i < EXCHANGED_SIZE; i++)
        exchanged[i] = rand();
    return NULL;
}

void create_dummy_file(const char *filename) {
    // Delete the old files
    remove(filename);
    // Create new file
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("cannot open dummy file");
        exit(1);
    }
    // Write random stuff in it
    for (int i = 0; i < EXCHANGED_SIZE; i++)
        fputc(rand() % 256, file);
    // Done
    fclose(file);
}

void assign_exclusive_cpu_core(pthread_attr_t *attr) {
    static int cpu_core_counter = 0;
    int current_cpu_core = ++cpu_core_counter; // this makes the cores start from 1
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(current_cpu_core, &cpus);
    pthread_attr_setaffinity_np(attr, sizeof(cpu_set_t), &cpus);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    // Clean up the old files and create new files
    create_dummy_file(EXCHANGED_FILENAME1);
    // Do the thing
    pthread_t threads[4];
    pthread_attr_t attr;
    pthread_attr_init(&attr); // apparently, we can do this once
    // First thread shall allocate the memory
    assign_exclusive_cpu_core(&attr);
    pthread_create(&threads[0], &attr, mapper_thread, NULL);
    // Second thread shall unmap
    assign_exclusive_cpu_core(&attr);
    pthread_create(&threads[1], &attr, unmapper_thread, NULL);
    // Here we act as the part of question asks us
    assign_exclusive_cpu_core(&attr);
    pthread_create(&threads[2], &attr, reader_thread, NULL);
    assign_exclusive_cpu_core(&attr);
    pthread_create(&threads[3], &attr, reader_thread, NULL);
    // Join the threads
    for (int i = 0; i < 4; i++)
        pthread_join(threads[i], NULL);
    return 0;
}