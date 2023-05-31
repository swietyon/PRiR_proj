#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define BUFFER_SIZE 1024
#define EXTRA_CHARS 100

int main(int argc, char *argv[])
{
    int max_num_threads = 8;
    char *filename = "file_6mb.txt";
    char *target_phrase = "with the";
    int phrase_count = 0;
    double start_time, end_time;

    // open file
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error: Could not open file.\n");
        return 1;
    }

    // get file size
    fseek(file, 0L, SEEK_END);
    long int file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    // allocate memory for file buffer
    char *file_buffer = malloc(file_size + 1);
    if (file_buffer == NULL)
    {
        printf("Error: Could not allocate memory.\n");
        return 1;
    }

    // read file into buffer
    fread(file_buffer, file_size, 1, file);
    file_buffer[file_size] = '\0';

    // append extra characters at the end
    for (int i = 0; i < EXTRA_CHARS; i++)
    {
        file_buffer[file_size + i] = ' ';
    }

    // close file
    fclose(file);

    // write results to file
    FILE *output_file = fopen("output_openmp.txt", "w");
    if (output_file == NULL)
    {
        printf("Error: Could not open output file.\n");
        return 1;
    }

    for (int num_threads = 1; num_threads <= max_num_threads; num_threads++)
    {
        // set number of threads
        omp_set_num_threads(num_threads);

        // start timer
        start_time = omp_get_wtime();

        // initialize phrase count
        phrase_count = 0;

        // loop through file buffer in parallel
        #pragma omp parallel for reduction(+ \
                                        : phrase_count)
        for (long int i = 0; i < file_size + EXTRA_CHARS; i += BUFFER_SIZE)
        {
            // copy buffer to local variable
            char local_buffer[BUFFER_SIZE + strlen(target_phrase) + 1];
            strncpy(local_buffer, file_buffer + i, BUFFER_SIZE);
            local_buffer[BUFFER_SIZE] = '\0';

            // loop through characters in local buffer
            char *phrase = strstr(local_buffer, target_phrase);
            while (phrase != NULL)
            {
                // check if the phrase is a complete word
                if ((phrase == local_buffer || phrase[-1] == ' ') && (phrase[strlen(target_phrase)] == ' '))
                {
                    // increment phrase count
                    phrase_count++;
                }

                // move pointer to the end of the phrase to search for next occurrence
                phrase = strstr(phrase + 1, target_phrase);
            }
        }

        // end timer
        end_time = omp_get_wtime();

        // print and write results to file
        printf("Target phrase \"%s\" appears %d times in file \"%s\" using %d threads.\n", target_phrase, phrase_count, filename, num_threads);
        fprintf(output_file, "Target phrase \"%s\" appears %d times in file \"%s\" using %d threads.\n", target_phrase, phrase_count, filename, num_threads);
        fprintf(output_file, "Execution time: %f seconds\n", end_time - start_time);
    }

    // free file buffer memory
    free(file_buffer);

    fclose(output_file);

    return 0;
}
