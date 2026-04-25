#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_NAME_LENGTH 30
#define TOTAL_STUDENTS 10

typedef struct {
    char name[MAX_NAME_LENGTH];
    int rollno;
    float total_marks;
    char grade;
} Student;

char calculate_grade(float total_marks) {
    if (total_marks >= 90) return 'A';
    else if (total_marks >= 75) return 'B';
    else if (total_marks >= 60) return 'C';
    else if (total_marks >= 50) return 'D';
    else return 'F';
}

void read_input_file(Student *students, int count) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        perror("Error opening input.txt");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    char header[256];
    fgets(header, sizeof(header), file);
    for (int i = 0; i < count; i++) {
        fscanf(file, "%s %d %f", students[i].name, &students[i].rollno, &students[i].total_marks);
    }
    fclose(file);
}

void write_results_to_file(Student *students, int count) {
    FILE *file = fopen("grades.txt", "w");
    if (!file) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%-15s %-10s %-10s %-5s\n", "Name", "RollNo", "Marks", "Grade");
    fprintf(file, "--------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        fprintf(file, "%-15s %-10d %-10.2f %-5c\n",
                students[i].name, students[i].rollno, students[i].total_marks, students[i].grade);
    }
    fclose(file);
}

int main(int argc, char** argv) {
    int rank, size;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (TOTAL_STUDENTS % size != 0) {
        if (rank == 0) printf("Error: TOTAL_STUDENTS (%d) must be divisible by process count (%d)\n", TOTAL_STUDENTS, size);
        MPI_Finalize();
        return 0;
    }

    int chunk_size = TOTAL_STUDENTS / size;
    Student students[TOTAL_STUDENTS];
    Student local_students[chunk_size];

    if (rank == 0) {
        read_input_file(students, TOTAL_STUDENTS);
        start_time = MPI_Wtime();
    }

    // Distribute the data
    MPI_Scatter(students, sizeof(Student) * chunk_size, MPI_BYTE,
                local_students, sizeof(Student) * chunk_size, MPI_BYTE,
                0, MPI_COMM_WORLD);

    // Parallel Processing with Process Tracking
    for (int i = 0; i < chunk_size; i++) {
        local_students[i].grade = calculate_grade(local_students[i].total_marks);

        // Print which process is handling which student
        printf("[Process %d] is calculating grade for: %-10s (Roll: %d)\n",
               rank, local_students[i].name, local_students[i].rollno);
    }

    // Barrier to ensure all prints finish before Rank 0 concludes
    MPI_Barrier(MPI_COMM_WORLD);

    // Collect results
    MPI_Gather(local_students, sizeof(Student) * chunk_size, MPI_BYTE,
               students, sizeof(Student) * chunk_size, MPI_BYTE,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        write_results_to_file(students, TOTAL_STUDENTS);
        end_time = MPI_Wtime();

        // printf("\n==========================================\n");
        // printf(" Execution Summary\n");
        // printf(" Total Students: %d\n", TOTAL_STUDENTS);
        // printf(" Processes Used: %d\n", size);
        printf(" Execution Time: %f seconds\n", end_time - start_time);
        // printf("==========================================\n");
    }

    MPI_Finalize();
    return 0;
}