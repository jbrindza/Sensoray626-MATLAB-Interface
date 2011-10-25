/**
 *  Jordan Brindza
 *  brindza@seas.upenn.edu
 *  University of Pennsylvania
 *  December 2010
 */

#include "s626_util.h"
#include "haptics_util.h"

#include "s626drv.h"
#include "App626.h"
#include "s626mod.h"
#include "s626core.h"
#include "s626api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for strlen, memcpy
#include <sys/time.h>
#include <unistd.h>

#include <pthread.h>

void quadrature_test(HBD board, WORD counter) {
  time_t time_now;

  // Reset Counter
  S626_CounterCapFlagsReset(board, counter);

  // Configure counter xA/xB as a encoder counter.
  create_quadrature_encoder_counter(board, counter);

  while (1) {
    printf("\tCurrent position/counting of Counter #%i = %d ", counter, S626_CounterReadLatch(board, counter) - 8388608);
    time(&time_now);
    printf("at %s", ctime(&time_now));
    sleep(1);
  }

}

int main() {
  HBD board = 0;
  WORD counter = CNTR_0B; 

  // initialize board
  init_board(board);
  
  // create counter
  quadrature_test(board, counter);   

  // close board
  close_board(board);

  return 0;
}
