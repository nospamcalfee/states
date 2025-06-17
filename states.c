/**********************************************************************
 *
 * Filename:    states.c
 *
 * Description: define a table driven state machine mechanism.
 *
 * Notes:       This software should be portable to Posix compatible systems.
 *
 *
 * Copyright (c) 2017 by Steve Calfee.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/
#include "states.h"
/* define commonly required states */
/*
 * state machine function to initiate a delay using the following
 * "function" ptr in table as a delay amount emitted by the SM_START_TIMER macro
 */
int sm_delay_ticks_state(struct state_machine *sm)
{
    if ( sm->stateptrptr ) {
        unsigned int delay;
        sm->stateptrptr += 1; /* point to timer value */
        delay = *(unsigned int *)(sm->stateptrptr);

        SM_START_TIMER(sm, delay); /* no callback, just timer */

        return 1; /* another state will poll for completion or watch timeout */
    }
    return 0; /* try again later */
}
/*
 *  state machine function to poll the timer and wait for it to complete,
 *  before advancing the state machine emitted by the SM_START_TIMER macro
 */
int sm_wait_ticks_state(struct state_machine *sm)
{

    if (SM_IS_TIMER_DONE(sm)) {
        return 1; /* timed out, go to next state. */
    }
    return 0; /* not done, keep polling */
}
/*
    state machine function to
    jump to a new state table (to reduce duplicate table code)
    emitted by the JUMP macro
*/
int sm_jump_table_state(struct state_machine *sm)
{
    sm->stateptrptr += 1; /* point to next ptr */

    sm->stateptrptr = (state_func *)*sm->stateptrptr;

    return 0;   /* and start the new table */
}

/*
 * main state machine execution monitor
 * call with every statemachine every mainloop
 * Will execute ONE state_func per call
 *  returns 0 if last state will be repeated next time
 *  returns >0 if last state completed
 *  returns <0 if last state reported an error, sm is aborted
 */
int sm_run_state(struct state_machine *sm) {
    state_func func_ptr;
    int result = -1;

    if ( sm && sm->stateptrptr && (func_ptr = *(sm->stateptrptr)) )
    {

        /* call the state function right here ...*/
        result = (func_ptr)( sm );
        if (result >= 0) {
            /*
             * success update to next state unless returned zero, so then
             * repeat on next call
             */
            sm->stateptrptr += result;
        } else  { /* ERRORS abort state machine, caller needs to fix */
            sm->stateptrptr = NULL;
            SM_STOP_TIMER(sm);
        }
    }
    return result;
}
