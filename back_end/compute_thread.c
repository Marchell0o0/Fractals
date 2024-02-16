
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include "utils.h"
#include "messages.h"
#include "event_queue.h"
#include "prg_io_nonblock.h"
#include "compute.h"

#include "compute_thread.h"

#ifndef STARTUP_MSG 
#define STARTUP_MSG "HORPYMAR"
#endif

#define VERSION_MAJOR 1
#define VERSION_MINOR 5
#define VERSION_PATCH 2

static void process_pipe_message(event * const ev, message *msg);

void* compute_thread(void* d)
{
    my_assert(d != NULL, __func__, __LINE__, __FILE__);
    int pipe_out = *(int*)d;

    uint8_t msg_buf[sizeof(message)];
    int msg_len;
    message msg = { .type = MSG_STARTUP };

    const char* str = STARTUP_MSG;
    for (int i = 0; i < STARTUP_MSG_LEN; i++) {
        msg.data.startup.message[i] = str[i];
    }
 
    // int ret = 
    send_message(&msg, msg_buf, &msg_len, pipe_out);
    // fprintf(stderr, "DEBUG: Sending startup %s\n", (ret == 1 ? "OK" : "FAIL"));

    bool quit = false;
    do {
        event ev = queue_pop();
        msg.type = MSG_NBR;
        if (ev.type == EV_PIPE_IN_MESSAGE){
            process_pipe_message(&ev, &msg);
        }

        if (msg.type == MSG_COMPUTE_DATA){
            while (compute_chunk(&msg)) {
                // ret = 
                send_message(&msg, msg_buf, &msg_len, pipe_out);
                // fprintf(stderr, "DEBUG: Sending one dot %s\n", (ret == 1 ? "OK" : "FAIL"));
            }
            msg.type = MSG_DONE;
        }
        if (msg.type != MSG_NBR) {
            // int ret = 
            send_message(&msg, msg_buf, &msg_len, pipe_out);
            // fprintf(stderr, "DEBUG: Sending message %s\n", (ret == 1 ? "OK" : "FAIL"));
        }

        quit = is_quit();
    } while (!quit);

    return NULL;
}


void process_pipe_message(event * const ev, message *msg_to_send)
{
    my_assert(ev != NULL && ev->type == EV_PIPE_IN_MESSAGE && ev->data.msg, __func__, __LINE__, __FILE__);
    ev->type = EV_TYPE_NUM;

    const message *msg_received = ev->data.msg;

    switch (msg_received->type){
        case MSG_QUIT:
            info("Received quit, exiting..");
            set_quit();
            break;
        case MSG_ABORT:
            info("Aborintg computation");
            break;
        case MSG_GET_VERSION:
            msg_to_send->type = MSG_VERSION;
            msg_to_send->data.version.major = VERSION_MAJOR;
            msg_to_send->data.version.minor = VERSION_MINOR;
            msg_to_send->data.version.patch = VERSION_PATCH;
            break;
        case MSG_SET_COMPUTE:
            set_compute(msg_received);
            info("Set compute");
            msg_to_send->type = MSG_OK;
            break;
        case MSG_COMPUTE:
            set_chunk(msg_received);
            fprintf(stderr, "INFO: Received chunk id %d\n", msg_received->data.compute.cid);
            msg_to_send->type = MSG_COMPUTE_DATA;
            break;
        // case MSG_COMPUTE_BURST:
        //     set_chunk(msg_received);
        //     compute_chunk_burst(msg_to_send);
        //     break;
        default:
            fprintf(stderr, "WARNING: Unhandled message type %d\n", msg_received->type);
            break;
    } // end switch
    free(ev->data.msg);
    ev->data.msg = NULL;
}