#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int num_threads = omp_get_max_threads();
    char *filename = "file_6mb.txt";
    char *target_phrase = "with the";
    char buffer[BUFFER_SIZE];
    int phrase_count = 0;
    double start_time, end_time;

    // open file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        return 1;
    }

    // start timer
    start_time = omp_get_wtime();

    // loop through file
    while (fgets(buffer, BUFFER_SIZE, file)) {
        // loop through words in buffer
        char *phrase = strstr(buffer, target_phrase);
        while (phrase != NULL) {
            // increment phrase count
            #pragma omp atomic
            phrase_count++;
            // move pointer to the end of the phrase to search for next occurrence
            phrase += strlen(target_phrase);
            phrase = strstr(phrase, target_phrase);
        }
    }

    // end timer
    end_time = omp_get_wtime();

    // close file
    fclose(file);

    // print results
    printf("Target phrase \"%s\" appears %d times in file \"%s\".\n", target_phrase, phrase_count, filename);
    printf("Execution time: %f seconds using %d threads.\n", end_time - start_time, num_threads);

    return 0;
}
