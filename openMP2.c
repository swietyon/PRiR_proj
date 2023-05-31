#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    char *filename = "file_6mb.txt";
    char *target_phrase = "with the";
    int phrase_count = 0;
    double start_time, end_time;

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

    // read file before parallel region
    while (!feof(file)) {
        char local_buffer[BUFFER_SIZE];
        int local_count = 0;

        fgets(local_buffer, BUFFER_SIZE, file);

        // loop through words in buffer
        char *phrase = strstr(local_buffer, target_phrase);
        while (phrase != NULL) {
            // increment local phrase count
            local_count++;
            // move pointer to the end of the phrase to search for next occurrence
            phrase += strlen(target_phrase);
            phrase = strstr(phrase, target_phrase);
        }

        // accumulate local counts
        phrase_count += local_count;
    }

    // close file
    fclose(file);

    // start timer
    start_time = omp_get_wtime();

    // loop through number of threads
    for (int num_threads = 1; num_threads <= 32; num_threads++) {
        // set number of threads
        omp_set_num_threads(num_threads);

        // reset phrase count
        phrase_count = 0;

        // parallel region
        #pragma omp parallel shared(phrase_count)
        {
            int local_count = 0;

            // open file inside parallel region
            FILE *file = fopen(filename, "r");
            if (file == NULL) {
                printf("Error: Could not open file.\n");
                exit(1);
            }

            // loop through file in parallel
            while (!feof(file)) {
                char buffer[BUFFER_SIZE];
                fgets(buffer, BUFFER_SIZE, file);

                // loop through words in buffer
                char *phrase = strstr(buffer, target_phrase);
                while (phrase != NULL) {
                    // increment local phrase count
                    local_count++;
                    // move pointer to the end of the phrase to search for next occurrence
                    phrase += strlen(target_phrase);
                    phrase = strstr(phrase, target_phrase);
                }
            }

            // accumulate local counts
            #pragma omp critical
            {
                phrase_count += local_count;
            }

            // close file inside parallel region
            fclose(file);
        }

        // print results to output file
        fprintf(out_file, "%d  %d\n", num_threads, phrase_count);
    }

    // end timer
    end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;

    // print total execution time
    fprintf(out_file, "Total execution time: %f seconds\n", elapsed_time);

    // close output file
    fclose(out_file);

    return 0;
}
