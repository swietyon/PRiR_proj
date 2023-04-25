

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int num_threads = 7;
    omp_set_num_threads(num_threads);
    char *filename = "file.txt";
    char *target_phrase = "it";
    char buffer[BUFFER_SIZE];
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

    // close file
    fclose(file);

    // start timer
    start_time = omp_get_wtime();

// loop through file buffer in parallel
#pragma omp parallel for reduction(+ \
                                   : phrase_count)
    for (long int i = 0; i < file_size; i += BUFFER_SIZE)
    {
        // copy buffer to local variable
        char local_buffer[BUFFER_SIZE + strlen(target_phrase) + 1];
        strncpy(local_buffer, file_buffer + i, BUFFER_SIZE);
        local_buffer[BUFFER_SIZE] = '\0';

        // loop through words in local buffer
        char *phrase = strstr(local_buffer, target_phrase);
        while (phrase != NULL)
        {
            // increment phrase count
            phrase_count++;
            // move pointer to the end of the phrase to search for next occurrence
            phrase += strlen(target_phrase);
            phrase = strstr(phrase, target_phrase);
        }
    }

    // end timer
    end_time = omp_get_wtime();

    // free file buffer memory
    free(file_buffer);

    // print results
    printf("Target phrase \"%s\" appears %d times in file \"%s\".\n", target_phrase, phrase_count, filename);
    printf("Execution time: %f seconds using %d threads.\n", end_time - start_time, num_threads);

    return 0;
}
