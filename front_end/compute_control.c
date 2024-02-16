#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#include "compute_control.h"
#include "utils.h"
#include "gui.h"
#include "event_queue.h"

#define PROPORTION_OF_BLACK_PIXELS 0.0005
#define DAMPING_FACTOR 0.07
#define LIMIT_OF_FRAMES 10000
#define NUMBER_OF_THREADS 12

// Settings and info for the computation
static struct{
    // c constant
    double c_re; 
    double c_im;

    int grid_w; // Width of the grid
    int grid_h; // Height of the grid

    uint16_t *grid; // Grid to store computed values

    uint8_t chunk_n_re; // Width of one chunk
    uint8_t chunk_n_im; // Height of one chunk

    int nbr_chunks; // Total number of chunks

    // Iteration limit
    int starting_n; 

    // Real and Imaginary ranges
    double starting_range_re_min;
    double starting_range_re_max;
    double starting_range_im_min;
    double starting_range_im_max;

    /* All of this variables could be changed during copmutations*/
    int current_n;

    double current_range_re_min;
    double current_range_re_max;
    double current_range_im_min;
    double current_range_im_max;

    double re_center;
    double im_center;

    // Step sizes
    double d_re; 
    double d_im;

    // Coordinates for drawing
    int cur_x; 
    int cur_y;

    // Chunk ID
    int cid; 
   
    // Chunk coordinates
    double chunk_re; 
    double chunk_im; 
    
    // Minimal number of iterations to start from
    int min_k; 

    double zoom_factor;

    bool save_the_images;

    // Flags of the computation
    bool computing_module; // Computation is in progress
    bool computing_locally; // Computation is in progress
    bool done; // Computation is done
    bool aborted; // Computation is aborted (waits for done message from the module and sets aborted to false)

} comp = {
    .starting_n = 216,
    .min_k = 0,

    .re_center = 0.3,
    .im_center = -0.3,

    // .starting_range_re_min = -1.7777777777,
    // .starting_range_re_max = 1.7777777777,
    // .starting_range_im_min = -1,
    // .starting_range_im_max = 1,

    .grid = NULL,
 
    .grid_w = 1280,
    .grid_h = 720,
    
    // .grid_w = 1920,
    // .grid_h = 1080,

    // .grid_w = 3840,
    // .grid_h = 2160,
    
    .chunk_n_re = 128, 
    .chunk_n_im = 72,

    // .c_re = -0.4, 
    // .c_im = 0.6,

    .c_re = -0.7269,
    .c_im = 0.1889,

    // .c_re = -1.77029598415682387070596815828735774662104581,
    // .c_im =  -0.005852576008784477443909236300355828076899285495,

    .zoom_factor = 0.03,

    .save_the_images = true,

    .computing_module = false,
    .computing_locally = false,
    .done = false,
    .aborted = false,
};

void set_parameters(const double parameters[]) {
    comp.c_re = parameters[0];
    comp.c_im = parameters[1];
    comp.starting_n = parameters[2];
    comp.starting_range_re_min = parameters[3];
    comp.starting_range_re_max = parameters[4];
    comp.starting_range_im_min = parameters[5];
    comp.starting_range_im_max = parameters[6];
    comp.zoom_factor = parameters[7];
    info("Saved settings");
}

void get_parameters(double parameters[]){
    parameters[0] = comp.c_re;
    parameters[1] = comp.c_im;
    parameters[2] = comp.starting_n;
    parameters[3] = comp.starting_range_re_min;
    parameters[4] = comp.starting_range_re_max;
    parameters[5] = comp.starting_range_im_min;
    parameters[6] = comp.starting_range_im_max;
    parameters[7] = comp.zoom_factor;
}

static void update_step_size(void){
    comp.d_re = (comp.current_range_re_max - comp.current_range_re_min) / (1. * comp.grid_w);
    comp.d_im = -(comp.current_range_im_max - comp.current_range_im_min) / (1. * comp.grid_h);
}

