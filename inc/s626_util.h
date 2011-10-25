#ifndef __S626_UTIL_H__
#define __S626_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "haptics_util.h"

// encoder zero at 2^(24-1)
#define ENCODER_ZERO 8388608 

// scalar multipliet to convert volts to binary
//   for use with DAC
#define DAC_VSCALAR 819.1

#ifdef __cplusplus
extern "C" {
#endif

#include "s626drv.h"
#include "App626.h"
#include "s626mod.h"
#include "s626core.h"
#include "s626api.h"


int init_board(HBD board); 

void close_board(HBD board);


// Configure a counter xA/xB for Encoder Counter.
void create_quadrature_encoder_counter(HBD board, WORD counter);

// reset/zero a counter
void zero_encoder_counter(HBD board, WORD counter);

//  Quadrature Encoder test:  test real Quadrature Encoder  
//  Sets up a Counter xA/xB as a counter for quadrature encoder test.
    // CNTR_0A: inputs on J5: CNTR_0A - A-,A+,B-,B+,I-,I+
    // CNTR_1A: inputs on J5: CNTR_1A - A-,A+,B-,B+,I-,I+
    // CNTR_2A: inputs on J5: CNTR_2A - A-,A+,B-,B+,I-,I+ 
    // CNTR_0B: inputs on J4: CNTR_0B - A-,A+,B-,B+,I-,I+ 
    // CNTR_1B: inputs on J4: CNTR_1B - A-,A+,B-,B+,I-,I+
    // CNTR_2B: inputs on J4: CNTR_2B - A-,A+,B-,B+,I-,I+ 
void quadrature_test(HBD board, WORD counter);

// Ramp a selected DAC channel of board 0.
// output a waveform that will continuosly ramp from minimum
// to maximum output voltage.
void set_dac_voltage(HBD board, WORD channel, double volts);

#ifdef __cplusplus
}
#endif

#endif

