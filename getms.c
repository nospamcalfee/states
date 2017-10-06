/**********************************************************************
 *
 * Filename:    getms.c
 *
 * Description: A Linux millisecond tick fetch routine.
 *
 * Notes: Highly un-portable. All statemachines need a 16 bit, monotomic,
 * incrementing counter that shows as a millisecond value. On linux call the os
 * to get a more-or less accurate instantaneous view of elapsed time.
 *
 *
 * Copyright (c) 2017 by Steve Calfee.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/
#include <time.h>

/*
 * define posix millisecond time function.
 * used to calculate delays needed in run to completion systems.
 */

unsigned int getms()
{
    struct timespec et;

    clock_gettime(CLOCK_MONOTONIC, &et); /* cannot error return */

    return (et.tv_nsec / 1000000) + (et.tv_sec * 1000);

}