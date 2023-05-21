#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    char *filename = "Ryz.txt";
    char *target_phrase = "on";
    char buffer[BUFFER_SIZE];
    int phrase_count = 0;
    double start_time, end_time, tick;

    // open file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        return 1;
    }

    // open output file
    FILE *out_file = fopen("outputOpenMP.txt", "w");
    if (out_file == NULL) {
        printf("Error: Could not open output file.\n");
        return 1;
    }

    // loop through number of threads
    for (int num_threads = 1; num_threads <= 32; num_threads++) {
        // set number of threads
        omp_set_num_threads(num_threads);

        // start timer
        start_time = omp_get_wtime();

        // loop through file
        #pragma omp parallel shared(phrase_count)
        {
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
        }

        // end timer
        end_time = omp_get_wtime();
        //tick = omp_get_wtick(); nie działa :/
        double elapsed_time = ((end_time - start_time) );
        // print results to output file
        fprintf(out_file, " %d  ",  num_threads);
        fprintf(out_file, " %f \n", elapsed_time);

        // reset phrase count
        phrase_count = 0;
        // reset file position to the beginning
        rewind(file);
    }

    // close files
    fclose(file);
    fclose(out_file);

    return 0;
}
