/**********************************************************************
 *
 * Filename:    states.h
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

/*
 * Prototype for state machine functions.
 * Each state returns >=0 for success,
 * or a <0 error code if statemachine should be aborted.
 *
 * positive status is the amount to add to the state table, almost always 0
 * to repeat the current state, or 1 to go to the next state. Other values
 * are risky... See JMP macro.
 *
 * This software is designed to be used in a run-to-completion single
 * task/thread scheme. Never wait on something, set the state machine to check
 * for completion of an event.
 *
 */
#ifndef __STATES_H__
#define __STATES_H__
#include <stddef.h>
#include <stdint.h>
/*
 * macros for controlling the statemachine convert to ticks from ms for
 * oneshot. used to build state machine tables.
 */
/* convert to ticks from ms for oneshot.
 * Used to build state machine tables.
*/
/* defines ticks per second */
#ifndef SM_TICK_RATE
#define SM_TICK_RATE 1000L
#endif

/* round up, ie, 1 tick is the smallest delay */
#define SM_MS_TO_TICKS(ms) ((((ms)*SM_TICK_RATE)+0000)/(1000L))
#define SM_DELAY_MS(ms) (sm_delay_ticks_state), \
                        (state_func)(SM_MS_TO_TICKS(ms)), \
                        (sm_wait_ticks_state)
/* loads the timer, but does not wait for it */
#define SM_SET_TIMER_MS(ms)     (sm_delay_ticks_state), \
                                (state_func)(SM_MS_TO_TICKS(ms))

#define SM_JUMP(dest) (sm_jump_table_state), (state_func)(dest)
/* if a state wants to skip the next 2 jumps, use this to return */
#define SM_RETURN_SKIP_TWO_JUMPS 5
/* if a state wants to skip the next jump, use this to return */
#define SM_RETURN_SKIP_JUMP 3
/* to get really tricky you can skip additional jumps with this */
#define SM_RETURN_SKIP_JUMP_SIZE 2
/* if a state wants to skip the next state, use this to return */
#define SM_RETURN_SKIP_NEXT 2
/* if a state wants to be called again */
#define SM_RETURN_REPEAT 0
/* if a state is done, to proceed to the next state */
#define SM_RETURN_DONE 1
/* if an error was detected, exit state machine with error */
#define SM_RETURN_ERROR -1

uint32_t getms(); /* system specific function to read timer */
#define READ_GLOBAL_TICKS getms()

#define SM_START_TIMER(sm, delay_) \
    do {    (sm)->start_timer=READ_GLOBAL_TICKS; \
            (sm)->delay=(delay_); \
    } while (0)

#define SM_STOP_TIMER(x)

/* do an unsigned modulo 16 bit subtract to get duration, compare to delay */
#define SM_IS_TIMER_DONE(sm) ((uint16_t)( \
    ((uint16_t)READ_GLOBAL_TICKS) - (sm)->start_timer) >= (sm)->delay)
#define SM_SET_TABLE(sm, table) ((sm)->stateptrptr = table)
/**
* container_of - cast a member of a structure out to the containing structure
* From linux - uses two gcc extensions (not portable to vc8)
*
* @ptr:the pointer to the member.
* @type:the type of the container struct this is embedded in.
* @member:the name of the member within the struct.
*
*/
#define container_of(ptr, type, member) ({\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);\
        (type *)( (char *)__mptr - offsetof(type,member) );})

/**
 * cast_p_to_outer - cast a pointer to an outer, containing struct
 * @ptype:The type of pointer (ie member type)
 * @ptr:The pointer to the member
 * @mtype:The type of the outer, container
 * @mname:The name of the member in outer struct (ptr is pointing at it)
 *
 * This uses a portable (no GNU extension) method of casting
 * All types are checked. Trust the C optimizer
 */
#define cast_p_to_outer(ptype, ptr, mtype, mname) (\
    ((ptype) ptr == ptr) ? \
    (mtype *)( (char *)ptr - offsetof(mtype, mname) ) \
    : NULL \
)
/**
 * elements_of - determine number of elements in an array
 * @array_name
 * returns number of elements
 */
#define elements_of(a) (sizeof(a)/sizeof(a[0]))

struct state_machine;
typedef int (*state_func)(struct state_machine *sm);

struct state_machine {
    state_func  *stateptrptr;   /* ptr to a table of state_func pointers */
    uint16_t    start_timer;    /* save timer when started */
    uint16_t    delay;          /* set number of ticks to wait from now */
};

/*
 * function prototypes (of state_func) for built-in state control
 */
int sm_wait_ticks_state(struct state_machine *sm);
int sm_delay_ticks_state(struct state_machine *sm);
int sm_jump_table_state(struct state_machine *sm);

/*  main state machine execution monitor
    call with every statemachine every mainloop

  returns the state status of the one state executed.
  So if returns 0, it means the last state wants to be repeated,
  if it returned >=1, the last state completed.
  if it returns <0 but > -256 it is a state reported error condition
  if it returns <0 but < -257 (NULL_STATE_PTR_ERROR)
        ( means run_state does not have ptrs set correctly, yet)

  This status should be used sparingly if at all. It means a higher level is
  watching the state transitions, maybe the states are not factored correctly?

*/
#define NULL_STATE_PTR_ERROR -257
int sm_run_state(struct state_machine *sm);

#endif
