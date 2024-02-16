#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>

#include "messages.h"
#include "utils.h"

void my_assert(bool r, const char *fcname, int line, const char *fname)
{
    if (!r){
        fprintf(stderr, "ERORR: my_assert FAIL: %s() line %d in %s\n", fcname, line, fname);
        exit(105);
    }
}

void* my_malloc(size_t size)
{
    void *ret = malloc(size);
    if (!ret) {
        fprintf(stderr, "ERROR: Memory allocation error\n");
        exit(101);
    }
    return ret;
}

int send_message(const message *msg, uint8_t msg_buf[], int *msg_len, int pipe_out){
    fill_message_buf(msg, msg_buf, sizeof(message), msg_len);
    return (write(pipe_out, msg_buf, *msg_len) == *msg_len) ? 1 : 0;
}

void call_termios(int reset){
    static struct termios tio, tioOld;
    tcgetattr(STDIN_FILENO, &tio);
    if(reset){
    tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
    } else {
        tioOld = tio;
        cfmakeraw(&tio);
        tio.c_oflag |= OPOST;
        tcsetattr(STDIN_FILENO, TCSANOW, &tio);
    }
}

void info(const char *str)
{
    fprintf(stderr, "INFO: %s\n", str);
}
void debug(const char *str)
{
    fprintf(stderr, "DEBUG: %s\n", str);
}
void error(const char *str)
{
    fprintf(stderr, "ERROR: %s\n", str);
}
void warn(const char *str)
{
    fprintf(stderr, "WARNING: %s\n", str);
}
