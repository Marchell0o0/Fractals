#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <SDL.h>

#include "event_queue.h"
#include "utils.h"
#include "compute_control.h"
#include "xwin_sdl.h"
#include "gui.h"

#include "gui_win_thread.h"


static void handle_set_computing_params(event *ev) {
    if (!is_computing_module() && !is_computing_locally()) {
        ev->type = EV_SET_COMPUTE;
    } else {
        warn("New computation parameters discarded due to an ongoing computation"); 
    }
}

static void handle_compute_with_module(event *ev) {
    if (!is_computing_module() && !is_computing_locally()) {
        ev->type = EV_COMPUTE; 
    } else { 
        warn("New computation discarded due to an ongoing computation"); 
    }
}

static void handle_draw_animation(event *ev) {
    if (!is_computing_locally() && !is_computing_module()) {
        info("Drawing an animation");
        ev->type = EV_COMPUTE_CPU; 
    } else { 
        warn("New computation discarded due to an ongoing computation"); 
    }
}

static void handle_pause_computation(event *ev) {
    if (is_computing_module() || is_computing_locally()) {
        set_aborted();
        ev->type = EV_ABORT;
        info("Aborting computation"); 
    } else { 
        warn("Abort requested but it is not computing"); 
    }
}

static void handle_reset_computation(event *ev) {
    if (!is_computing_locally() && !is_computing_module()){
        info("Resseting computation parameters");
        ev->type = EV_RESET;
    } else {
        warn("Reset discarded due to an ongoing computation");
    }
}

static void handle_reset_image(event *ev) {
    info("Resseting the image");
    ev->type = EV_RESET_IMAGE;
}

static void handle_request_version(event *ev) {
    ev->type = EV_GET_VERSION;
    info("Requesting version");
}

static void handle_save_image(event *ev) {
    ev->type = EV_SAVE_IMAGE;
    info("Saving the image");
}

static void handle_quit_program(event *ev) {
    set_aborted();
    ev->type = EV_QUIT;
    info("Exiting");
}

void* gui_win_thread(void* d)
{
    event ev;
    SDL_Event sdl_event;

    bool quit = false;

    int pressed_button = NONE;

    gui_init();
    while (!quit)
    {
        ev.type = EV_TYPE_NUM;

        if (SDL_PollEvent(&sdl_event) == 0) { continue; }
        switch(sdl_event.type){
            // If user closes the window
            case SDL_QUIT:
                ev.type = EV_QUIT;
                break;
            // If a key on a keyboard is pressed    
            case SDL_KEYDOWN:
                if(!is_focused()){
                    switch (sdl_event.key.keysym.sym)
                    {
                        case SDLK_q:
                            handle_quit_program(&ev);
                            break;
                        case SDLK_g:
                            handle_request_version(&ev);
                            break;
                        case SDLK_x:
                            handle_draw_animation(&ev);
                            break;
                        case SDLK_a:
                            handle_pause_computation(&ev);
                            break;
                        case SDLK_s:
                            handle_set_computing_params(&ev);
                            break;
                        case SDLK_c:
                            handle_compute_with_module(&ev);
                            break;
                        case SDLK_r:
                            handle_reset_computation(&ev);
                            break;
                        case SDLK_i:
                            handle_reset_image(&ev);
                            break;
                        default:
                            break;
                    }
                    break;
                } else {
                    menu_react(sdl_event);
                }
                break;
            // If user presses a key
            case SDL_MOUSEBUTTONUP:
                pressed_button = menu_react(sdl_event);
                switch (pressed_button){
                    case BUTTON_QUIT_PROGRAM:
                        handle_quit_program(&ev);
                        break;
                    case BUTTON_REQUEST_VERSION:
                        handle_request_version(&ev);
                        break;
                    case BUTTON_DRAW_ANIMATION:
                        handle_draw_animation(&ev);
                        break;
                    case BUTTON_PAUSE_COMPUTATION:
                        handle_pause_computation(&ev);
                        break;
                    case BUTTON_SET_COMPUTING_PARAMS:
                        handle_set_computing_params(&ev);
                        break;
                    case BUTTON_COMPUTE_WITH_MODULE:
                        handle_compute_with_module(&ev);
                        break;
                    case BUTTON_RESET_COMPUTATION:
                        handle_reset_computation(&ev);
                        break;
                    case BUTTON_RESET_IMAGE:
                        handle_reset_image(&ev);
                        break;
                    case BUTTON_SAVE_IMAGE:
                        handle_save_image(&ev);
                    case BUTTON_SAVE_SETTINGS:
                        pass_settings();
                    default:
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                menu_react(sdl_event);
                break;   
            case SDL_MOUSEBUTTONDOWN:
                menu_react(sdl_event);
                break;   
            case SDL_TEXTINPUT:
                menu_react(sdl_event);
                break;  
            default:
                break;
        }
        
        if (ev.type != EV_TYPE_NUM){
            queue_push(ev);
        }
        quit = is_quit();
    }
    gui_cleanup();

    return NULL;
}

