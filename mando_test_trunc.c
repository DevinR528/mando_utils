#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define FACTOR 4
#define NUM_THREADS 32

const char* TO_WRITE[NUM_THREADS] = {
    "yo 01\n", "yo 02\n", "yo 03\n", "yo 04\n",
    "yo 05\n", "yo 06\n", "yo 07\n", "yo 08\n",
    "yo 09\n", "yo 10\n", "yo 11\n", "yo 12\n",
    "yo 13\n", "yo 14\n", "yo 15\n", "yo 16\n",
    "yo 17\n", "yo 18\n", "yo 19\n", "yo 20\n",
    "yo 21\n", "yo 22\n", "yo 23\n", "yo 24\n",
    "yo 25\n", "yo 26\n", "yo 27\n", "yo 28\n",
    "yo 29\n", "yo 30\n", "yo 31\n", "yo 32\n"
};

struct tuple {
    char* path;
    int chunks;
    int start;
};

int EXIT = -1;

void* thread_function(void* arg) {
    struct tuple* pair = (struct tuple*)arg;
    char* path = pair->path;
    int cnt = pair->chunks;

    int start = pair->start * cnt;
    int end = start + cnt;

    // This tests open/write with truncation for every write to the file
    for (int i = start; i < end; ++i) {
        int fd = open(path, O_WRONLY | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open");
            pthread_exit((void*)&EXIT);
        }

        if (write(fd, TO_WRITE[i], strlen(TO_WRITE[i])) == -1) {
            perror("write");
            close(fd);
            pthread_exit((void*)&EXIT);
        }

        close(fd);
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Must pass valid file name to run got `%s`\n", argv[0]);
        return -1;
    }

    char* path = argv[1];
    pthread_t threads[NUM_THREADS];

    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    int div = (num_cpus <= FACTOR) ? 1 : FACTOR;
    int threads_to_spawn = num_cpus / div;

    int num_chunks = NUM_THREADS / threads_to_spawn;
    printf("chunks = %d threads = %d\n", num_chunks, threads_to_spawn);

    for (int i = 0; i < threads_to_spawn; ++i) {
        struct tuple* foo = malloc(sizeof(struct tuple));
        foo->path = path;
        foo->chunks = num_chunks;
        foo->start = i;
        int result = pthread_create(&threads[i], NULL, thread_function, (void*)foo);
        if (result != 0) {
            fprintf(stderr, "Error creating thread: %d\n", result);
            return -1;
        }
    }

    for (int i = 0; i < threads_to_spawn; ++i) {
        int res = 0;
        int result = pthread_join(threads[i], (void*)&res);
        if (result != 0 || res != 0) {
            fprintf(stderr, "Error joining thread: %d\n", result);
            return -1;
        }
    }

    return 0;
}
