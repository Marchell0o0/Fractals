#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "event_queue.h"
#include "utils.h"
#include "xwin_sdl.h"

#include "terminal_redirect_thread.h"

#define OUTPUT_FILE_PATH "output.txt"

// my_assert(file != NULL, __func__, __LINE__, __FILE__);
void* terminal_redirect_thread(void* d){
    FILE* my_stdout = freopen("output.txt", "w", stdout);
    FILE* my_stderr = freopen("output.txt", "w", stderr);
    FILE* file = fopen(OUTPUT_FILE_PATH, "r");

    // Get the initial size of the file
    fseek(file, 0, SEEK_END);
    long int oldSize = ftell(file);
    rewind(file);

    char* buffer = NULL;
    size_t bufferSize = 0;

    bool quit = false;

    while (!quit) {
        fflush(my_stdout);
        fflush(my_stderr);

        fseek(file, 0, SEEK_END);
        long int newSize = ftell(file);

        if (newSize > oldSize) {
            // There is new data in the file, read it
            bufferSize = newSize - oldSize;
            buffer = realloc(buffer, bufferSize + 1);
            fseek(file, oldSize, SEEK_SET);
            fread(buffer, 1, bufferSize, file);
            buffer[bufferSize] = '\0';  // Null-terminate the buffer
            terminal_redraw(buffer, bufferSize);

            oldSize = newSize;
        }

        // Sleep for a while to reduce CPU usage
        usleep(10000);  // sleep for 10ms
        quit = is_quit();
    }

    if (my_stdout != NULL) {
        fclose(my_stdout);
    }
    if (my_stderr != NULL) {
        fclose(my_stderr);
    }

    free(buffer);
    fclose(file);
    return NULL;
}