#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define BUFFER_SIZE 10240

int main(int argc, char *argv[]) {
    int rank, num_procs;
    char *filename = "file_6mb.txt";
    char *target_phrase = "outside";
    char buffer[BUFFER_SIZE];
    double start_time, end_time;
    int phrase_count = 0;

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

    // Start timer
    

    // Iterate over different number of threads
    for (int i = 1; i <= num_procs; i++) {
        // Only execute for the first `i` threads
        start_time = MPI_Wtime();
        if (rank < i) {
            // Calculate chunk size for each process
            long int chunk_size = file_size / i;
            if (rank == i - 1) {
                // Last process gets any remaining bytes
                chunk_size += file_size % i;
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

            // Free memory allocated for chunk buffer
            free(chunk_buffer);
        }

        // Reduce phrase count across processes
        int global_phrase_count;
        MPI_Reduce(&phrase_count, &global_phrase_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // End timer
        end_time = MPI_Wtime();

        // Print results for first process
        if (rank == 0) {
            printf("Target phrase \"%s\" appears %d times in file \"%s\" using %d processes.\n", target_phrase, global_phrase_count, filename, i);
            printf("Execution time: %f seconds.\n", end_time - start_time);
        }

        // Write results to file
        if (rank == 0) {
            FILE *output_file = fopen("outputMPI.txt", "a");
            fprintf(output_file, "%d    %f\n", i, end_time - start_time);
            fclose(output_file);
        }

        // Reset phrase count for next iteration
        phrase_count = 0;
    }

    // Close file
    fclose(file);

    // Finalize MPI
    MPI_Finalize();

    return 0;
}
