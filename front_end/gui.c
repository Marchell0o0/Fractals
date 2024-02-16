
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

#include "xwin_sdl.h"
#include "utils.h"
#include "compute_control.h"

#include "gui.h"

static struct {
    int w;
    int h;
    unsigned char *img;
} gui = { .img = NULL};

void gui_init(void)
{
    get_grid_size(&gui.w, &gui.h);
    gui.img = my_malloc(gui.w * gui.h * 3);
    my_assert(xwin_init(gui.w, gui.h) == 0, __func__, __LINE__, __FILE__);
}

void gui_cleanup(void)
{
    if (gui.img) {
        free(gui.img);
        gui.img = NULL;
    }
    xwin_close();
}

void gui_refresh(void)
{
    if (gui.img){
        update_image(gui.w, gui.h, gui.img);
        fractal_draw(gui.w, gui.h, gui.img);
    }
}

void gui_save_image(void){
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(gui.img, gui.w, gui.h, 24, gui.w * 3, 0x0000FF, 0x00FF00, 0xFF0000, 0);
    if (surface == NULL) {
        fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
        return;
    }

    static int frame_count = 0; // frame counter
    char filename[256];
    snprintf(filename, sizeof(filename), "frames/frame%04d.png", frame_count++);
    IMG_SavePNG(surface, filename); // save the SDL_Surface as PNG

    SDL_FreeSurface(surface);
}