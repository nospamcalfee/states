/*!
** \file timer.h
** \brief [brief description]
**
**  [Provide Detailed description here.]
**
** \copyright (c) 2011, Array Converter, Inc.  All Rights Reserved.
*/

#ifndef __TIMER_H__
#define __TIMER_H__

#include "types.h"
#include "system.h" //TMR2

#define TIM_FAST_COUNTER ((volatile int) TMR2)

#define TIM_USEC 40
#define TIM_800_USEC 32000

/*!
** \brief Wait for specified time
**
** This will wait for the specified timeout, doing NOP while waiting
** for the timeout to occur.  Equivalent of a spinlock delay
*/

#define TIM_WAIT(clocks) {int start = TIM_FAST_COUNTER; while (TIM_FAST_COUNTER - start < clocks);}

/*!
** \brief Wait until specified time
**
** Similiar to TIM_WAIT, waits until a specified time
*/

#define TIM_WAIT_UNTIL(time) {while (time - TIM_FAST_COUNTER > 0);}

/*
** \brief Wait for timeout
**
** Waits for a timeout if a condition does not occur
**
** \warning This contains a return in a macro.  Bad Karma
*/

#define TIM_WAIT_FOR(cond, start, tmout, err) \
  {while (!cond) {if (TIM_FAST_COUNTER - start > tmout) return err;}}

/*
** \brief Stop Counter
**
** Resets the counter running flag, so that it does not increment
*/

#define TIM_STOP_COUNTER(ctr) {ctr.isRunning = 0;}

/*
** Prototypes
*/

void TIM_Init(void);
void TIM_StartCounter (timCounterT* ctrP);
bool TIM_IsTimedOut (timCounterT* ctrP, int32 msecTO);
void TIM_Wait (int32 msec);
void TIM_Set(uint32 time);
void TIM_SetSecSinceEpoch(secondsSinceEpochT time);
secondsSinceEpochT TIM_GetSecSinceEpoch(void);
void TIM_ResetSampleCtr (void);
inline int32 TIM_Msec (void);
void TIM_PollCat(void *arg,void (*pc)(void *));
void __attribute__((__deprecated__)) TIM_Set(uint32 time);
uint32 __attribute__((__deprecated__)) TIM_Get();

#endif

