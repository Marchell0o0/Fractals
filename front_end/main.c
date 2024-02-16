#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "utils.h"
#include "event_queue.h"
#include "messages.h"
#include "compute_control.h"
#include "gui.h"


#include "main.h"

static void process_pipe_message(event * const ev);

void* main_thread(void* d)
{
    my_assert(d != NULL, __func__, __LINE__, __FILE__);
    int pipe_out = *(int*)d;

    message msg;
    uint8_t msg_buf[sizeof(message)];
    int msg_len;
    bool quit = false;

    computation_init();
    do {
        event ev = queue_pop();
        msg.type = MSG_NBR;
        switch(ev.type){
            case EV_QUIT:
                debug("Quit received");
                msg.type = MSG_QUIT;
                set_quit();
                break;
            case EV_GET_VERSION:
                msg.type = MSG_GET_VERSION;
                break;
            case EV_SET_COMPUTE:
                set_compute(&msg);
                break;
            case EV_COMPUTE:
                compute_module(&msg);
                break;
            case EV_COMPUTE_CPU:
                draw_animation(&msg);
                break;
            case EV_ABORT:
                if (is_computing_module()) {
                    msg.type = MSG_ABORT;
                }
                break;
            case EV_SAVE_IMAGE:
                gui_save_image();
                break;
            case EV_RESET:
                reset_computation();
                break;
            case EV_RESET_IMAGE:
                clean_image();
                gui_refresh();
                break;
            case EV_PIPE_IN_MESSAGE:
                process_pipe_message(&ev);
                break;
            default:
                break;
        } 

        if (msg.type != MSG_NBR){
            // int ret = 
            send_message(&msg, msg_buf, &msg_len, pipe_out);
            // fprintf(stderr, "DEBUG: Sending message %s\n", (ret == 1 ? "OK" : "FAIL")); 
        }
        quit = is_quit();
    } while (!quit);
    computation_cleanup();

    return NULL;
}

void process_pipe_message(event * const ev)
{
    my_assert(ev != NULL && ev->type == EV_PIPE_IN_MESSAGE && ev->data.msg, __func__, __LINE__, __FILE__);
    ev->type = EV_TYPE_NUM;
    const message *msg = ev->data.msg;
    switch (msg->type){
        case MSG_OK:
            info("Received OK from comp module");
            break;
        case MSG_STARTUP:
            fprintf(stderr, "INFO: Comp module has started with this message: %s\n", msg->data.startup.message);
            break;
        case MSG_VERSION:
            fprintf(stderr, "INFO: Module version %d.%d.%d\n", msg->data.version.major, msg->data.version.minor, msg->data.version.patch);
            break;
        case MSG_COMPUTE_DATA:
            update_data(&(msg->data.compute_data));
            break;
        case MSG_DONE:
            gui_refresh();

            if (is_saving()) { gui_save_image(); }

            if (is_aborted()){
                set_not_aborted();
                info("Received last package");
            } else if (is_done()){
                info("Computation is done");
            } else if (is_computing_module()){
                event ev = { .type = EV_COMPUTE};
                queue_push(ev);
            }
            break;
        case MSG_ERROR:
            error("Received error from comp module");
            break;
        
        default:
            fprintf(stderr, "WARNING: Unhandled message type %d\n", msg->type);
            break;
    }
    free(ev->data.msg);
    ev->data.msg = NULL;
}