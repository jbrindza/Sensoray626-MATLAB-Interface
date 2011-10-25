/**
 *  Jordan Brindza
 *  brindza@seas.upenn.edu
 *  University of Pennsylvania
 *  December 2010
 */

#include <map>
#include <string>
#include <vector>
#include <pthread.h>
#include <signal.h>
#include <math.h>

#include "s626Thread.h"
#include "s626Functions.h"
#include "haptics_util.h"
#include "s626_util.h"
#include "s626api.h"
#include "timeScalar.h"

#include "mex.h"

#define DEFAULT_ENC_PER_REV 2000
#define DEFAULT_VEL_FILTER_WEIGHT 0.06
#define DEFAULT_ACCEL_FILTER_WEIGHT 0.06

typedef unsigned char uint8;
typedef unsigned int uint32;

static pthread_t s626_thread = 0;
pthread_mutex_t s626_mutex = PTHREAD_MUTEX_INITIALIZER;

HBD board = 0;

mxArray *fps;
mxArray *velFilterWeight;
mxArray *accelFilterWeight;
mxArray *encoders;
mxArray *encTicksPerRev;

/**
 * This thread is designed to run at approximately 1kHz
 * It is not running on a real time os so there is no garuentee that
 * it will actually run at 1kHz
 */
void *s626_thread_func(void *id) {
  int ret;

  printf("Starting s626 thread...\n");
  sigset_t sigs;
  sigfillset(&sigs);
  pthread_sigmask(SIG_BLOCK, &sigs, NULL);

  static double *encodersPtr = mxGetPr(encoders);
  static double *countsPtr = encodersPtr + N_ENCODERS*0; 
  static double *positionsPtr = encodersPtr + N_ENCODERS*1;  
  static double *velocitiesPtr = encodersPtr + N_ENCODERS*2;  
  static double *accelerationsPtr = encodersPtr + N_ENCODERS*3; 

  static double *fpsPtr = mxGetPr(fps);
  static double *velFilterWeightPtr = mxGetPr(velFilterWeight);
  static double *accelFilterWeightPtr = mxGetPr(accelFilterWeight);
  static double *encTicksPerRevPtr = mxGetPr(encTicksPerRev);

  static double prevPositions[N_ENCODERS];
  static double prevVelocities[N_ENCODERS];
  static double prevAcclerations[N_ENCODERS];

  static double t0 = time_scalar();
  static double dt;
  static double tupdate = time_scalar();
  static int nupdate = 0;

  while (1) {
    pthread_mutex_lock(&s626_mutex);

    // time since last update
    dt = time_scalar() - t0;

    // get encoder counts
    // TODO: make this work with N_ENCODERS
    *(countsPtr + 0) = (double)S626_CounterReadLatch(board, CNTR_0A) - ENCODER_ZERO;
    *(countsPtr + 1) = (double)S626_CounterReadLatch(board, CNTR_1A) - ENCODER_ZERO;
    *(countsPtr + 2) = (double)S626_CounterReadLatch(board, CNTR_2A) - ENCODER_ZERO;
    *(countsPtr + 3) = (double)S626_CounterReadLatch(board, CNTR_0B) - ENCODER_ZERO;
    *(countsPtr + 4) = (double)S626_CounterReadLatch(board, CNTR_1B) - ENCODER_ZERO;
    *(countsPtr + 5) = (double)S626_CounterReadLatch(board, CNTR_2B) - ENCODER_ZERO;

    // update time
    t0 = time_scalar();

    // not sure if matlab optimization will unroll loops or not...
    for (int i = 0; i < N_ENCODERS; i++) {
      // positions in radians
      *(prevPositions + i) = *(positionsPtr + i);
      *(positionsPtr + i) = *(countsPtr + i) * 2 * M_PI / *(encTicksPerRevPtr + i);
      // velocity estimate
      *(prevVelocities + i) = *(velocitiesPtr + i);
      *(velocitiesPtr + i) = IIR_FILTER( (*(positionsPtr + i) - *(prevPositions + i)) / dt,
                                          *(velocitiesPtr + i), *(velFilterWeightPtr + i));
      // accleration estimate
      *(accelerationsPtr + i) = IIR_FILTER( (*(velocitiesPtr + i) - *(prevVelocities + i)) / dt,
                                              *(accelerationsPtr + i), *(accelFilterWeightPtr + i));
    }
      
    /*
    // positions in radians
    *(prevPositions + 0) = *(positionsPtr + 0);
    *(prevPositions + 1) = *(positionsPtr + 1);
    *(prevPositions + 2) = *(positionsPtr + 2);
    *(prevPositions + 3) = *(positionsPtr + 3);
    *(prevPositions + 4) = *(positionsPtr + 4);
    *(prevPositions + 5) = *(positionsPtr + 5);
    *(positionsPtr + 0) = *(countsPtr + 0) * 2 * M_PI / *(encTicksPerRevPtr + 0);
    *(positionsPtr + 1) = *(countsPtr + 1) * 2 * M_PI / *(encTicksPerRevPtr + 1);
    *(positionsPtr + 2) = *(countsPtr + 2) * 2 * M_PI / *(encTicksPerRevPtr + 2);
    *(positionsPtr + 3) = *(countsPtr + 3) * 2 * M_PI / *(encTicksPerRevPtr + 3);
    *(positionsPtr + 4) = *(countsPtr + 4) * 2 * M_PI / *(encTicksPerRevPtr + 4);
    *(positionsPtr + 5) = *(countsPtr + 5) * 2 * M_PI / *(encTicksPerRevPtr + 5);

    // velocity estimate
    *(prevVelocities + 0) = *(velocitiesPtr + 0);
    *(prevVelocities + 1) = *(velocitiesPtr + 1);
    *(prevVelocities + 2) = *(velocitiesPtr + 2);
    *(prevVelocities + 3) = *(velocitiesPtr + 3);
    *(prevVelocities + 4) = *(velocitiesPtr + 4);
    *(prevVelocities + 5) = *(velocitiesPtr + 5);
    *(velocitiesPtr + 0) = IIR_FILTER( (*(positionsPtr + 0) - *(prevPositions + 0)) / dt,
                                        *(velocitiesPtr + 0), *velFilterWeightPtr);
    *(velocitiesPtr + 1) = IIR_FILTER( (*(positionsPtr + 1) - *(prevPositions + 1)) / dt,
                                        *(velocitiesPtr + 1), *velFilterWeightPtr);
    *(velocitiesPtr + 2) = IIR_FILTER( (*(positionsPtr + 2) - *(prevPositions + 2)) / dt,
                                        *(velocitiesPtr + 2), *velFilterWeightPtr);
    *(velocitiesPtr + 3) = IIR_FILTER( (*(positionsPtr + 3) - *(prevPositions + 3)) / dt,
                                        *(velocitiesPtr + 3), *velFilterWeightPtr);
    *(velocitiesPtr + 4) = IIR_FILTER( (*(positionsPtr + 4) - *(prevPositions + 4)) / dt,
                                        *(velocitiesPtr + 4), *velFilterWeightPtr);
    *(velocitiesPtr + 5) = IIR_FILTER( (*(positionsPtr + 5) - *(prevPositions + 5)) / dt,
                                        *(velocitiesPtr + 5), *velFilterWeightPtr);

    // accleration estimate
    *(accelerationsPtr + 0) = IIR_FILTER( (*(velocitiesPtr + 0) - *(prevVelocities + 0)) / dt,
                                            *(accelerationsPtr + 0), *accelFilterWeightPtr);
    *(accelerationsPtr + 1) = IIR_FILTER( (*(velocitiesPtr + 1) - *(prevVelocities + 1)) / dt,
                                            *(accelerationsPtr + 1), *accelFilterWeightPtr);
    *(accelerationsPtr + 2) = IIR_FILTER( (*(velocitiesPtr + 2) - *(prevVelocities + 2)) / dt,
                                            *(accelerationsPtr + 2), *accelFilterWeightPtr);
    *(accelerationsPtr + 3) = IIR_FILTER( (*(velocitiesPtr + 3) - *(prevVelocities + 3)) / dt,
                                            *(accelerationsPtr + 3), *accelFilterWeightPtr);
    *(accelerationsPtr + 4) = IIR_FILTER( (*(velocitiesPtr + 4) - *(prevVelocities + 4)) / dt,
                                            *(accelerationsPtr + 4), *accelFilterWeightPtr);
    *(accelerationsPtr + 5) = IIR_FILTER( (*(velocitiesPtr + 5) - *(prevVelocities + 5)) / dt,
                                            *(accelerationsPtr + 5), *accelFilterWeightPtr);
    */


    pthread_mutex_unlock(&s626_mutex);

    // check if the thread has been canceled
    pthread_testcancel();

    // update fps estimate
    if (nupdate % 1000 == 0) {
      *fpsPtr = 1000 / (time_scalar() - tupdate);
      tupdate = time_scalar();
    }

    nupdate++;

    // run thread at 1 kHz
    usleep(1000 - (time_scalar() - t0));
  }
}

