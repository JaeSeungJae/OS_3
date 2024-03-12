#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/resource.h> // Required for setpriority()

#define MAX_PROCESSES 10000

int main() {
    int i;
    pid_t pid;
    int value;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(i = 0; i < MAX_PROCESSES; i++) {
        pid = fork();

        if(pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if(pid == 0) { // child process
            // Step 2: CPU 스케줄링 정책 변경
            struct sched_param param;
            //param.sched_priority = 0; // Set the priority, usually 0 for SCHED_OTHER
            //param.sched_priority = sched_get_priority_max(or min)(SCHED_FIFO or SCHED_RR);
            param.sched_priority = 99;
            if(sched_setscheduler(0, SCHED_RR, &param) == -1) { // set scheduler
                perror("sched_setscheduler failed");
                exit(EXIT_FAILURE);
            }

            
            // if(setpriority(PRIO_PROCESS, getpid(), 19) == -1) { // only use in SCHED_OTHER
            //     perror("setpriority failed"); // set value of nice
            //     exit(EXIT_FAILURE);
            // }

            // Step 3: Read the integer from the corresponding file
            char filename[256];
            snprintf(filename, sizeof(filename), "./temp/%d", i);
            FILE *f_read = fopen(filename, "r");
            if(!f_read) {
                perror("File reading error");
                exit(EXIT_FAILURE);
            }
            fscanf(f_read, "%d", &value);
            fclose(f_read);
            //printf("Process %d read value: %d\n", i, value);

            exit(EXIT_SUCCESS);
        }
    }

    // Parent waits for all children to complete
    for(i = 0; i < MAX_PROCESSES; i++) {
        wait(NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("%f\n", elapsed_time);

    return 0;
}
