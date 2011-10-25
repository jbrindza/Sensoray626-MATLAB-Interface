/**
 *  Jordan Brindza
 *  brindza@seas.upenn.edu
 *  University of Pennsylvania
 *  December 2010
 */

#include "s626_util.h"
#include "haptics_util.h"

int init_board(HBD board) {      
  printf("\n");
  S626_OpenBoard(board, 0, 0, 0);
  S626_InterruptEnable(board, FALSE);
  //S626_SetErrCallback(board, S626ErrorCallback);
  unsigned long err = S626_GetErrors(board);

  if (err !=0x0) {
    printf("Board open/installation with Err = 0x%x : ", board);

    switch (err) {
      case ERR_OPEN:
        printf("\t\t Can't open driver.\n"); 
        break;
      case ERR_CARDREG:
        printf("\t\t Can't attach driver to board.\n"); 
        break;
      case ERR_ALLOC_MEMORY:
        printf("\t\t Memory allocation error.\n"); 
        break;
      case ERR_LOCK_BUFFER:
        printf("\t\t Error locking DMA buffer.\n"); 
        break;
      case ERR_THREAD:
        printf("\t\t Error starting a thread.\n"); 
        break;
      case ERR_INTERRUPT:
        printf("\t\t Error enabling interrupt.\n"); 
        break;
      case ERR_LOST_IRQ:
        printf("\t\t Missed interrupt.\n"); 
        break;
      case ERR_INIT:
        printf("\t\t Board object not instantiated.\n"); 
        break;
      case ERR_SUBIDS:
        printf("\t\t PCI SubDevice/SubVendor ID mismatch.\n"); 
        break;
      case ERR_CFGDUMP:
        printf("\t\t PCI configuration dump failed.\n"); 
        break;
      default:
        printf("\t\t other Unknown errors.\n"); 
        break;
    }

    printf("Fix board open/installation error, and then try again. \n\n");
    return -1;
  }

  printf ("Board(%d) on PCI bus %d; slot %d\n\n", board, 
                                                  S626_GetAddress(board) >> 16, 
                                                  S626_GetAddress(board) & 0xffff);
  return board;
}

void close_board(HBD board) {
  //close 626 board
  S626_CloseBoard(board);  
}



void create_quadrature_encoder_counter(HBD board, WORD counter) {
  // Set operating mode for the given counter.
  S626_CounterModeSet(board, counter,
      ( LOADSRC_INDX << BF_LOADSRC ) |    // Preload upon index.
      //( INDXSRC_HARD << BF_INDXSRC ) |    // Enable hardware index.
      ( INDXSRC_SOFT << BF_INDXSRC ) |    // Enable hardware index.
      ( INDXPOL_POS << BF_INDXPOL ) |     // Active high index.
      ( CLKSRC_COUNTER << BF_CLKSRC ) |   // Operating mode is counter.
      ( CLKPOL_POS  << BF_CLKPOL ) |      // Active high clock.
      ( CLKMULT_4X   << BF_CLKMULT ) |    // Clock multiplier is 4x.
      ( CLKENAB_INDEX << BF_CLKENAB ) );  // Counting is initially disabled.
  

	S626_CounterPreload(board, counter, ENCODER_ZERO);	

  // Enable latching of accumulated counts on demand.
  S626_CounterLatchSourceSet(board, counter, LATCHSRC_AB_READ);

	// Reset Counter
  zero_encoder_counter(board, counter);

  // Enable the counter.
  S626_CounterEnableSet(board, counter, CLKENAB_ALWAYS);
}

void zero_encoder_counter(HBD board, WORD counter) {
  S626_CounterSoftIndex(board, counter);
}

void set_dac_voltage(HBD board, WORD channel, double volts) {
  // make sure input is in valid range
  volts = MIN(10.0, MAX(-10.0, volts));

  // set DAC
  S626_WriteDAC(board, channel, (LONG)(volts * DAC_VSCALAR));
}

