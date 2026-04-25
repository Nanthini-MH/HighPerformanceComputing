#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>

#define N 100  // maximum matrix size
int i,j,k;
// Function to generate random matrix
void generateMatrix(int n, int **A) {
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            A[i][j] = rand() % 10;
}

// Function to print matrix
void printMatrix(int n, int **A) {
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            printf("%d ", A[i][j]);
        printf("\n");
    }
}
void printSharedMatrix(int n, int (*A)[n]) {
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            printf("%d ", A[i][j]);
        printf("\n");
    }
}

int main() {
    srand(time(NULL));
    int i,j,k;
    int n = 50;
    int **A = malloc(n * sizeof(int *));
    int **B = malloc(n * sizeof(int *));
    int **C = malloc(n * sizeof(int *));
    int **C_serial=malloc(n*sizeof(int *));
    for(i=0;i<n;i++)
        C_serial[i]=malloc(n*sizeof(int));

    for (i = 0; i < n; i++) {
        A[i] = malloc(n * sizeof(int));
        B[i] = malloc(n * sizeof(int));
        C[i] = malloc(n * sizeof(int));
     }

    printf("Matrix size: %dx%d\n\n", n, n);

    generateMatrix(n, A);
    generateMatrix(n, B);

    struct timespec start, end;

    // ================= SERIAL EXECUTION =================
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            C_serial[i][j] = 0;
            for (k = 0; k < n; k++) {
                if(n<=3){
               // printf("Serial: C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                       // i, j, i, k, k, j);
                }
                C_serial[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double serial_time =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

   // printf("\nSerial Result Matrix:\n");
   // printMatrix(n, C_serial);

    // ================= PARALLEL EXECUTION =================
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * n * n, IPC_CREAT | 0666);
    int (*C_parallel)[n] = shmat(shmid, NULL, 0);

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            for (j = 0; j < n; j++) {
                C_parallel[i][j] = 0;
                for (k = 0; k < n; k++) {
                    if(n<=3){
                        //printf("Child %d: C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                          //  getpid(), i, j, i, k, k, j);
                       }
                        C_parallel[i][j] += A[i][k] * B[k][j];
                }
            }
            exit(0);
        }
    }

    for (i = 0; i < n; i++)
        wait(NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double parallel_time =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("\nExecution Time:\n");
    printf("Serial Time   : %lf milli seconds\n", serial_time*1000);
    printf("Parallel Time : %lf milli seconds\n", parallel_time*1000);

    shmdt(C_parallel);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}