void computation_init(void){
    comp.grid = my_malloc(comp.grid_w * comp.grid_h * sizeof(uint16_t));
    comp.nbr_chunks = (comp.grid_w * comp.grid_h) / (comp.chunk_n_re * comp.chunk_n_im);

    comp.current_n = comp.starting_n;
    comp.starting_range_im_max = comp.im_center + 1.0;
    comp.starting_range_im_min = comp.im_center - 1.0;
    comp.starting_range_re_max = comp.re_center + 1.777777777;
    comp.starting_range_re_min = comp.re_center - 1.777777777;
    
    comp.current_range_re_min = comp.starting_range_re_min;
    comp.current_range_re_max = comp.starting_range_re_max;
    comp.current_range_im_min = comp.starting_range_im_min;
    comp.current_range_im_max = comp.starting_range_im_max;

    
    update_step_size();
}

void computation_cleanup(void){
    if (comp.grid){
    free(comp.grid);
    }
    comp.grid = NULL;
}

bool is_computing_module(void){ return comp.computing_module; }
bool is_computing_locally(void){ return comp.computing_locally; }
bool is_done(void){ return comp.done; }

void set_aborted(void){ comp.computing_module = false; comp.aborted = true; }

void set_not_aborted(void){ comp.aborted = false; }
bool is_aborted(void){ return comp.aborted; }

bool is_saving(void) { return comp.save_the_images; };

void get_grid_size(int *w, int *h){
    *w = comp.grid_w;
    *h = comp.grid_h;
}

bool set_compute(message *msg){
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);

    bool ret  = !is_computing_module();
    if (ret){
        msg->type = MSG_SET_COMPUTE;
        msg->data.set_compute.c_re = comp.c_re;
        msg->data.set_compute.c_im = comp.c_im;
        msg->data.set_compute.d_re = comp.d_re;
        msg->data.set_compute.d_im = comp.d_im;
        msg->data.set_compute.n = comp.current_n;
        comp.done = false;
        fprintf(stderr, "INFO: Setting compute, resolution %dx%d\n", comp.grid_w, comp.grid_h);

    }
    return ret;
}

bool compute_module(message *msg){
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);

    if (!is_computing_module()){ // First chunk
        comp.cid = 0;
        comp.computing_module = true;
        comp.done = false;
        comp.cur_x = comp.cur_y = 0; // Start computation of the whole image
        comp.chunk_re = comp.current_range_re_min; // Upper-"left" corner
        comp.chunk_im = comp.current_range_im_max; // "Upper"-left corner
        msg->type = MSG_COMPUTE;
    } else { // Next chunks
        comp.cid += 1;
        if (comp.cid < comp.nbr_chunks) {
            comp.cur_x += comp.chunk_n_re;
            comp.chunk_re += comp.chunk_n_re * comp.d_re;
            if (comp.cur_x >= comp.grid_w) {
                comp.chunk_re = comp.current_range_re_min;
                comp.chunk_im += comp.chunk_n_im * comp.d_im;
                comp.cur_x = 0;
                comp.cur_y += comp.chunk_n_im;
            }
            msg->type = MSG_COMPUTE;
        } else { // Everything has been computed
        }
    }

    fprintf(stderr, "INFO: Sending a new chunk of data cid: %d\n", comp.cid);

    if (comp.computing_module && msg->type == MSG_COMPUTE){
        msg->data.compute.cid = comp.cid;
        msg->data.compute.re = comp.chunk_re;
        msg->data.compute.im = comp.chunk_im;
        msg->data.compute.n_re = comp.chunk_n_re;
        msg->data.compute.n_im = comp.chunk_n_im;
    }
    return is_computing_module();
}

void update_data(const msg_compute_data *compute_data){
    my_assert(compute_data != NULL, __func__, __LINE__, __FILE__);

    if (compute_data->cid == comp.cid) {
        const int idx = (comp.cur_x + compute_data->i_re) + (comp.cur_y + compute_data->i_im) * comp.grid_w;
        if (idx >= 0 && idx < (comp.grid_w * comp.grid_h)) {
            comp.grid[idx] = compute_data->iter;
        }
        if ((comp.cid + 1) >= comp.nbr_chunks && (compute_data->i_re + 1) == comp.chunk_n_re && (compute_data->i_im + 1) == comp.chunk_n_im){
            comp.done = true;
            comp.computing_module = false;
        }
    } else {
        fprintf(stderr, "Received chunk with unexpected id %d\n", compute_data->cid);
    }
}

