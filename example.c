/**********************************************************************
 *
 * Filename:    example.c
 *
 * Description: A Linux Two statemachine example program.
 *
 * Notes: Should be portable to any ARCH running Linux.
 *
 *
 * Copyright (c) 2017 by Steve Calfee.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

/*
 * compile: " gcc -Wall -Wextra -g -o example example.c states.c getms.c "
 * test: " gdb ./example "
 */
#include "states.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <time.h>
#include <termios.h>

/*
 * define structure containing 2 statemachines for input and output
 */
static struct fab_main_state {
    struct state_machine in;
    struct state_machine out;
    char data[512];
    unsigned int head;
    unsigned int tail;
} fab_main_state;

/*
 * example where state is followed by jump on timeout,
 * repeats if no timeout and no key
 * and continues when a key is ready
 */
static int input_available_state(struct state_machine *sm)
{
    struct timeval tv;
    fd_set fds;

    if (SM_IS_TIMER_DONE(sm)) {
        return SM_RETURN_DONE; /* timed out, tell table interp */
    }

    tv.tv_sec = 0;
    tv.tv_usec = 100;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    if (FD_ISSET(STDIN_FILENO, &fds)) {
        return SM_RETURN_SKIP_JUMP; /* a key is ready, continue */
    }
    return SM_RETURN_REPEAT; /* not done, keep polling */
}

static int read_key_state(struct state_machine *sm)
{
    struct fab_main_state *pt = cast_p_to_outer(
            struct state_machine *, sm, struct fab_main_state, in);
    unsigned int in_byte, next;

    in_byte = getchar();

    /* push byte into buffer for other "thread" */
    next = pt->head + 1;
    if (next >= sizeof(pt->data))
        next = 0;
    if (next == pt->tail)
        return SM_RETURN_DONE; //error return, buffer is full

    pt->data[pt->head] = in_byte;
    pt->head = next;

    return SM_RETURN_SKIP_JUMP; //everything is good, continue
}

static int print_key_state(struct state_machine *sm)
{
    struct fab_main_state *pt = cast_p_to_outer(
            struct state_machine *, sm, struct fab_main_state, out);
    unsigned int out_byte, next;

    if (SM_IS_TIMER_DONE(sm)) {
        printf("hey give me keys\n");
        return SM_RETURN_DONE; /* timed out, tell table interp */
    }
    if (pt->head == pt->tail)
        return SM_RETURN_REPEAT; //empty, call again later
    /* next is where tail will point to after this read. */
    next = pt->tail + 1;
    if (next >= sizeof(pt->data))
        next = 0;

    out_byte = pt->data[pt->tail];
    pt->tail = next;

    putchar(out_byte);

    return SM_RETURN_DONE; //everything is good, continue
}

/*
 * example state machine sequence tables
 */
state_func get_key_table[] = {
    SM_SET_TIMER_MS(3000),      /* fast timeout for testing 5 second */
    input_available_state,
    SM_JUMP(get_key_table),     /* timeout, nag or just retry */
    read_key_state,             /* will choose one of the following jumps */
    SM_JUMP(get_key_table),     /* error buffer is full return */
    SM_JUMP(get_key_table),     /* all ok, just get more input */
};
state_func display_key_table[] = {
    SM_SET_TIMER_MS(6000),      /* fast timeout for testing 5 second */
    print_key_state,             /* will choose one of the following jumps */
    SM_JUMP(display_key_table),     /* error buffer is full return */
};

/*
 * example run to completion using state machines.
 * One task will collect user input or nag when not available in time.
 * Other task will slowly send out user messages.
 */

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    int ret;

    fab_main_state.head = 0;
    fab_main_state.tail = 0;
    fab_main_state.in.stateptrptr = NULL;
    fab_main_state.out.stateptrptr = NULL;

    struct termios ctrl;
    tcgetattr(STDIN_FILENO, &ctrl);
    ctrl.c_lflag &= ~(ECHO | ICANON);
    /* turning off canonical mode makes input unbuffered */
    tcsetattr(STDIN_FILENO, TCSANOW, &ctrl);

    printf("type keys!\n");

    while (1) {

        if (!fab_main_state.in.stateptrptr)
        {
            /* restart if machine exited */
            fab_main_state.in.stateptrptr = (void *)get_key_table;
        }
        do {
            /* run until jump, delay or complete */
            ret = sm_run_state(&fab_main_state.in);
        } while (ret > 0 && fab_main_state.in.stateptrptr);

        if (!fab_main_state.out.stateptrptr)
        {
            /* restart if machine exited */
            fab_main_state.out.stateptrptr = (void *)display_key_table;
        }
        do {
            /* run until jump, delay or complete */
            ret = sm_run_state(&fab_main_state.out);
        } while (ret > 0 && fab_main_state.out.stateptrptr);

        nanosleep((struct timespec[]){{0, 1000000}},NULL); /* sleep 1 ms */
    }
}