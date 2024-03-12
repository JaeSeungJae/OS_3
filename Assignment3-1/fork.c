#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_PROCESSES 8

int add_numbers(int a, int b) {
    pid_t pid = fork();
    if (pid == 0) {  // Child process
        exit(a + b);
    } 
    else {        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int main() {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    FILE *file = fopen("temp.txt", "r");
    if (!file) {
        perror("Failed to open temp.txt");
        return 1;
    }

    int results[MAX_PROCESSES];
    for (int i = 0; i < MAX_PROCESSES; i++) {
        int a, b;
        if (fscanf(file, "%d", &a) == EOF || fscanf(file, "%d", &b) == EOF) {
            fprintf(stderr, "Not enough numbers in temp.txt\n");
            fclose(file);
            return 1;
        }
        results[i] = add_numbers(a, b);
    }
    fclose(file);
    int max = MAX_PROCESSES;
    while (max > 1) {
        for (int i = 0; i < max / 2; i++) {
            results[i] = add_numbers(results[2*i], results[2*i + 1]);
        }
        max /= 2;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Value of fork: %d\n", results[0]);
    printf("%f\n", elapsed_time);

    return 0;
}