void reset_computation(void){
    
    comp.current_n = comp.starting_n;
    comp.current_range_re_min = comp.starting_range_re_min;
    comp.current_range_re_max = comp.starting_range_re_max;
    comp.current_range_im_min = comp.starting_range_im_min;
    comp.current_range_im_max = comp.starting_range_im_max;
    update_step_size();
    
    comp.computing_module = false,
    comp.computing_locally = false,
    comp.done = false,
    comp.aborted = false,

    comp.cid = 0;
    comp.min_k = 0;
    return;
}

void clean_image(void){
    for (int i = 0; i < comp.grid_h*comp.grid_w; ++i){
        comp.grid[i] = 0;
    }
}

static int compute_pixel(double real, double imag){
    double new_real, new_imag;
    double real_squared, imag_squared;

    int k;
    for (k = (comp.min_k * 0.7); k <= comp.current_n; ++k){ 
        real_squared = real * real;
        imag_squared = imag * imag;

        if(real_squared + imag_squared > 4.0) {
            break;
        }

        new_imag = 2.0 * real * imag + comp.c_im;
        new_real = real_squared - imag_squared + comp.c_re;

        real = new_real;
        imag = new_imag;
    }
    return k;
}

static int compute_pixel_four_points(double real, double imag){
    int sum = 0;
    double d_re = comp.d_re / 4;
    double d_im = comp.d_im / 4;
    sum += compute_pixel(real - d_re, imag + d_im);
    sum += compute_pixel(real + d_re, imag + d_im);
    sum += compute_pixel(real - d_re, imag - d_im);
    sum += compute_pixel(real + d_re, imag - d_im);

    // Return the average of the four corners
    return sum / 4;
}

typedef struct {
    int start;
    int end;
} ThreadData;


void* compute_rows(void* args){
    // Cast the argument to ThreadData
    ThreadData* data = (ThreadData*)args;

    // Loop over every pixel in given rows and calculate iterations for it
    for (int im = data->start; im < data->end; ++im){
        double real = comp.current_range_re_min;
        double imag = comp.current_range_im_max + im * comp.d_im;
        for (int re = 0; re < comp.grid_w; ++re){
            int idx = re + im * comp.grid_w;
            comp.grid[idx] = compute_pixel_four_points(real, imag);
            real = real + comp.d_re;
        }
    }

    free(args);
    return NULL;
}

static void zoom(void){
    double re_center = (comp.current_range_re_min + comp.current_range_re_max) / 2.0;
    double im_center = (comp.current_range_im_min + comp.current_range_im_max) / 2.0;

    double re_range = comp.current_range_re_max - comp.current_range_re_min;
    double im_range = comp.current_range_im_max - comp.current_range_im_min;

    comp.current_range_re_min = re_center - (re_range * (1 - comp.zoom_factor)) / 2.0;
    comp.current_range_re_max = re_center + (re_range * (1 - comp.zoom_factor)) / 2.0;

    comp.current_range_im_min = im_center - (im_range * (1 - comp.zoom_factor)) / 2.0;
    comp.current_range_im_max = im_center + (im_range * (1 - comp.zoom_factor)) / 2.0;

    update_step_size();
}

static void change_n(void){
    // Count the number of black pixels and adjust the iteration limit accordingly
    long long num_black_pixels = 0;
    for (int idx = 0; idx < comp.grid_h * comp.grid_w; ++idx){
        if (comp.grid[idx] >= comp.current_n * 0.9){ 
            /* 
            IMPORTANT: 
            The pixel is counted as black even when it's only close to the limit, 
            because with super-sampling they rarely reach actual limit
            */
            num_black_pixels++;
        }
    }

    float proportion_black = (float) num_black_pixels / (comp.grid_h * comp.grid_w);
    float change_factor;

    change_factor = proportion_black / PROPORTION_OF_BLACK_PIXELS;

    // adjust n with the change_factor
    comp.current_n = (int)(comp.current_n * (1 + (change_factor - 1) * DAMPING_FACTOR));

    // fprintf(stderr, "DEBUG: Proportion of black pixels: %f\n", proportion_black);
    // fprintf(stderr, "DEBUG: Change factor: %f\n", change_factor);
    // fprintf(stderr, "DEBUG: n after adjustment: %d\n", comp.n);
}

