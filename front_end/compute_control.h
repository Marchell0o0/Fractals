#ifndef __COMPUTE_CONTROL_H__
#define __COMPUTE_CONTROL_H__

#include <stdbool.h>

#include "messages.h"

/* Initialize the grid and free it after the computation */
void computation_init(void);
void computation_cleanup(void);

/* Flags of the computation */
bool is_computing_module(void);
bool is_computing_locally(void);
bool is_done(void);
void set_aborted(void);
void set_not_aborted(void);
bool is_aborted(void);
bool is_saving(void);

void set_parameters(const double parameters[]);

void get_parameters(double parameters[]);


void reset_computation(void);
void clean_image(void);

void get_grid_size(int *w, int *h);

/* Messages for comp module*/
/* Prepare a message to set the info about the computation */
bool set_compute(message *msg);

/* Prepare a message for the next chunk computation*/
bool compute_module(message *msg);

/* Move data from the message to the grid and finish the computation if needed*/
void update_data(const msg_compute_data *compute_data);


/* Local computations for the animation*/
/* Function to compute the Julia set images locally using multiple threads */
void draw_animation(message *msg);

/* Function to compute the iteration for a specific range of rows*/
void* compute_rows(void* args);

/* Color every pixel according to the number of iterations */
void update_image(int w, int h, unsigned char *img);



#endif