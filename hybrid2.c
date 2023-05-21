#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int rank, num_procs, num_threads;
    char *filename = "Ryz.txt";
    char *target_phrase = "on";
    char buffer[BUFFER_SIZE];
    int phrase_count = 0, local_phrase_count = 0;
    double start_time, end_time;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Open file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        MPI_Finalize();
        return 1;
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    long int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Calculate chunk size for each process
    long int chunk_size = file_size / num_procs;
    if (rank == num_procs - 1) {
        // Last process gets any remaining bytes
        chunk_size += file_size % num_procs;
    }

    // Allocate buffer for each process
    char *chunk_buffer = (char*) malloc(chunk_size + strlen(target_phrase) + 1);

    // Loop through thread counts from 1 to 32
    for (int num_threads = 0; num_threads <= 31; num_threads++) {

        // Read chunk from file for each process
        fseek(file, rank * chunk_size, SEEK_SET);
        fread(chunk_buffer, chunk_size, 1, file);
        chunk_buffer[chunk_size] = '\0';

        // Start timer
        start_time = MPI_Wtime();

        // Find target phrase in chunk using OpenMP
        local_phrase_count = 0;
        #pragma omp parallel for num_threads(num_threads) reduction(+:local_phrase_count)
        for (long int i = 0; i < chunk_size; i++) {
            if (strncmp(&chunk_buffer[i], target_phrase, strlen(target_phrase)) == 0) {
                local_phrase_count++;
            }
        }

        // Reduce phrase count across processes
        MPI_Reduce(&local_phrase_count, &phrase_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // End timer
        end_time = MPI_Wtime();

        // Print results to file
        if (rank == 0) {
            FILE *results_file = fopen("outputHybrid.txt", "a");
            fprintf(results_file, "%d\t%f\n", num_threads, end_time - start_time);
            fclose(results_file);
        }

    }

    // Close file
    fclose(file);

    // Free allocated memory
    free(chunk_buffer);

    // Finalize MPI
    MPI_Finalize();

    return 0;
}
