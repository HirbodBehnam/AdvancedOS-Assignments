#define _GNU_SOURCE // https://stackoverflow.com/a/7269859/4213397
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

pthread_mutex_t allocated_memory_cond_var_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t allocated_memory_cond_var = PTHREAD_COND_INITIALIZER;
char *allocated_memory = NULL;
#define ALLOCATED_MEMORY_SIZE 4096

void *allocator_thread(void *args) {
    // Allocate memory and signal threads that we have allocated it
    pthread_mutex_lock(&allocated_memory_cond_var_lock);
    // memory must be on page boundary I guess
    allocated_memory = pvalloc(ALLOCATED_MEMORY_SIZE);
    if (allocated_memory == NULL) {
        perror("memalign");
        exit(1);
    }
    pthread_cond_broadcast(&allocated_memory_cond_var);
    pthread_mutex_unlock(&allocated_memory_cond_var_lock);
    return NULL;
}

void *protector_thread(void *args) {
    // Wait for memory to be allocated
    pthread_mutex_lock(&allocated_memory_cond_var_lock);
    while (allocated_memory == NULL)
        pthread_cond_wait(&allocated_memory_cond_var, &allocated_memory_cond_var_lock);
    pthread_mutex_unlock(&allocated_memory_cond_var_lock);
    // Lockdown the read access
    int result = mprotect(allocated_memory, ALLOCATED_MEMORY_SIZE, PROT_READ);
    if (result < 0) {
        perror("cannot protect memory");
        exit(1);
    }
    return NULL;
}

void *reader_thread(void *args) {
    // Wait for memory to be allocated
    pthread_mutex_lock(&allocated_memory_cond_var_lock);
    while (allocated_memory == NULL)
        pthread_cond_wait(&allocated_memory_cond_var, &allocated_memory_cond_var_lock);
    pthread_mutex_unlock(&allocated_memory_cond_var_lock);
    // Read data from memory and discard it
    int _temp = 0;
    for (int i = 0; i < ALLOCATED_MEMORY_SIZE; i++)
        _temp += allocated_memory[i];
    return (void *)_temp;
}

void *write_thread(void *args) {
    // Wait for memory to be allocated
    pthread_mutex_lock(&allocated_memory_cond_var_lock);
    while (allocated_memory == NULL)
        pthread_cond_wait(&allocated_memory_cond_var, &allocated_memory_cond_var_lock);
    pthread_mutex_unlock(&allocated_memory_cond_var_lock);
    // Write random data in memory
    for (int i = 0; i < ALLOCATED_MEMORY_SIZE; i++)
        allocated_memory[i] = rand();
    return NULL;
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
    // Get iteration count
    if (argc < 2) {
        puts("Please pass the number of iterations as the first argument");
        exit(1);
    }
    int iterations = 0;
    sscanf(argv[1], "%d", &iterations);
    // Do the thing
    while (iterations--) {
        pthread_t threads[4];
        pthread_attr_t attr;
        pthread_attr_init(&attr); // apparently, we can do this once
        // First thread shall allocate the memory
        assign_exclusive_cpu_core(&attr);
        pthread_create(&threads[0], &attr, allocator_thread, NULL);
        // Second thread shall protect the memory
        assign_exclusive_cpu_core(&attr);
        pthread_create(&threads[1], &attr, protector_thread, NULL);
        // Here we act as the part of question asks us
        assign_exclusive_cpu_core(&attr);
        pthread_create(&threads[2], &attr, write_thread, NULL);
        assign_exclusive_cpu_core(&attr);
        pthread_create(&threads[3], &attr, write_thread, NULL);
        // Join the threads
        for (int i = 0; i < 4; i++)
            pthread_join(threads[i], NULL);
        // Clean up the memory
        free(allocated_memory);
        allocated_memory = NULL;
    }
    return 0;
}