static void change_min_k(void) {
    int min_k = comp.current_n;  // Start with maximum possible k value

    for (int idx = 0; idx < comp.grid_h * comp.grid_w; ++idx) {
        if (comp.grid[idx] < min_k) {
            min_k = comp.grid[idx];
        }
    }
    comp.min_k = min_k;
}

void draw_animation(message *msg){
    comp.computing_locally = true;

    // Divide the image into equal parts for the number of threads
    int rows_per_thread = comp.grid_h / NUMBER_OF_THREADS;

    int i = 0;
    while(i <= LIMIT_OF_FRAMES){
    
        // Create the threads and pass the data to them
        pthread_t threads[NUMBER_OF_THREADS];
        for (int t = 0; t < NUMBER_OF_THREADS; ++t){
            ThreadData* data = malloc(sizeof(ThreadData));
            data->start = t * rows_per_thread;
            data->end = (t == NUMBER_OF_THREADS - 1) ? comp.grid_h : (t+1) * rows_per_thread;

            if (pthread_create(&threads[t], NULL, compute_rows, data) != 0){
                fprintf(stderr, "ERROR: Could not create thread\n");
                return;
            }
        }

    // Wait until they are finished
    for (int t = 0; t < NUMBER_OF_THREADS; ++t){
        if (pthread_join(threads[t], NULL) != 0){
            fprintf(stderr, "ERROR: Could not join thread\n");
            return;
        }
    }


    gui_refresh(); // Draw the image

    if (comp.save_the_images) { gui_save_image(); }

    zoom(); // Zoom the ranges of the computation

    if (comp.aborted) { break; }

    fprintf(stderr, "INFO: Current frame n: %d minimal k: %d", comp.current_n, comp.min_k);

    change_min_k();
    change_n(); // Change n to keep the proportion of black pixels
    
    i++;
    }
    // clean_image();

    if (comp.aborted){
        comp.aborted = false;
        comp.computing_locally = false;
    } else {
        fprintf(stderr, "\n");
    }

}

typedef struct {
    int r, g, b;
} Color;

Color interpolate(Color a, Color b, double t) {
    Color result = {
        a.r * (1 - t) + b.r * t,
        a.g * (1 - t) + b.g * t,
        a.b * (1 - t) + b.b * t
    };
    return result;
}

void update_image(int w, int h, unsigned char *img)
{
    my_assert(img && comp.grid && w == comp.grid_w && h == comp.grid_h, __func__, __LINE__,__FILE__);

    Color colors[] = {
        {0, 0, 80},       // Dark Blue 
        {135, 208, 255},  // Light Blue
        {255, 255, 255},  // White
        {255, 215, 70},   // Yellow
        {175, 50, 15},    // Red
        {0, 0, 0},        // Black
    };

    // manually specified CDF
    double cdf[] = {
        0.2, // Dark Blue appears in this % of the image
        0.3, // Light Blue
        0.4, // White 
        0.7, // Yellow
        0.9, // Red
        1.0  // Black
    };

    // Color colors[] = {
    //     {7, 72, 42},
    //     {154, 167, 72},
    //     {255, 255, 255},
    // };

    // double cdf[] = {
    //     0.2,
    //     0.7,
    //     1.0,
    // };

    int num_colors = sizeof(colors) / sizeof(Color);
    int num_weights = sizeof(cdf) / sizeof(double);

    my_assert(num_colors == num_weights, __func__, __LINE__,__FILE__);

    for (int i = 0; i < w*h; ++i) {

        double t = 1.0 * (comp.grid[i] - comp.min_k) / ((comp.current_n - comp.min_k) + 1.0);

        if (t < 0) { t = 0; }

        // determine the color index based on the CDF
        int index = 0;
        while (index < num_colors && t > cdf[index]) {
            ++index;
        }
        index = index == num_colors ? index - 1 : index;

        // calculate the local t value for interpolation
        double local_t = index == 0 ? t / cdf[index] : (t - cdf[index - 1]) / (cdf[index] - cdf[index - 1]);

        Color c = index == num_colors - 1 ? colors[index] : interpolate(colors[index], colors[index+1], local_t);

        *(img++) = c.r;
        *(img++) = c.g;
        *(img++) = c.b;
    }
}

