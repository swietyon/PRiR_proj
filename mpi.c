#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int rank, num_procs;
    char *filename = "file.txt";
    char *target_phrase = "it";
    char buffer[BUFFER_SIZE];
    int phrase_count = 0;
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

    // Start timer
    start_time = MPI_Wtime();

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
    char *chunk_buffer = (char*) malloc(chunk_size + strlen(target_phrase));

    // Read chunk from file for each process
    fseek(file, rank * chunk_size, SEEK_SET);
    fread(chunk_buffer, chunk_size, 1, file);
    chunk_buffer[chunk_size] = '\0';

    // Find target phrase in chunk
    char *phrase = strstr(chunk_buffer, target_phrase);
    while (phrase != NULL) {
        // Increment phrase count
        phrase_count++;

        // Move pointer to the end of the phrase to search for next occurrence
        phrase += strlen(target_phrase);
        phrase = strstr(phrase, target_phrase);
    }

    // Reduce phrase count across processes
    int global_phrase_count;
    MPI_Reduce(&phrase_count, &global_phrase_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // End timer
    end_time = MPI_Wtime();

    // Close file
    fclose(file);

    // Print results
    if (rank == 0) {
        printf("Target phrase \"%s\" appears %d times in file \"%s\".\n", target_phrase, global_phrase_count, filename);
        printf("Execution time: %f seconds using %d processes.\n", end_time - start_time, num_procs);
    }

    // Finalize MPI
    MPI_Finalize();

    return 0;
}
