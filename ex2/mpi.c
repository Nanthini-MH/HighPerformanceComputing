#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>
#include <time.h>

const int MAX_STRING = 100;

/* Palindrome check function */
int isPalindrome(char str[]) {
    int i = 0, j = strlen(str) - 1;
    while (i < j) {
        if (str[i] != str[j])
            return 0;
        i++;
        j--;
    }
    return 1;
}

int main(void) {
    char message[MAX_STRING];
    int comm_sz;          /* Number of processes */
    int my_rank;          /* Process rank */
    MPI_Status status;

    /* List of strings */
    char *string_list[] = {
        "hello",
        "level",
        "world",
        "madam",
        "computer",
        "radar"
    };
    int list_size = 6;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    struct timespec start,end;
    /* Non-master processes */
    if (my_rank != 0) {

        /* Pick string from list */
        strcpy(message, string_list[my_rank % list_size]);

        if (my_rank % 2 == 1) {
            /* Odd rank → Type 1 (UPPERCASE only) */
            MPI_Send(message, strlen(message) + 1,
                     MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        } else {
            /* Even rank → Type 2 (Palindrome only) */
            MPI_Send(message, strlen(message) + 1,
                     MPI_CHAR, 0, 2, MPI_COMM_WORLD);
        }
    }
    /* Master process */
    else {
        char recv_msg[MAX_STRING];

        for (int q = 1; q < comm_sz; q++) {
           clock_gettime(CLOCK_MONOTONIC,&start);
            MPI_Recv(recv_msg, MAX_STRING, MPI_CHAR,
                     MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == 1) {
                /* Convert ONLY Type 1 to uppercase */
                for (int i = 0; recv_msg[i]; i++)
                    recv_msg[i] = toupper(recv_msg[i]);

                printf("From process %d | Type 1 | Uppercase: %s\n",
                       status.MPI_SOURCE, recv_msg);
            }
            else if (status.MPI_TAG == 2) {
                /* No conversion here */
                if (isPalindrome(recv_msg))
                    printf("From process %d | Type 2 | Palindrome: %s\n",
                           status.MPI_SOURCE, recv_msg);
                else
                    printf("From process %d | Type 2 | Not Palindrome: %s\n",
                           status.MPI_SOURCE, recv_msg);
            }
            clock_gettime(CLOCK_MONOTONIC,&end);
            double time = (end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec);
            printf(" Execution time for process %d is: %lf ns \n",q,time);

        }
    }

    MPI_Finalize();
    return 0;
}