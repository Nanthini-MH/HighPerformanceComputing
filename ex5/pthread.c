#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX 10 // Maximum size of the matrices

int a[MAX][MAX], b[MAX][MAX];
int add_result[MAX][MAX], mul_result[MAX][MAX];
int size;

// Function to perform matrix addition
void* add_matrices(void* arg) {
   int i, j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            add_result[i][j] = a[i][j] + b[i][j];
        }
    }
    pthread_exit(NULL);
}

// Function to perform matrix multiplication
void* multiply_matrices(void* arg) {
   int i,j,k;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            mul_result[i][j] = 0; // Initialize result for multiplication
            for (k = 0; k < size; k++) {
                mul_result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t add_thread, mul_thread;
    int i,j;

    printf("Enter the size of the matrices (max %d): ", MAX);
    scanf("%d", &size);

    printf("Enter matrix A:\n");
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            scanf("%d", &a[i][j]);
        }
    }

    printf("Enter matrix B:\n");
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            scanf("%d", &b[i][j]);
        }
    }

    // Create thread for matrix addition
    pthread_create(&add_thread, NULL, add_matrices, NULL);

    // Create thread for matrix multiplication
    pthread_create(&mul_thread, NULL, multiply_matrices, NULL);

    // Wait for both threads to finish
    pthread_join(add_thread, NULL);
    pthread_join(mul_thread, NULL);

    // Display results
    printf("Matrix Addition Result:\n");
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            printf("%d  ", add_result[i][j]);
        }
        printf("\n");
    }

    printf("Matrix Multiplication Result:\n");
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            printf("%d  ", mul_result[i][j]);
        }
        printf("\n");
    }

    return 0;
}