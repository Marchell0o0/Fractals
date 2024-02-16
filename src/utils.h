

#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "messages.h"

void my_assert(bool r, const char *fcname, int line, const char *fname);

void* my_malloc(size_t size);

int send_message(const message *msg, uint8_t msg_buf[], int *msg_len, int pipe_out);

void call_termios(int reset);

void info(const char *str);
void debug(const char *str);
void error(const char *str);
void warn(const char *str);

#endif