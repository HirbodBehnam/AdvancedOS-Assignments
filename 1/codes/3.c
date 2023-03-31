// Most of this code is from https://www.jabperf.com/how-to-deter-or-disarm-tlb-shootdowns/

#define _GNU_SOURCE // https://stackoverflow.com/a/7269859/4213397
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>

pthread_barrier_t barrier;
void *allocated_memory;
#define ALLOCATED_MEMORY_SIZE (1024 * 32)

void *runner(void *arg) {
    // Wait for all threads
    pthread_barrier_wait(&barrier);
    // Loop
    for (int i = 0; i < 100000; i++)
        madvise(allocated_memory, ALLOCATED_MEMORY_SIZE, MADV_DONTNEED);
    return NULL;
}

int main(int argc, char **argv) {
    // Parse arguments
    if (argc < 2) {
        puts("Pass the number of threads as the second argument");
        exit(1);
    }
    int thread_count = 0;
    sscanf(argv[1], "%d", &thread_count);
    // Allocate memory
    allocated_memory = pvalloc(ALLOCATED_MEMORY_SIZE);
    if (allocated_memory == NULL) {
        perror("cannot allocate memory");
        exit(1);
    }
    // Create barrier to sync threads
    pthread_barrier_init(&barrier, NULL, thread_count);
    // Create threads
    pthread_t threads[thread_count];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    cpu_set_t cpus;
    for (int i = 1; i <= thread_count; i++) {    
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus); // This will assign CPU cores except zero'th core
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        pthread_create(&threads[i - 1], &attr, runner, NULL);
    }
    // Wait for threads
    for (int i = 0; i < thread_count; i++)
        pthread_join(threads[i], NULL);
    return 0;
}