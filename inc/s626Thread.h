/**
 *  Jordan Brindza
 *  brindza@seas.upenn.edu
 *  University of Pennsylvania
 *  December 2010
 */

#ifndef encoderThread_h_DEFINED
#define encoderThread_h_DEFINED

#include "haptics_util.h"
#include "s626_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define N_ENCODERS 6

int s626_thread_init();
void s626_thread_cleanup();

#ifdef __cplusplus
}
#endif

#endif 
