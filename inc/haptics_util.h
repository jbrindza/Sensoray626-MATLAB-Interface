/**
 *  Jordan Brindza
 *  brindza@seas.upenn.edu
 *  University of Pennsylvania
 *  December 2010
 */

#ifndef __HAPTIC_UTIL_H__
#define __HAPTIC_UTIL_H__

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX(x,y) (x < y ? y : x)
#define MIN(x,y) (x < y ? x : y)
#define SIGN(x) (x < 0 ? -1 : 1)

#define IIR_FILTER(curr, prev, weight) (weight * curr + (1 - weight) * prev)

#ifdef __cplusplus
extern "C" {
#endif

// Take a 16-bit value and print it in binary (msb --> lsb)
void PrintHexAsBin(int data);

// Return number of milliseconds elapsed since midnight? 
//                 since the Epoch (00:00:00 UTC, January 1, 1970) ?
unsigned long get_time_now();

// Return elapsed time in milliseconds.
unsigned long get_time_elapsed(unsigned long start_time);

#ifdef __cplusplus
}
#endif

#endif

