#ifndef __COMPUTE_H__
#define __COMPUTE_H__

#include <stdbool.h>

#include "messages.h"

/* Set settings for the computation */
void set_compute(const message *msg);
void set_chunk(const message *msg);

/* Prepare a message with data for one pixel*/
bool compute_chunk(message *compute_data);

// void compute_chunk_burst(message *msg);

#endif