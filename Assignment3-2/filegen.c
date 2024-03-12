#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_PROCESSES 10000

int main() {
    int i;
    char filename[256];

    // Step 1: 디렉토리 생성
    mkdir("temp", 0755);

    // Step 2: MAX_PROCESSES 만큼 파일 생성
    for(i = 0; i < MAX_PROCESSES; i++) {
        snprintf(filename, sizeof(filename), "./temp/%d", i);
        FILE *f_write = fopen(filename, "w");
        if(f_write == NULL) {
            perror("File opening error");
            exit(EXIT_FAILURE);
        }
        fprintf(f_write, "%d", 1 + rand() % 9);
        fclose(f_write);
    }

    return 0;
}
