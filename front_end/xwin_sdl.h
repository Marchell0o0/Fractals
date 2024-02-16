/*
 * Filename: xwin_sdl.h
 * Date:     2015/06/18 14:37
 * Author:   Jan Faigl
 */

#ifndef __XWIN_SDL_H__
#define __XWIN_SDL_H__

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

typedef enum {
    NONE = -1,
    BUTTON_REQUEST_VERSION,
    BUTTON_SET_COMPUTING_PARAMS,
    BUTTON_COMPUTE_WITH_MODULE,
    BUTTON_DRAW_ANIMATION,
    BUTTON_PAUSE_COMPUTATION,
    BUTTON_RESET_COMPUTATION,
    BUTTON_RESET_IMAGE,
    BUTTON_SAVE_IMAGE,
    BUTTON_QUIT_PROGRAM,
    BUTTON_SAVE_SETTINGS,
    TEXT_BOX_RESOLUTION_WIDTH,
    TEXT_BOX_RESOLUTION_HEIGHT,
    TEXT_BOX_C_CONSTANT_REAL,
    TEXT_BOX_C_CONSTANT_IMAG,
    TEXT_BOX_NUMBER_OF_ITERATIONS,
    TEXT_BOX_RANGE_REAL_MAX,
    TEXT_BOX_RANGE_REAL_MIN,
    TEXT_BOX_RANGE_IMAG_MAX,
    TEXT_BOX_RANGE_IMAG_MIN,
    TEXT_BOX_ZOOM_FACTOR,
    NUMBER_OF_GUI_ELEMENTS
} GUI_Element;


int xwin_init(int w, int h);
void xwin_close();
void fractal_draw(int w, int h, unsigned char *img);
void xwin_poll_events(void);
int menu_react(SDL_Event event);
int is_hovering(SDL_Point mouse_coords);
bool is_focused(void);
void pass_settings(void);
void terminal_redraw(char* buffer, int buffer_size);

#endif

/* end of xwin_sdl.h */
