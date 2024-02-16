#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "utils.h"
#include "messages.h"

#include "compute.h"

static int compute_pixel(int re, int im);

static struct {
    int cid;               // Chunk's id
    double re;             // Start of real coordinates of the chunk
    double im;             // Start of imaginary coordinates of the chunk
    uint8_t n_re;          // Width of one chunk
    uint8_t n_im;          // Height of one chunk
    double c_re;           // Real part of the constant c
    double c_im;           // Imaginary part of the constant c
    double d_re;           // Real step size
    double d_im;           // Imaginary step size
    int n;                 // Number of iterations
} chunk = { .cid = 0};


void set_compute(const message *msg){
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    chunk.c_re = msg->data.set_compute.c_re;
    chunk.c_im = msg->data.set_compute.c_im;
    chunk.d_re = msg->data.set_compute.d_re; 
    chunk.d_im = msg->data.set_compute.d_im; 
    chunk.n = msg->data.set_compute.n; 
}
void set_chunk(const message *msg){
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    chunk.cid = msg->data.compute.cid; 
    chunk.re = msg->data.compute.re; 
    chunk.im = msg->data.compute.im; 
    chunk.n_re = msg->data.compute.n_re;
    chunk.n_im = msg->data.compute.n_im;
}

static int re = 0;
static int im = 0;

bool compute_chunk(message *compute_data) {
    if (re < chunk.n_re && im < chunk.n_im) {
        compute_data->data.compute_data.cid = chunk.cid;
        compute_data->data.compute_data.i_re = re;
        compute_data->data.compute_data.i_im = im;
        compute_data->data.compute_data.iter = compute_pixel(re, im);
        im++;
        if (im == chunk.n_im) {
            im = 0;
            re++;
        }
        return true;
    } else {
        re = 0;
        im = 0;
        return false;
    }
}

static int compute_pixel(int re, int im){
    double new_real, new_imag;
    double real, imag;
    double real_squared, imag_squared;

    real = chunk.re + (re * chunk.d_re);
    imag = chunk.im + (im * chunk.d_im);

    int k;
    for (k = 0; k <= chunk.n; ++k){
        real_squared = real * real;
        imag_squared = imag * imag;

        if(real_squared + imag_squared > 4.0) {
            return k;
        }

        new_imag = 2.0 * real * imag + chunk.c_im;
        new_real = real_squared - imag_squared + chunk.c_re;

        real = new_real;
        imag = new_imag;
    }
    return k;
}

