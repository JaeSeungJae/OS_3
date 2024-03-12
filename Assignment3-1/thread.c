#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 8

typedef struct {
    int a, b;
    int result;
} Args;

void *add_numbers(void *arguments) {
    Args *args = (Args *)arguments;
    args->result = args->a + args->b;
    return NULL;
}

int main() {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    FILE *file = fopen("temp.txt", "r");
    if (!file) {
        perror("Failed to open temp.txt");
        return 1;
    }

    pthread_t threads[MAX_THREADS];
    Args thread_args[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        if (fscanf(file, "%d", &thread_args[i].a) == EOF || fscanf(file, "%d", &thread_args[i].b) == EOF) {
            fprintf(stderr, "Not enough numbers in temp.txt\n");
            fclose(file);
            return 1;
        }
        pthread_create(&threads[i], NULL, add_numbers, &thread_args[i]);
    }
    fclose(file);

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    int thread_count = MAX_THREADS;
    while (thread_count > 1) {
        for (int i = 0; i < thread_count / 2; i++) {
            thread_args[i].a = thread_args[2*i].result;
            thread_args[i].b = thread_args[2*i + 1].result;
            pthread_create(&threads[i], NULL, add_numbers, &thread_args[i]);
        }

        for (int i = 0; i < thread_count / 2; i++) {
            pthread_join(threads[i], NULL);
        }

        thread_count /= 2;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Value of thread : %d\n", thread_args[0].result);
    printf("%f\n", elapsed_time);

    return 0;
}