int s626_thread_init() {
  int ret = 0;

  printf("Initializing Sensoray626 Board %d...", board);
  fflush(stdout);
  if ((ret = init_board(board)) < 0) {
    printf("Couldn't open Sensory626 Board %d connection.", board);
    return ret;
  }
  printf("done\n");
  
  printf("Initializing quadrature encoder counters...");
  fflush(stdout);
  create_quadrature_encoder_counter(board, CNTR_0A); 
  create_quadrature_encoder_counter(board, CNTR_1A); 
  create_quadrature_encoder_counter(board, CNTR_2A); 
  create_quadrature_encoder_counter(board, CNTR_0B); 
  create_quadrature_encoder_counter(board, CNTR_1B); 
  create_quadrature_encoder_counter(board, CNTR_2B); 
  printf("done\n");

  // create encoder mxarray to store encoder values
  encoders = mxCreateDoubleMatrix(N_ENCODERS, 4, mxREAL);
  mexMakeArrayPersistent(encoders);

  // create encoder ticks, and filter weight mxarrays
  encTicksPerRev = mxCreateDoubleMatrix(N_ENCODERS, 1, mxREAL);
  velFilterWeight = mxCreateDoubleMatrix(N_ENCODERS, 1, mxREAL);
  accelFilterWeight = mxCreateDoubleMatrix(N_ENCODERS, 1, mxREAL);

  double *encTicksPerRevPtr = mxGetPr(encTicksPerRev);
  double *velFilterWeightPtr = mxGetPr(velFilterWeight);
  double *accelFilterWeightPtr = mxGetPr(accelFilterWeight);
  for (int i = 0; i < N_ENCODERS; i++) {
    *(encTicksPerRevPtr + i) = DEFAULT_ENC_PER_REV; 
    *(velFilterWeightPtr + i) = DEFAULT_VEL_FILTER_WEIGHT;
    *(accelFilterWeightPtr + i) = DEFAULT_ACCEL_FILTER_WEIGHT;
  }

  mexMakeArrayPersistent(encTicksPerRev);
  mexMakeArrayPersistent(velFilterWeight);
  mexMakeArrayPersistent(accelFilterWeight);

  // create fps estimate scalar
  fps = mxCreateDoubleScalar(0.0);
  mexMakeArrayPersistent(fps);

  printf("Starting Thread...\n");
  ret = pthread_create(&s626_thread, NULL, s626_thread_func, NULL);
  
  return ret;
}

void s626_thread_cleanup() {
  // cancel thread
  if (s626_thread) {
    printf("Canceling thread...\n");
    pthread_cancel(s626_thread);
    usleep(500000L);
  }
  
  // close board connection
  printf("Closing s626 board connection...\n");
  close_board(board);
}

