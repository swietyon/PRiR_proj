#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    char *filename = "file.txt";
    char *target_phrase = "it";
    char buffer[BUFFER_SIZE];
    int phrase_count = 0;
    clock_t start_time, end_time;

    // open file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        return 1;
    }

    // start timer
    start_time = clock();

    // loop through file
    while (fgets(buffer, BUFFER_SIZE, file)) {
        // loop through words in buffer
        char *phrase = strstr(buffer, target_phrase);
        while (phrase != NULL) {
            // increment phrase count
            phrase_count++;
            // move pointer to the end of the phrase to search for next occurrence
            phrase += strlen(target_phrase);
            phrase = strstr(phrase, target_phrase);
        }
    }

    // end timer
    end_time = clock();

    // close file
    fclose(file);

    // print results
    printf("Target phrase \"%s\" appears %d times in file \"%s\".\n", target_phrase, phrase_count, filename);
    printf("Execution time: %f seconds.\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    return 0;
}
