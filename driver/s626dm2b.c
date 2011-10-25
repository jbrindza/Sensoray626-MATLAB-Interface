//************************************ 626demo.c ************************************
//* This is a sample application program that shows you how to use multi-626 boards,
//* with loadable Linux device driver 's626drv' and the API functions of 626 core&mod
//* driver library 'libs626.a' to develop your own application(s) for any projects.
//* The program is only used for the Sensoray Model 626 under Linux.
//*
//* Copyright (c) 2006.  Sensoray Co., Inc
//* Author:              Charlie X. Liu
//* Revision:
//*         Feb., 2004   Charlie X. Liu	 Initial, ported from Version 2.0 
//*                                      (C++ based driver&demo package,
//*                                      written by David Stroupe & Charlie Liu).
//*         Feb., 2004   Charlie X. Liu	 Added demo for measuring ADC throughput.
//*         Apr., 2004   Charlie X. Liu  Added demo for testing throughput of
//*                                      digital output, digital input, and DAC
//*         Apr., 2004   Charlie X. Liu  Added demo for scattered AD acquisition,
//*                                      DIO interrupts, toggling relay on board,
//*         Apr., 2004   Charlie X. Liu  Added demo for quadrature encoder tests.
//*         Dec., 2006   Charlie X. Liu  Added demo to show using multi-626 boards.
//*
//*---------------------------------------------------------------------------------
//* This program is free software; you can redistribute it and/or
//* modify it under the terms of the GNU General Public License
//* as published by the Free Software Foundation; either version 2
//* of the License, or (at your option) any later version.

//* This program is distributed in the hope that it will be useful,
//* but WITHOUT ANY WARRANTY; without even the implied warranty of
//* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//* GNU General Public License for more details.
//*
//* You should have received a copy of the GNU General Public License
//* along with this program; if not, write to the Free Software
//* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
//* *******************************************************************************/

#include "s626drv.h"
#include "App626.h"
#include "s626mod.h"
#include "s626core.h"
#include "s626api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/time.h>
#include <pthread.h>

//*-------------------------------------------------------
//* Function prototypes (forwarding)
//*-------------------------------------------------------

void				MainMenu (void);
void				InterruptAppISR0( DWORD board );
void				InterruptAppISR1( DWORD board );
void				Err (DWORD ecode);
void				HexBin (int data);
void				IntTest( DWORD board );
void				ReadDIN( DWORD board );
void				ToggleDIO( DWORD board );
void				ReadADC( DWORD board );
void				PrintAllWatchdogs (void);
void				RampDAC( DWORD board );
void				Toggle( int brd );
void				MeasurePulseWidth( DWORD board );
void				MeasureFrq( DWORD board );
void				ErrorFunction0 (DWORD ErrFlags);
void				ErrorFunction1 (DWORD ErrFlags);
void				MeasureADC( DWORD board );
void				TestDO( DWORD board );
void				MeasureDI( DWORD board );
void				TestDAC( DWORD board );
void				ScatteredAcq( DWORD board );
void				IntDIO( DWORD board );
void				ToggleRL( int brd );
void				EncoderTest( DWORD board );
void				QuadratureTest( DWORD board );


#define MAXLEN		60		// Maximum length of user command string.
#define	MAXBUF		MAXLEN		// maximum number of characters in input command buffer.

int			numboards = 0;
unsigned long		errFlags  = 0x0;	// error flags
char			cmdbuff[MAXBUF + 1];	// buffer for input command

static int		cnt0 = 0;		// count for interrupts from 626 board #0
static int		cnt1 = 0;		// count for interrupts from 626 board #1
WORD			cntr_chan0 = CNTR_2B;	// Tested, using all counters to generate interrupts:
						//		5 - CNTR_2B, 4 - CNTR_1B, 3 - CNTR_0B,
						//		2 - CNTR_2A, 1 - CNTR_1A, 0 - CNTR_0A
WORD			cntr_chan1 = CNTR_1A;	// Tested, using all counters to generate interrupts:
						//		5 - CNTR_2B, 4 - CNTR_1B, 3 - CNTR_0B,
						//		2 - CNTR_2A, 1 - CNTR_1A, 0 - CNTR_0A

const DWORD		zeros[] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
						// Array of zeros for fast resetting of interrupt counters.
DWORD			IntCounts[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };	// Interrupt counters.
pthread_mutex_t		CriticalSection = PTHREAD_MUTEX_INITIALIZER;
						// Synchronization mutex for thread-safe execution.

int main( int argc, char *argv[] )
{

  int                   i, ch;
  unsigned int		board;			// operating board
  unsigned int		brd0 = 0;		// 1st 626 board #0
  unsigned int		brd1 = 1;		// 2nd 626 board #1
  char			cmd[MAXLEN];
  char			arg[MAXLEN];
  char			*c;
  unsigned long		bus_slot;

  memset(cmdbuff,0, MAXBUF+1);
  printf("\n");
  S626_OpenBoard( brd0, 0, InterruptAppISR0, 1 );
  numboards++;
  S626_OpenBoard( brd1, 0, InterruptAppISR1, 1 );
  numboards++;
  // show all S626 boards found in the system and their slots.
  printf ("Found %d Sensoray Model 626 board%c.\n", numboards, numboards > 1 ? 's' : ' ');
/*
  for (i = 0; i < numboards; i++) 
  {
  	bus_slot = S626_GetAddress ((HBD)i);
  	printf ("Board(%d) is on PCI bus %d; slot %d\n",
			i, S626_GetAddress ((HBD)i) >> 16, S626_GetAddress ((HBD)i) & 0xffff);
	//		i, bus_slot>>16, bus_slot&0xffff);
  }
*/
  
  S626_InterruptEnable( brd0, FALSE );
  S626_InterruptEnable( brd1, FALSE );
  S626_SetErrCallback( brd0, ErrorFunction0 );
  S626_SetErrCallback( brd1, ErrorFunction1 );
  
  errFlags = S626_GetErrors (brd0);
  printf("Board(%i) -  ErrFlags = 0x%x \n", brd0, errFlags );

  if (errFlags!=0x0) 
  {
	  printf("Board #0 open/installation with Err = 0x%x : ");
	  switch (errFlags)
	  {
		  case ERR_OPEN:
			  printf("\t\t Can't open driver.\n"); break;
		  case ERR_CARDREG:
			  printf("\t\t Can't attach driver to board.\n"); break;
		  case ERR_ALLOC_MEMORY:
			  printf("\t\t Memory allocation error.\n"); break;
		  case ERR_LOCK_BUFFER:
			  printf("\t\t Error locking DMA buffer.\n"); break;
		  case ERR_THREAD:
			  printf("\t\t Error starting a thread.\n"); break;
		  case ERR_INTERRUPT:
			  printf("\t\t Error enabling interrupt.\n"); break;
		  case ERR_LOST_IRQ:
			  printf("\t\t Missed interrupt.\n"); break;
		  case ERR_INIT:
			  printf("\t\t Board object not instantiated.\n"); break;
		  case ERR_SUBIDS:
			  printf("\t\t PCI SubDevice/SubVendor ID mismatch.\n"); break;
		  case ERR_CFGDUMP:
			  printf("\t\t PCI configuration dump failed.\n"); break;
		  default:
			  printf("\t\t other Unknown errors.\n"); break;
	  }
	  printf("Fix board open/installation error, and then try again. \n\n");
	  return -1;
  }

  errFlags = S626_GetErrors (brd1);
  printf("Board(%i) -  ErrFlags = 0x%x \n", brd1, errFlags );

  if (errFlags!=0x0) 
  {
	  printf("Board #1 open/installation with Err = 0x%x : ");
	  switch (errFlags)
	  {
		  case ERR_OPEN:
			  printf("\t\t Can't open driver.\n"); break;
		  case ERR_CARDREG:
			  printf("\t\t Can't attach driver to board.\n"); break;
		  case ERR_ALLOC_MEMORY:
			  printf("\t\t Memory allocation error.\n"); break;
		  case ERR_LOCK_BUFFER:
			  printf("\t\t Error locking DMA buffer.\n"); break;
		  case ERR_THREAD:
			  printf("\t\t Error starting a thread.\n"); break;
		  case ERR_INTERRUPT:
			  printf("\t\t Error enabling interrupt.\n"); break;
		  case ERR_LOST_IRQ:
			  printf("\t\t Missed interrupt.\n"); break;
		  case ERR_INIT:
			  printf("\t\t Board object not instantiated.\n"); break;
		  case ERR_SUBIDS:
			  printf("\t\t PCI SubDevice/SubVendor ID mismatch.\n"); break;
		  case ERR_CFGDUMP:
			  printf("\t\t PCI configuration dump failed.\n"); break;
		  default:
			  printf("\t\t other Unknown errors.\n"); break;
	  }
	  printf("Fix board open/installation error, and then try again. \n\n");
	  return -1;
  }


  board = brd0;		// initially
  
  // print Main Menu.
  MainMenu();

  // main continuous loop.
  for (;;)
  {
    printf ("\n->");

    // Input user command string, and if no errors encountered ...
    if ( fgets (cmdbuff, MAXBUF, stdin) != NULL )
    {
      // Truncate newline char at end of command string.
	  ch = cmdbuff[0];	// save 1st char of the command, before truncating.
      cmdbuff[ strlen( cmdbuff ) - 1 ] = 0;

      // If command line is not blank ...
      if ( cmdbuff[0] != 0 )
      {
		  // Force command string to lower case.
		  for ( c = cmdbuff; *c != 0; c++ ) *c = tolower( *c );

		  // Parse command string into opcode and argument.
		  cmd[0] = arg[0] = 0;
		  sscanf( cmdbuff, "%s %s", cmd, arg );

		  // Terminate application.
		  if ( ch == 'x' ) break;	// exit.

		  // for command = '0', '1', '2', ..., numboards.
		  if ( ch>='0' && ch<='9' && (ch-'0')<numboards )
		  {
			  board = ch-'0';
			  Toggle (board);	//toggle the corresponding boards watchdog state.
			  continue;
		  }

		  // for command = ' ' - space, flash all LEDs.
		  if ( ch == ' ' )
		  {
			  for ( i=0; i<numboards; i++ ) Toggle (i);	// toggle all the present boards watchdog state.
			  continue;
		  }

		  // for other commands.
		  else if ( strcmp( cmd, "board" ) == 0 ) {
			  printf("\tCurrent operating 626 board # = %i \n", board); 
			  		// print current operating 626 board number.
			  continue;
		  } else if ( strcmp( cmd, "brd" ) == 0 ) {
			  printf("\tCurrent operating 626 board # = %i \n", board); 
			  		// print current operating 626 board number.
			  continue;
		  } else if ( strcmp( cmd, "di" ) == 0 ) {
			  ReadDIN( board );		// read all digital inputs.
			  continue;
		  } else if ( strcmp( cmd, "i" ) == 0 ) {	// shortcut of "di"
			  ReadDIN( board );		// read all digital inputs.
			  continue;
		  } else if ( strcmp( cmd, "do" ) == 0 ) {
			  ToggleDIO( board );	// toggle all digital outputs.
			  continue;
		  } else if ( strcmp( cmd, "o" ) == 0 ) {	// shortcut of "do"
			  ToggleDIO( board );	// toggle all digital outputs.
			  continue;
		  } else if ( strcmp( cmd, "rdad" ) == 0 ) {
			  ReadADC( board );		// read analog input.
			  continue;
		  } else if ( strcmp( cmd, "a" ) == 0 ) {	// shortcut of "rdad"
			  ReadADC( board );		// read analog input.
			  continue;
		  } else if ( strcmp( cmd, "rpda" ) == 0 ) {
			  RampDAC( board );		// ramp DAC output.
			  continue;
		  } else if ( strcmp( cmd, "d" ) == 0 ) {	// shortcut of "rpda"
			  RampDAC( board );		// ramp DAC output.
			  continue;
		  } else if ( strcmp( cmd, "mpw" ) == 0 ) {
			  MeasurePulseWidth( board );	// measure pulse width.
			  continue;
		  } else if ( strcmp( cmd, "w" ) == 0 ) {	// shortcut of "mplsw"
			  MeasurePulseWidth( board );	// measure pulse width.
			  continue;
		  } else if ( strcmp( cmd, "mfrq"  ) == 0 ) {
			  MeasureFrq( board );			// measure frequency.
			  continue;
		  } else if ( strcmp( cmd, "f" ) == 0 ) {	// shortcut of "mfrq"
			  MeasureFrq( board );			// measure frequency.
			  continue;
		  } else if ( strcmp( cmd, "intrpt" ) == 0 ) {
			  IntTest( board );				// test interrupt with counter.
			  continue;
		  } else if ( strcmp( cmd, "r" ) == 0 ) {	// shortcut of "intrpt"
			  IntTest( board );				// test interrupt.
			  continue;
		  } else if ( strcmp( cmd, "tad"  ) == 0 ) {
			  MeasureADC( board );			// measure ADC throughput.
			  continue;
		  } else if ( strcmp( cmd, "tdo"  ) == 0 ) {
			  TestDO( board );				// test Digital Output throughput.
			  continue;
		  } else if ( strcmp( cmd, "tdi"  ) == 0 ) {
			  MeasureDI( board );			// measure Digital Input throughput.
			  continue;
		  } else if ( strcmp( cmd, "tda"  ) == 0 ) {
			  TestDAC( board );				// test DAC throughput.
			  continue;
		  } else if ( strcmp( cmd, "sacq"  ) == 0 ) {
			  ScatteredAcq( board );		// Scattered Acquisition from all ADC channels.
			  continue;
		  } else if ( strcmp( cmd, "s"  ) == 0 ) {
			  ScatteredAcq( board );		// Scattered Acquisition from all ADC channels.
			  continue;
		  } else if ( strcmp( cmd, "intdio"  ) == 0 ) {
			  IntDIO( board );				// test DIO interrupts.
			  continue;
		  } else if ( strcmp( cmd, "relay"  ) == 0 ) {
			  ToggleRL( board );			// test Relay on board.
			  continue;
		  } else if ( strcmp( cmd, "l"  ) == 0 ) {
			  ToggleRL( board );			// test Relay on board.
			  continue;
		  } else if ( strcmp( cmd, "enc"  ) == 0 ) {
			  EncoderTest( board );			// test encoder with encoder simulator.
			  continue;
		  } else if ( strcmp( cmd, "quadr"  ) == 0 ) {
			  QuadratureTest( board );		// test real quadrature encoder.
			  continue;
		  } else if ( strcmp( cmd, "qdr"  ) == 0 ) {
			  QuadratureTest( board );		// test real quadrature encoder.
			  continue;
		  } else {
			  MainMenu();			// print main manu
		  }

	  } else {

		  MainMenu();	// print main manu.

	  }
	}
  }

  // close all boards.
  printf("\n");
  //return 0;	// close everything automatically by OS
  for (i = numboards-1; i >= 0; i--) 
  {
	S626_CloseBoard (i);	//close 626 boards, reversely
	printf("Board(%i) is closed.\n", i);
  }

  printf("\n");

}

//***** Print the main screen menu
void MainMenu (void)
{
  int                       i;

  // show all S626 boards found in the system and their slots.
  printf ("\n\t\t\t  Sensoray 626 Demo Ver 4.0  \n"
			"\t\t\t  -------------------------  \n");

  printf ("\n==============================================================================\n\n"
		"Basic Demo/Tests:                                                             \n\n"
		"   board ....... print current operating Sensoray Model 626 board               \n"
		"   0 ........... toggle the flashing of the 1st boards LED's on/off,            \n"
		"                 and set operating board to the 1st 626 board                   \n"
		"   1 ........... toggle the flashing of the 2nd boards LED's on/off,            \n"
		"                 and set operating board to the 2nd 626 board                   \n"
		"   2 ........... toggle the flashing of the 3rd boards LED's on/off,            \n"
		"                 and set operating board to the 3rd 626 board                   \n"
		"   3 ........... toggle the flashing of the 4th boards LED's on/off,            \n"
		"                 and set operating board to the 4th 626 board                   \n"
		"   space ....... toggle the flashing of all the boards LED's on/off             \n"
		"   relay ....... toggle relay on/off                                            \n"
		"   dO .......... toggle all digital outputs ( DIO: 47-0, 0=+5V, 1=0V )          \n"
		"   dI .......... read all digital inputs ( DIO: 47-0, 0=+5V, 1=0V )             \n"
		"   rdAD ........ read any analog input ( channel: 0-15 )                        \n"
		"   rpDA ........ ramp any DAC output ( channel: 0-3, -10V_/+10V, repeatly )     \n"
		"   Sacq ........ scattered acquisition from all ADC channels                    \n"
		"   mpW ......... use counter to measure the width of a pulse                    \n"
		"   mFrq ........ use a pair of counter to measure the frequency of a signal     \n"
		"   intRpt ...... generate a 1 second interrupt via counter                      \n"
		"   intDIO ...... count interrupts occured on DIO channels                       \n"
		"   enc ......... encoder test (a simulated encoder is provided in demo program) \n"
		"   quadr ....... use counter to test real quadrature encoder                    \n"
		"\nPerformance Tests:                                                          \n\n"
		"   tDO ......... test Digital Output throughput                                 \n"
		"   tDI ......... test Digital Input throughput                                  \n"
		"   tAD ......... test ADC throughput                                            \n"
		"   tDA ......... test DAC throughput                                            \n"
		"\nPress x to exit.                                                            \n\n"
		"==============================================================================\n\n", numboards);
  printf ("On the following board/s the watchdog LED should be:  \n\n");
  PrintAllWatchdogs ();
}


//***** Take a 16-bit value and print it in binary (msb --> lsb)
void HexBin (int data)
{
  int  i;

  for (i = 0; i < 16; i++)
  {
    printf ((data << i) & 0x8000 ? "1" : "0");
    if (i % 4 == 3)
      printf (" ");
  }
}

//***** Error routine: prints error code
void Err (DWORD ecode)
{
  printf ("\n\n\n**********************\n"
	  "    Error %d\n" "**********************\n\n", ecode);
}

//***** print error code

VOID ErrorFunction0 (DWORD ErrFlags)
{
  printf ("Got an error on board #0 0x%x\n", ErrFlags);
}
VOID ErrorFunction1 (DWORD ErrFlags)
{
  printf ("Got an error on board #1 0x%x\n", ErrFlags);
}

//***** Print the state of all the boards watchdogs
void PrintAllWatchdogs (void)
{
  int                       i, data;

  for (i = 0; i < numboards; i++)
  {
    if ((i + 1) != 1)
      printf (", ");				//only print , if this is not the first
    data = S626_WatchdogEnableGet (i);
    if (data)
      printf ("\tBoard %d - Flashing", i);	//true if watchdog is enabled
    else
      printf ("\tBoard %d - Disabled", i);
  }
  printf ("\n");
}


//***** Toggle the watchdog of the specified board on/off *******
//*  The watchdog timing out in 1/8th sec
//*  and then flashing at a period of 1/8th sec
//***************************************************************

void Toggle (int brd)
{
  int	data;

  data = S626_WatchdogEnableGet (brd);	//read register containing watchdog enabled flag
  if (data)
  {				//true if watchdog is enabled
    //Disable watchdog
    S626_WatchdogEnableSet (brd, FALSE);
  }
  else
  {
    //Enable watchdog with 1/8 period
    S626_WatchdogPeriodSet (brd, 0);
    S626_WatchdogEnableSet (brd, TRUE);
  }
  PrintAllWatchdogs ();
}


//*************************** RampDAC ****************************
//*  Ramp a selected DAC channel of board 0.
//*  output a waveform that will continuosly ramp from minimum
//*  to maximum output voltage.
//****************************************************************

void RampDAC( DWORD board )
{
  int				chan;
  int				steps;
  int				max = 100000;
  int				cnt;
  SHORT				i;

  printf ("Enter ramping Channel # and steps [0-3 1-%i, q] : ", max );

  for (;;)
  {
	  fgets (cmdbuff, MAXBUF, stdin);
	  if (cmdbuff[0] == 'q') return;
	  sscanf( cmdbuff, "%d %i", &chan, &steps );
	  if ( chan<0 || chan>3 )
	  {
		  printf ("Enter ramping Channel # and steps [0-3 1-%i, q] : ", max );
		  continue;
	  }
	  if ( steps<1 || steps>max )
	  {
		  printf ("Enter ramping Channel # and steps [0-3 1-%i, q] : ", max );
		  continue;
	  }

#if 0    	// test setpoint for DACs
    printf("\tenter setpoint [ - 8190, + 8190 ] : ");
    scanf("%i", &i); if (i<-8190) i=-8190; if (i>8190) i=8190;
    S626_WriteDAC (board, chan, i);
    printf ("\tChannel #%d : DAC output = %i ( %f V ) \n", chan, i, 10.0*(float)(i)/8191.0);
    printf ("Channel [0-3, q] : ");
#endif
//#if 0    	// jump-rampimg: step = 819
    for (cnt = 0, i = -8190; cnt < steps; cnt++)
    {
      S626_WriteDAC (board, chan, i);
      printf ("\tChannel# %d :  DAC output [%d] = %6i ( %f V ) \n", chan, cnt, i, 10.0*(float)(i)/8191.0);
      usleep(100000);        // tested: 1.0s; 0.1s; 10ms; 1ms; 100us; 20us; 10us
      i = i+818;  if (++i > 8190) i = -8190;
    }
	printf ("Enter ramping Channel # and steps [0-3 1-%i, q] : ", max );
//#endif
#if 0    	// jump-rampimg: step = 90
    for (cnt = 0, i = -8190; cnt < (5*92*2-5); cnt++)
    {
      S626_WriteDAC (board, chan, i);
      usleep(10);        // tested: >= 10us
      i = i+89;  if (++i > 8190) i = -8190;
    }
    printf ("Channel [0-3, q] : ");
#endif
#if 0
    for (cnt = 0, i = -8190; cnt < 8190*2; cnt++)
    {
      S626_WriteDAC (board, chan, i);
      usleep(20);        // tested >= 20 us	// the DAC Conversion-time = 20 microseconds
      if (++i > 8190) i = -8190;
    }
    printf ("Channel [0-3, q] : ");
#endif

  }

}


//********************* ReadADC ****************************
//*  Read a selected ADC channel of board 0.
//**********************************************************

void ReadADC( DWORD board )
{
  BYTE				poll_list[16];		//used to setup the ADC
  WORD				data[16];		//holds the values read by the ADC
  int				chan, gain;
  int				ch;

  // Setup the poll list
  // The channels can be read in any order with any gain combination.
  // If less than the full 16 positions of the poll list are used then
  // the last position that is to be converted must have the End Of
  // Poll List (EOPL) bit set.

  gain = ADC_RANGE_10V;		//0->10V, 1->5V gain

  printf ("Channel [0-15, q] : ");	fflush(stdout);

  for (;;)
  {
    fgets (cmdbuff, MAXBUF, stdin);
    if (cmdbuff[0] == 'q')
      return;
    ch = atoi (cmdbuff);
    if ( ch==0 ) ch = (cmdbuff[0]=='0')? 0 : -1;     //printf ("\n\tbuff[0] = %s\n", cmdbuff);
    if (ch < 0 || ch > 15) {
       printf ("Channel [0-15, q] : ");
       continue;
    }
    chan = ch;
    poll_list[0] = (gain << 4) | chan | ADC_EOPL;	//one channel will be read
    S626_ResetADC (board, &poll_list[0]);		//set up the poll list
    S626_ReadADC (board, &data[0]);
    printf ("\tADC channel %d = %d ( %f V) \n", chan, (short) data[0], ((float)((short)data[0]))/3276.7);  //when gain = ADC_RANGE_10V
    printf ("Channel [0-15, q] : ");
  }
}


//**********************ToggleDIO********************************
//** Toggle all digital outputs of board 0
//***************************************************************

void ToggleDIO( DWORD board )
{
  static WORD	data = 0;

  printf ("\n"	//Press ESC to return to the main menu.\n\n"
		  "47               32   31               16   15                0   (DIO #)\n");

  S626_DIOWriteBankSet (board, 0, data);	//channels 1-16
  S626_DIOWriteBankSet (board, 1, data);	//channels 17-32
  S626_DIOWriteBankSet (board, 2, data);	//channels 33-48

  HexBin (data);		//convert data to binary & print it
  printf ("- ");
  HexBin (data);		//convert data to binary & print it
  printf ("- ");
  HexBin (data);		//convert data to binary & print it
  printf (" (all +%dV)\n", (data==0?5:0) );

  data = ~data;

}


//**************** Read digital inputs ********************
//*  Read digital inputs of board 0 and display the
//*  curent status of each digital input channel.
//*********************************************************

void ReadDIN( DWORD board )
{
  int                       data;

  printf ("\n"	//Press ESC to return to the main menu.\n\n"
		  "47               32   31               16   15                0   (DIO#)\n");

  S626_DIOWriteBankSet (board, 0, 0);	//channels 1-16 ready for input (pulled up)
  S626_DIOWriteBankSet (board, 1, 0);	//channels 17-32 ready for input (pulled up)
  S626_DIOWriteBankSet (board, 2, 0);	//channels 33-48 ready for input (pulled up)

  data = S626_DIOReadBank (board, 2);	//read inputs 33-48
  //printf ("\tdata [47:32] = 0x%x\n", data);
  HexBin (data);		//convert data to binary & print it

  printf ("- ");

  data = S626_DIOReadBank (board, 1);	//read inputs 17-32
  //printf ("\n\tdata [31:16] = 0x%x\n", data);
  HexBin (data);		//convert data to binary & print it

  printf ("- ");

  data = S626_DIOReadBank (board, 0);	//read inputs 1-16
  //printf ("\n\tdata [15:0 ] = 0x%x\n", data);
  HexBin (data);		//convert data to binary & print it

  printf (" (value)\n");
}


//*******************************IntTest**********************************
//*  Interrupt test:
//*  Sets up Counter 2A as a free running system down counter.
//*  Interrupt occurs on counter overflow.
//************************************************************************/

void IntTest( DWORD board )
{
  unsigned char         ch;		// for storing 1st letter from stdin
  WORD			cntr_chan = 0;	// default:   0 - CNTR_0A

  // Reset Counter
  if ( board == 0 ) cntr_chan = cntr_chan0;
  if ( board == 1 ) cntr_chan = cntr_chan1;
  S626_CounterCapFlagsReset (board, cntr_chan);
  // Set counter operating mode
  S626_CounterModeSet (board, cntr_chan,
		       (LOADSRC_INDX << BF_LOADSRC) |	// Index causes preload
		       (INDXSRC_SOFT << BF_INDXSRC) |	// Hardware index disabled
		       (CLKSRC_TIMER << BF_CLKSRC) |	// Operating mode is Timer
		       (CNTDIR_DOWN << BF_CLKPOL) |	// Counting direction is Down
		       (CLKMULT_1X << BF_CLKMULT) |	// Clock multiplier is 1x
		       (CLKENAB_INDEX << BF_CLKENAB));	// Counting is initially disabled
  // Set PreLoad value
  S626_CounterPreload (board, cntr_chan, 100 * 2000);	// when CLKSRC_TIMER, clk src is the fixed 2 MHz on-board clock.
  							// so, every 100*2000/2000000 = 100 ms generate an interrupt
  // Generate a soft index to force transfer of PreLoad value into counter core
  S626_CounterSoftIndex (board, cntr_chan);
  // Enable transfer of PreLoad value to counter in response to overflow. It will
  // cause the initial counts to reload every time when the counts reach zero
  S626_CounterLoadTrigSet (board, cntr_chan, LOADSRC_OVER);
  // Enable the counter to generate interrupt requests upon captured overflow
  S626_CounterIntSourceSet (board, cntr_chan, INTSRC_OVER);
  // Enable the timer. The first interrupt will occur after the specified time interval alapses.
  S626_CounterEnableSet (board, cntr_chan, CLKENAB_ALWAYS);

  // Enable interrupt to start the test
  S626_InterruptEnable (board, TRUE);

  printf ("\nAny time, enter 'q' to quit from interrupt-test: \n\n");
  for (;;)
  {
    //printf ("?\n");
    fgets (cmdbuff, MAXBUF, stdin);
    ch = cmdbuff[0];
    if (ch == 'q')
      break;
  }

  // Disable interrupts when finished the test
  S626_InterruptEnable (board, FALSE);

}


//*************************InterruptHandler0 *************************
//*  This routine is called when an interrupt occurs from 626 Board #0
//********************************************************************

void InterruptAppISR0( DWORD board )
{
  time_t		time_now;
  WORD			intStatus[4];
  register WORD		status;
  register DWORD	*pCounts;
  register WORD		mask;
  BOOL                  bIE = FALSE;
  
  DWORD 		brd = 0;		// we know this ISR is for board #0

  S626_InterruptStatus (brd, intStatus);	// Fetch IRQ status for all sources.
  //printf("\tintStatus = 0x%x 0x%x 0x%x 0x%x\n", intStatus[0], intStatus[1], intStatus[2], intStatus[3]);

  //if (intStatus[3]==0x4000) {  // interrupt from Counter 2A overflow
  if (intStatus[3]&0xfc00) {  // interrupts from all Counter's overflow
	cnt0++;
	if (cnt0 % 10 == 0) {
		//printf("\tintStatus = 0x%x 0x%x 0x%x 0x%x\n", intStatus[0], intStatus[1], intStatus[2], intStatus[3]);
		//printf ("%6d - %10d\n", cnt0, time (0));
		time(&time_now);
		//printf ("\tinterrupt # %d ( from Counter-2A ), occured at %s \n", cnt0, ctime(&time_now));
		printf ("\tinterrupt #%d ( from Counter-%d of board #%d ), occured at %s \n", cnt0, cntr_chan0, brd, ctime(&time_now));
	}
	S626_CounterCapFlagsReset( brd, cntr_chan0 );	//reset counter interrupts

	// Unmask boards master interrupt enable.
	//S626_InterruptEnable( brd, TRUE );		//Re-enable interrupts
	bIE = TRUE;
  }

  if (intStatus[0])		// interrupts from DIO channel 0-15
  {
	  // Cache a copy of DIO channel 0-15 interrupt request (IRQ) status.
	  status = intStatus[0];	// Cache DIO 0-15 IRQ status.

	  // Tally DIO 0-15 interrupts.
	  pCounts = IntCounts;		// Init pointer to interrupt counter.

	  pthread_mutex_lock(&CriticalSection);	// * Start thread-safe section -----------
	  for ( mask = 1; mask != 0; pCounts++ )
	  { // *
		  if ( status & mask )		// * If DIO is requesting service ...
			  ( *pCounts )++;	// * increment DIOs interrupt counter.
		  mask += mask;			// * Bump mask.
	  } // *
	  pthread_mutex_unlock(&CriticalSection);	// * End thread-safe section -------------

	  // Negate all processed DIO interrupt requests.
	  S626_DIOCapReset( brd, 0, status );		// group #0: DIO 0-15

	  // Unmask boards master interrupt enable.
	  //S626_InterruptEnable( brd, TRUE );		//Re-enable interrupts
	  bIE = TRUE;
  }
  if( bIE) {
    S626_InterruptEnable( brd, TRUE );		//Re-enable interrupts
  }
}

//*************************InterruptHandler0 *************************
//*  This routine is called when an interrupt occurs from 626 Board #1
//********************************************************************

void InterruptAppISR1( DWORD board )
{
  time_t			time_now;
  WORD				intStatus[4];
  register WORD		status;
  register DWORD	*pCounts;
  register WORD		mask;
  BOOL                  bIE = FALSE;
  DWORD 		brd = 1;		// we know this ISR is for board #1

  S626_InterruptStatus (brd, intStatus);	// Fetch IRQ status for all sources.
  //printf("\tintStatus = 0x%x 0x%x 0x%x 0x%x\n", intStatus[0], intStatus[1], intStatus[2], intStatus[3]);

  //if (intStatus[3]==0x4000) {  // interrupt from Counter 2A overflow
  if (intStatus[3]&0xfc00) {  // interrupts from all Counter's overflow
	cnt1++;
	if (cnt1 % 10 == 0) {
		//printf("\tintStatus = 0x%x 0x%x 0x%x 0x%x\n", intStatus[0], intStatus[1], intStatus[2], intStatus[3]);
		//printf ("%6d - %10d\n", cnt1, time (0));
		time(&time_now);
		//printf ("\tinterrupt # %d ( from Counter-2A ), occured at %s \n", cnt1, ctime(&time_now));
		printf ("\tinterrupt #%d ( from Counter-%d of board #%d ), occured at %s \n", cnt1, cntr_chan1, brd, ctime(&time_now));
	}
	S626_CounterCapFlagsReset( brd, cntr_chan1 );	//reset counter interrupts

	// Unmask boards master interrupt enable.
	//S626_InterruptEnable( brd, TRUE );		//Re-enable interrupts
	bIE = TRUE;
  }

  if (intStatus[0])		// interrupts from DIO channel 0-15
  {
	  // Cache a copy of DIO channel 0-15 interrupt request (IRQ) status.
	  status = intStatus[0];	// Cache DIO 0-15 IRQ status.

	  // Tally DIO 0-15 interrupts.
	  pCounts = IntCounts;		// Init pointer to interrupt counter.

	  pthread_mutex_lock(&CriticalSection);		// * Start thread-safe section -----------
	  for ( mask = 1; mask != 0; pCounts++ )
	  { // *
		  if ( status & mask )		// * If DIO is requesting service ...
			  ( *pCounts )++;	// * increment DIOs interrupt counter.
		  mask += mask;			// * Bump mask.
	  } // *
	  pthread_mutex_unlock(&CriticalSection);	// * End thread-safe section -------------

	  // Negate all processed DIO interrupt requests.
	  S626_DIOCapReset( brd, 0, status );		// group #0: DIO 0-15

	  // Unmask boards master interrupt enable.
	  //S626_InterruptEnable( brd, TRUE );		//Re-enable interrupts
	  bIE = TRUE;
  }
  if( bIE) {
    S626_InterruptEnable( brd, TRUE );		//Re-enable interrupts
  }

}


////////////////////////////////////////////////////////////////////
// Configure an A/B counter pair to measure frequency.
// The counter pair is specified by the A member of the pair.
// Acquisition gate time is specified in milliseconds.
////////////////////////////////////////////////////////////////////

VOID CreateFreqCounter( HBD hbd, WORD CounterA, WORD GateTime )
{
	// Set operating mode for counterA.
	S626_CounterModeSet( hbd, CounterA,
		( LOADSRC_OVER << BF_LOADSRC ) |   // Preload upon overflow.
		//( LOADSRC_INDX << BF_LOADSRC ) |   // Preload upon index.  works too.
		( INDXSRC_SOFT << BF_INDXSRC ) |   // Disable hardware index.
		( CLKSRC_TIMER << BF_CLKSRC ) |    // Operating mode is Timer.
		//( CLKSRC_COUNTER << BF_CLKSRC ) |     // Operating mode is Counter.  working
							// tested: driven by ext 2 MHz clock form xA - B+
		( CNTDIR_DOWN  << BF_CLKPOL ) |    // Count direction is Down.
		( CLKMULT_1X   << BF_CLKMULT ) |   // Clock multiplier is 1x.
		( CLKENAB_INDEX << BF_CLKENAB ) ); // Counting is initially disabled.

	// Set counterA core and preload value to the desired gate time. Since the counter
	// clock is fixed at 2 MHz, this is computed by multiplying milliseconds by 2,000.
	S626_CounterPreload( hbd, CounterA, GateTime * 2000 );
	S626_CounterSoftIndex( hbd, CounterA );

	// Enable preload of counterA in response to overflow. This causes the timer to
	// restart automatically when its counts reach zero.
	S626_CounterLoadTrigSet( hbd, CounterA, LOADSRC_OVER );

	// Set operating mode for counterB.
	S626_CounterModeSet( hbd, CounterA + 3,
		( LOADSRCB_OVERA << BF_LOADSRC ) |  // Preload zeros upon leading gate edge.
		( INDXSRC_SOFT   << BF_INDXSRC ) |  // Hardware index is disabled.
		( CLKSRC_COUNTER << BF_CLKSRC ) |   // Operating mode is Counter.
		//( CLKSRC_TIMER   << BF_CLKSRC ) |     // take Timer mode.  driven with internal clock, for debugging
		( CLKPOL_POS     << BF_CLKPOL ) |   // Clock is active high.
		( CLKMULT_1X     << BF_CLKMULT ) |  // Clock multiplier is 1x.
		( CLKENAB_ALWAYS << BF_CLKENAB ) ); // Clock is always enabled.

	// Initialize counterBs preload value to zero so that counterB core will be set
	// to zero in response to trailing gate edge (counter A overflow).
	S626_CounterPreload( hbd, CounterA + 3, 0 );

	// Enable latching of counterBs acquired frequency data in response to trailing
	// gate edge (counterA overflow).
	S626_CounterLatchSourceSet( hbd, CounterA + 3, LATCHSRC_B_OVERA );

	// Enable the acquisition gate generator.
	S626_CounterEnableSet( hbd, CounterA, CLKENAB_ALWAYS );

}

//******************************MeasureFrequency*******************************
//*  Counter test:   Frequency Measurement application
//*  Sets up Counter xA as a timer & xB as a measuring counter.
//*****************************************************************************/

void MeasureFrq( DWORD board )
{
	WORD            Counter = CNTR_0A;	// CNTR_0A:	signal input from J4-pin5 (CNTR_0B-B+)
						// CNTR_1A:	signal input from J4-pin14 (CNTR_1B-B+)
						// CNTR_2A:	signal input from J4-pin23 (CNTR_2B-B+) 
	int				i,j, times;
	int				max = 100;
	WORD			data = ~0;
	unsigned char	ch = ' ';
	time_t			time_now;
	BOOL			IsFirst = FALSE;

	// Reset Counter A
	S626_CounterCapFlagsReset( board, Counter );

	// Configure counter A/B pair as a frequency counter, gate time = 1 second.
	CreateFreqCounter( board, Counter, 1000 );	// Counter A (and implicitly, B) channel to use.

	// Ignore the data value from the first gate period.
	IsFirst = TRUE;

	for ( ; ; )
	{
#if 0
		// for testing purpose: generate TTL signals (0-5V) out to DIO channels 1-48.
		// one of them can be used for feeding Counter 0B's external driven input B+ (J4-pin5),
		// as a single-phase external clock source
		data = ~0;
		for ( i=0; i<20001; i++) {	// for generating ~22kHz, with my PC
			S626_DIOWriteBankSet (board, 0, data);	//DIO channels 1-16
			S626_DIOWriteBankSet (board, 1, data);	//DIO channels 17-32
			S626_DIOWriteBankSet (board, 2, data);	//DIO channels 33-48
			data = ~data;
		}
#endif
		// printf( "\nIsFirst = %i, CounterCapStatus = 0x%x \n",  IsFirst, S626_CounterCapStatus(board) );
		// Wait for a captured overflow event on counter A, then clear its capture flag.
		while ( !( S626_CounterCapStatus( board ) & OVERMASK( Counter ) ) );
		S626_CounterCapFlagsReset ( board, Counter );

		// Read and display measured frequency from counter B.
		if ( !IsFirst ) {
			//printf( "\n\tFrequency (Hz) = %8d.\n", S626_CounterReadLatch( board, Counter+3 ) );
			printf( "\n\tFrequency (Hz) = %d.\n", S626_CounterReadLatch( board, Counter+3 ) );
			time(&time_now);
			printf ("\tmeasuring_period = 1 second, finished at %s ", ctime(&time_now));
		} else {
			IsFirst = FALSE;
			continue;
		}

		// continue or quit?
		if (ch=='c') 
		{
			times++;
			if ( times<max ) continue;	// before times reach max, you have to use Ctrl+C to quit.
		}
		// Or if you want to quit to main manu, use following
		// when you use an external signal generator ( Clock TTL in through J4-pin5 (Counter 0B -- B+ ) )
		printf ("\nEnter 'q' to quit, 'c' to measure %d more times, or any other key to test one more time. > ", max);
		fgets (cmdbuff, MAXBUF, stdin);
		ch = cmdbuff[0];
		times = 0;		// clear times cnt.
		if (ch == 'q') break;

	}

}


////////////////////////////////////////////////////////////////////
// Measure positive pulse width using board 1, counter xA.
////////////////////////////////////////////////////////////////////

void MeasurePulseWidth( DWORD board )
{
	WORD            Counter = CNTR_0A;	// CNTR_0A:	pulse input from J5-pin8 (CNTR_0A-I+)
						// CNTR_1A:	pulse input from J5-pin17 (CNTR_1A-I+)
						// CNTR_2A:	pulse input from J5-pin26 (CNTR_2A-I+)
	WORD            data    = 0;
	unsigned char   ch      = ' ';
	BOOL			IsFirst	= FALSE;
	int				i, times;
	int				max = 10;

	// Set counter operating mode.
	S626_CounterModeSet( board, Counter,
		( LOADSRC_INDX  << BF_LOADSRC ) |  // Preload in response to index.
		( INDXSRC_HARD  << BF_INDXSRC ) |  // Pulse signal drives index.
		( INDXPOL_POS   << BF_INDXPOL ) |  // Active high pulse signal.
		( CLKSRC_TIMER  << BF_CLKSRC ) |   // Operating mode is Timer.
		( CNTDIR_UP     << BF_CLKPOL ) |   // Count direction is Up.
		( CLKMULT_1X    << BF_CLKMULT ) |  // Clock multiplier is 1x.
		( CLKENAB_INDEX << BF_CLKENAB ) ); // Counting is gated by index.

	// Initialize preload value to zero so that the counter core will be set
	// to zero upon the occurance of a hardware index.
	S626_CounterPreload( board, Counter, 0 );

	// Enable latching of accumulated counts in response to an index. This assumes that
	// there is no conflict with the latch source used by paired counter xB.
	S626_CounterLatchSourceSet( board, Counter, LATCHSRC_A_INDXA );

	// Ignore the data value from the first event.
	IsFirst = TRUE;

	for ( ; ; )
	{
		sleep(1);
//#if 0
		// for testing purpose: generate TTL Pulse signals (0-5V) out to DIO channels 1-48.
		// one of them can be used for feeding Counter 1A's index input I+ (J5-pin17)
		// or Counter 2A's index input I+ (J5-pin26),
		// as an external signal source for which the pulse width needs to be measured
		data = ~0;	// DIO - pins = 0 V
		for ( i=0; i<2; i++) {	// generating a pulse with width = 22 us (natural delay with my computer here)
			S626_DIOWriteBankSet (board, 0, data);	//DIO channels 1-16
			S626_DIOWriteBankSet (board, 1, data);	//DIO channels 17-32
			S626_DIOWriteBankSet (board, 2, data);	//DIO channels 33-48
			data = ~data;
		}
//#endif
		// Wait for a captured index event on counter 1A, then clear its capture flag.
		while ( !( S626_CounterCapStatus( board ) & INDXMASK( Counter ) ) );
		S626_CounterCapFlagsReset( board, Counter );

		// Read and display measured pulse width. Since timer is clocked by a 2 MHz source,
		// accumulated count is divided by 2 to convert pulse width to microseconds units.
		if ( !IsFirst ) 
		{
			printf( "\n\tPulse Width = %d (s)\n", S626_CounterReadLatch( board, Counter ) >> 1 );
			fflush(stdout);
		} else {
			IsFirst = FALSE;
			continue;
		}

		// continue or quit?
		if (ch=='c') 
		{
			times++;
			if ( times<max ) continue;	// before times reach max, you have to use Ctrl+C to quit.
		}
		// Or if you want to quit to main manu, use following
		// when you use an external pulse signal source ( the TTL signal in through J5-pin17 (Counter 1A -- I+ ) )
		printf ("\nEnter 'q' to quit, 'c' to measure %d more times, or any other key to test one more time. > ", max);
		fgets (cmdbuff, MAXBUF, stdin);
		ch = cmdbuff[0];
		times = 0;		// clear times cnt.
		if (ch == 'q') break;

	}

}


/////////////////////////////////////////////////////////////////////////////////
// Return number of milliseconds elapsed since midnight? 
//				since the Epoch (00:00:00 UTC, January 1, 1970) ?

unsigned long get_time_now()
{

	struct timeval tv;		// Structure to receive current time.
	struct timezone tz;

	// Get the current time of day.
	gettimeofday( &tv, &tz );
	
	// Compute and return the elapsed milliseconds since midnight.
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/////////////////////////////////////////////////////////////////////////////////
// Return elapsed time in milliseconds.

unsigned long get_time_elapsed( unsigned long start_time )
{
	// Compute the elapsed milliseconds since start_time.
	unsigned long t_elapsed = get_time_now() - start_time;
	
	// Return elapsed time, compensating for timer wraparound if necessary.
	return ( ( t_elapsed & 0x10000000 ) != 0 ) ? ( t_elapsed + 86400000 ) : t_elapsed;
}


////////////////////////////////////////////////////////////////////
// Test 626 performance -- ADC throughput .
////////////////////////////////////////////////////////////////////

void MeasureADC( DWORD board )
{
  BYTE                  poll_list[16];		// used to setup the ADC
  WORD                  *data;		// holds the values read by the ADC
  int                   chan, gain;
  int                   ch;
  int					i;
  int					samples = 10000;	// <10000
  int					cnt_start, cnt_stop;
  int					period_detected = 0;
  unsigned long			start_time;
  unsigned long			time_spent;
  float					samplingRate;

// Setup the poll list
// The channels can be read in any order with any gain combination.
// If less than the full 16 positions of the poll list are used then
// the last position that is to be converted must have the End Of
// Poll List (EOPL) bit set.

  gain = ADC_RANGE_10V;		//0->10V, 1->5V gain

  printf ("Channel [0-15, q] : ");	fflush(stdout);
  data = (WORD *) malloc( 10000 * sizeof(WORD));
  for (;;)
  {
    fgets (cmdbuff, MAXBUF, stdin);
    if (cmdbuff[0] == 'q') {
      free(data);
      return;
    }
    ch = atoi (cmdbuff);
    if ( ch==0 ) ch = (cmdbuff[0]=='0')? 0 : -1;	//printf ("\n\tbuff[0] = %s\n", cmdbuff);
    if (ch < 0 || ch > 15) {
       printf ("Channel [0-15, q] : ");
       continue;
    }
    chan = ch;
    poll_list[0] = (gain << 4) | chan | ADC_EOPL;	//one channel will be read
    S626_ResetADC (board, &poll_list[0]);		//set up the poll list
	period_detected = 0;

	// start ADC.
	start_time = get_time_now();	// in milliseconds since the Epoch (00:00:00 UTC, January 1, 1970).

	for (i=0; i<samples; i++)
	{
		S626_ReadADC (board, &data[i]);
	}

	// end ADC read.
	time_spent = get_time_elapsed( start_time );	// in milliseconds.

	// calculate and print ADC sampling rate.
	samplingRate =  (float) samples / (float) time_spent;			// in kHz. Tested: max = ~19kHz (with my PC).
    printf ("\ttime_spent = %d (ms) to get %i samples -- Detected sampling rate = %f kHz \n", time_spent, samples, samplingRate);
	//printf ("\t\t( start_time = %lu (ms), time_spent = %d (ms) ) \n", start_time, time_spent);

#define PRTDATA
//#undef PRTDATA
#ifdef PRTDATA

    printf ("print data [p - print 100 samples, d - print a period, q - quit ? ]: ");

	fgets (cmdbuff, MAXBUF, stdin);

	if (cmdbuff[0] == 'p')
	{
		printf ("\nPrint ADC data from beginnig of ADC read: \n");
		for (i=0; i<100; i++)
		{
			printf ("\tADC channel[%d]: data[%d] = %d ( %f V) \n", 
				chan, i, (short) data[i], (float)((short)data[i])/3276.7);  //when gain = ADC_RANGE_10V
		}
	}

	if (cmdbuff[0] == 'd')
	{
		printf ("Print ADC data from a period start: \n");
		// detect a period start and end; print ADC data from a period start.
		cnt_start = 0;
		cnt_stop  = samples;
		period_detected = 0;
		for (i=0; i<samples; i++)
		{
			// detect a period start.
			if ( i>0 && ((short)data[i-1])<0 && ((short)data[i])>0 && cnt_start==0 ) {
				cnt_start = i;
			}
			// detect a period end.
			if ( i>0 && ((short)data[i-1])<0 && ((short)data[i])>0 && cnt_start<i ) {
				cnt_stop = i;
				period_detected = 1;
				break;
			}
			// print data[i].
			if (cnt_start>0)
				printf ("\tADC channel[%d]: data[%d] = %d ( %f V) \n", 
					chan, i, (short) data[i], (float)((short)data[i])/3276.7);  //when gain = ADC_RANGE_10V
		}
	}

#endif	//#ifdef PRTDATA

    printf ("Channel [0-15, q] : ");

  }
  free(data);
}


////////////////////////////////////////////////////////////////////
// Test 626 performance -- Digital output throughput .
////////////////////////////////////////////////////////////////////

void TestDO( DWORD board )
{
  static WORD		data = 0;
  long int			i, times, loop;
  const long 		max		= 1000000000;	// 1000 million.
  const int			t_hold	= 1000000;	// in us, for usleep(t_hold).	
							// Tested: >1, ..., 10000us, all -> ~50Hz switching rate (with my PC).
							//         20ms->33.3Hz, 50ms->16.7Hz, 100ms->9.1Hz, 1s->~1Hz
  int				bank = 0;		// default, point to groug #0 -- channels 0-15.
  unsigned long		start_time;
  unsigned long		time_spent;
  float				switchingRate;

  printf ("Enter DO Group # and toggling times [0-2 1-%i, q] : ", max);

  for (;;)
  {
	  fgets (cmdbuff, MAXBUF, stdin);
	  if (cmdbuff[0] == 'q') return;
	  sscanf( cmdbuff, "%d %d", &bank, &times );
	  if ( times<1 || times>max ) 
	  {
		  printf ("Enter DO Group # and toggling times [0-2 1-%i, q] : ", max);
		  continue;
	  }
	  if ( bank<0 || bank >2 ) 
	  {
		  printf ("Enter DO Group # and toggling times [0-2 1-%i, q] : ", max);
		  continue;
	  }

	  // running message. 
	  switch (bank)
	  {
		  case 0:
			  printf ("DO group #0 (channels 0-15): toggling ..... ");	fflush(stdout);
			  break;
		  case 1:
			  printf ("DO group #1 (channels 16-31): toggling ..... ");	fflush(stdout);
			  break;
		  case 2:
			  printf ("DO group #2 (channels 32-47): toggling ..... ");	fflush(stdout);
			  break;
		  default:
			  printf ("Invalid group value = %d\n", bank );
			  return;
	  }

	  // start toggling.
	  start_time = get_time_now();	// in milliseconds since the Epoch (00:00:00 UTC, January 1, 1970).

			  // toggling:
			  for ( i=0; i<times; i++ )
			  {
				  S626_DIOWriteBankSet (board, bank, data);	// bank=0 --> channels: 0-15
										// bank=1 --> channels: 16-31
										// bank=2 --> channels: 32-47
				  //usleep( t_hold );	// with linux, get 10 ms minimum. So, always, switchingRate < 50 Hz
				  //for ( loop=0; loop<1; loop++) { }		//   0,1 --> switchingRate = ~127 kHz (with my PC, max).
				  //for ( loop=0; loop<10; loop++) { }		//    10 --> switchingRate = ~126 kHz (with my PC).
				  //for ( loop=0; loop<100; loop++) { }		//   100 --> switchingRate = ~122 kHz (with my PC).
				  //for ( loop=0; loop<1000; loop++) { }	//  1000 --> switchingRate = 100 kHz (with my PC).
				  //for ( loop=0; loop<10000; loop++) { }	// 10000 --> switchingRate = ~28 kHz (with my PC).
				  data = ~data;
			  }

	  // end toggling.
	  time_spent = get_time_elapsed( start_time );	// in milliseconds.
	  printf ("done \n");

	  // calculate and print DO switching rate.
	  switchingRate =  (float) times / (float) time_spent;	// in kHz
	  printf ("\ttime_spent = %d (ms) to toggle %li times -- Detected switching rate = %f kHz \n", time_spent, times, switchingRate );

	  // print final DO value.
	  data = ~data;
	  switch (bank)
	  {
		  case 0:
			  printf ("Final outputs:\n"
					  "15                0   (DIO #)\n");
			  break;
		  case 1:
			  printf ("Final outputs:\n"
					  "31               16   (DIO #)\n");
			  break;
		  case 2:
			  printf ("Final outputs:\n"
					  "47               32   (DIO #)\n");
			  break;
		  default:
			  printf ("Invalid group value = %d\n", bank );
			  return;
	  }
	  HexBin (data);		//convert data to binary & print it
	  printf (" (all +%dV)\n", (data==0?5:0) );

	  // re-print option.
	  printf ("Enter DO Group # and toggling times [0-2 1-%i, q] : ", max);
  }

}


////////////////////////////////////////////////////////////////////
// Test 626 performance -- Digital input throughput .
////////////////////////////////////////////////////////////////////

void MeasureDI( DWORD board )
{
  const int 		min		= 200;		// min times of read-in, make it big enough to get time_spend > 1 ms.
  const int 		max		= 1000000;	// max times of read-in.
  WORD			*data;			// holds the values read from digital input.
  long int		i, times;
  long int		psamples;			// for printing read-in samples.

  int			bank = 0;			// default, point to groug #0 -- channels 0-15.
  unsigned long		start_time;
  unsigned long		time_spent;
  float			readinRate;

  printf ("Enter DI Group# and read-in times [0-2 %i-%i, q] : ", min, max );
  data = (WORD *) malloc( max * sizeof(WORD));
  for (;;)
  {
	  fgets (cmdbuff, MAXBUF, stdin);
	  if (cmdbuff[0] == 'q') {
	    free(data);
	    return;
	  }
	  sscanf( cmdbuff, "%i %li", &bank, &times );
	  if ( times<min || times>max ) 
	  {
		  printf ("Enter DI Group# and read-in times [0-2 %i-%i, q] : ", min, max );
		  continue;
	  }
	  if ( bank<0 || bank >2 ) 
	  {
		  printf ("Enter DI Group# and read-in times [0-2 %i-%i, q] : ", min, max );
		  continue;
	  }

	  // get ready for read-in and print running message. 
	  switch (bank)
	  {
		  case 0:
			  S626_DIOWriteBankSet (board, bank, 0);	//channels  0-15 ready for input (pulled up)
			  printf ("DO group #0 (channels 0-15): reading ..... ");	fflush(stdout);
			  break;
		  case 1:
			  S626_DIOWriteBankSet (board, bank, 0);	//channels 16-31 ready for input (pulled up)
			  printf ("DO group #1 (channels 16-31): reading ..... ");	fflush(stdout);
			  break;
		  case 2:
			  S626_DIOWriteBankSet (board, bank, 0);	//channels 32-47 ready for input (pulled up)
			  printf ("DO group #2 (channels 32-47): reading ..... ");	fflush(stdout);
			  break;
		  default:
			  printf ("Invalid group value = %d\n", bank );
			  free(data);
			  return;
	  }

	  // start reading.
	  start_time = get_time_now();	// in milliseconds since the Epoch (00:00:00 UTC, January 1, 1970).

	  // reading:
	  for ( i=0; i<times; i++ )
	  {
		  data[i] = S626_DIOReadBank (board, bank);	//read-in from the group.
	  }

	  // end reading.
	  time_spent = get_time_elapsed( start_time );		// in milliseconds.
	  printf ("done \n");

	  // calculate and print DI read-in rate.
	  readinRate =  (float) times / (float) time_spent;	// in kHz
	  printf ("\ttime_spent = %d (ms) for read-in %li times -- Detected read-in rate = %f kHz \n", time_spent, times, readinRate );

	  // print DI values.
	  printf ("print read-in data: How many last samples? [1-times, q - quit ? ]: ");
	  fgets (cmdbuff, MAXBUF, stdin);
	  psamples = atoi(cmdbuff);
	  if ( cmdbuff[0] == 'q')
	  {
		  printf ("Enter DI read-in times and Group# from which you want to read [1-%i 0-2, q] : ", max );
		  continue;
	  }
	  if ( psamples<1 || psamples>times )
	  {
		  printf ("Enter DI read-in times and Group# from which you want to read [1-%i 0-2, q] : ", max );
		  continue;
	  }
	  switch (bank)
	  {
		  case 0:
			  printf ("last %i read-in value:\n"
					  "15                0     (DIO #)\n", psamples );
			  break;
		  case 1:
			  printf ("last %i read-in value:\n"
					  "31               16     (DIO #)\n", psamples );
			  break;
		  case 2:
			  printf ("last %i read-in value:\n"
					  "47               32     (DIO #)\n", psamples );
			  break;
		  default:
			  printf ("Invalid group value = %d\n", bank );
			  free(data);
			  return;
	  }
	  for ( i=0; i<psamples; i++ )
	  {
		  if ( (times-1-psamples+i)>=0 )
		  {
			  HexBin( data[times-1-psamples+i] );		//convert last read-in data to binary & print it
			  printf(" (0-+5V, 1-0V)\n");
		  }
	  }

	  // re-print option.
	  printf ("Enter DI Group# and read-in times [0-2 %i-%i, q] : ", min, max );
  }
  // returns earlier from loop
  //  free(data);
  
}


////////////////////////////////////////////////////////////////////
// Test 626 performance -- DAC throughput .
////////////////////////////////////////////////////////////////////

void TestDAC( DWORD board )
{
  const long int 	max	= 1000000000;	// max ramping times.
  long int			cnt, times;
  // changed from WORD to unsigned long.  sccanf causing stack smashing with WORD
  unsigned long			chan;  
  int				i, outLast = 0;
  //  long int			loop;

  unsigned long		start_time;
  unsigned long		time_spent;
  float				aoutRate;

  printf ("Enter DAC channel # and ramping outputs [0-3 1-%ld, q] : ", max );

  for (;;)
  {
	  fgets (cmdbuff, MAXBUF, stdin);
	  if (cmdbuff[0] == 'q') return;
	  //sscanf( cmdbuff, "%i %ld", &chan, &times );
	  sscanf( cmdbuff, "%ld %ld", &chan, &times );
	  if ( chan>3 ) 
	  {
		  printf ("Enter DAC channel # and ramping outputs [0-3 1-%li, q] : ", max );
		  continue;
	  }
	  if ( times<1 || times>max ) 
	  {
		  printf ("Enter DAC channel # and ramping outputs [0-3 1-%li, q] : ", max );
		  continue;
	  }

	  // print running message.
	  printf ("DAC channel #%i ramping (-10V_/+10V) ..... ", chan );	
	  fflush(stdout);

	  // start ramping.
	  start_time = get_time_now();	// in milliseconds since the Epoch (00:00:00 UTC, January 1, 1970).

	  // DAC ramping: -10V _/ +10V.
	  for ( cnt=0, i=-8190; cnt<times; cnt++ )	
	  {
	    S626_WriteDAC (board, chan, i);
		  //usleep(100000);		// tested: aoutRate =  ~ 9.1Hz (with my PC).
		  //usleep(20000);		// tested: aoutRate =  ~33.3Hz
		  //usleep(10000);		// tested: aoutRate =  ~50Hz
		  //usleep(1);			// tested: aoutRate =  ~50Hz
		  //usleep(0);			// tested: aoutRate = ~100Hz
		  //for ( loop=0; loop<1; loop++) { }			//      0,1 --> aoutRate =  ~25 kHz (with my PC, max).
		  //for ( loop=0; loop<10000; loop++) { }		//    10000 --> aoutRate =  ~15 kHz (with my PC).
		  //for ( loop=0; loop<100000; loop++) { }		//   100000 --> aoutRate = ~3.3 kHz (with my PC).
		  //for ( loop=0; loop<1000000; loop++) { }		//  1000000 --> aoutRate = ~372 Hz (with my PC).	v
		  //for ( loop=0; loop<10000000; loop++) { }	// 10000000 --> aoutRate =  ~36 Hz (with my PC).	v
		  outLast = i;
		  i = i+818;					// +1.0V  
		  if (++i > 8190) i = -8190;	// trim back to -10.0V
	  }

	  // end ramping.
	  time_spent = get_time_elapsed( start_time );	// in milliseconds.
	  printf ("done \n");

	  // calculate and print DI read-in rate.
	  aoutRate =  (float) times / (float) time_spent;	// in kHz
	  printf ("\ttime_spent = %d (ms) for % i analog outputs -- Detected analog-out rate = %f kHz \n", time_spent, times, aoutRate );

	  // last DAC output
	  printf ("\tChannel #%d :  DAC output [%d] = %6i ( %f V ) \n", chan, cnt, outLast, 10.0*(float)(outLast)/8191.0);

	  // re-print option.
	  printf ("Enter DAC channel # and ramping outputs [0-3 1-%li, q] : ", max );
  }


}


//*************************** Scattered Acquisition ***************************
//*  Digitize all ADC channels on a given board.
//*****************************************************************************/

#define RANGE_10V		0x00		// Range code for ADC 10V range.
#define RANGE_5V		0x10		// Range code for ADC 5V range.
#define EOPL			0x80		// ADC end-of-poll-list marker.
#define CHANMASK		0x0F		// ADC channel number mask.

void ScatteredAcq( DWORD board )
{
	unsigned long		start_time;
	unsigned long		time_spent;
	int					i, j;
	int					turns = 1000;

	// Allocate data structures. We allocate enough space for maximum possible
	// number of items (16) even though this example only has 3 items. Although
	// this is not required, it is the recommended practice to prevent programming
	// errors if your application ever modifies the number of items in the poll list.
	BYTE				poll_list[16];		// List of items to be digitized.
	SHORT				databuf[16];		// Buffer to receive digitized data.

	// Populate the poll list.
	poll_list[0]  =  0 | RANGE_10V;			// Chan 0, 10V range.
	poll_list[1]  =  1 | RANGE_10V;			// Chan 1, 10V range.
	poll_list[2]  =  2 | RANGE_10V;			// Chan 2, 10V range.
	poll_list[3]  =  3 | RANGE_10V;			// Chan 3, 10V range.
	poll_list[4]  =  4 | RANGE_10V;			// Chan 4, 10V range.
	poll_list[5]  =  5 | RANGE_10V;			// Chan 5, 10V range.
	poll_list[6]  =  6 | RANGE_10V;			// Chan 6, 10V range.
	poll_list[7]  =  7 | RANGE_10V;			// Chan 7, 10V range.
	poll_list[8]  =  8 | RANGE_5V;			// Chan 8,  5V range.
	poll_list[9]  =  9 | RANGE_5V;			// Chan 9,  5V range.
	poll_list[10] = 10 | RANGE_5V;			// Chan 10, 5V range.
	poll_list[11] = 11 | RANGE_5V;			// Chan 11, 5V range.
	poll_list[12] = 12 | RANGE_5V;			// Chan 12, 5V range.
	poll_list[13] = 13 | RANGE_5V;			// Chan 13, 5V range.
	poll_list[14] = 14 | RANGE_5V;			// Chan 14, 5V range.
	poll_list[15] = 15 | RANGE_5V | EOPL;	// Chan 15, 5V range, mark as list end.

	// Prepare for A/D conversions by passing the poll list to the driver.
	S626_ResetADC( board, poll_list );

	// Digitize all items in the poll list. As long as the poll list is not modified,
	// S626_ReadADC() can be called repeatedly without calling S626_ResetADC() again.
	// This could be implemented as two calls: S626_StartADC() and S626_WaitDoneADC().
	start_time = get_time_now();	// in milliseconds since the Epoch (00:00:00 UTC, January 1, 1970).
	for ( j=1; j<=turns; j++ )
		S626_ReadADC( board, databuf );
	time_spent = get_time_elapsed( start_time );	// in milliseconds.
	printf ("Average time_spent = %f (ms) for Scattered Acquisition from all ADC channels: \n", (float)time_spent / (float)turns );

	// Display all digitized binary data values.
	for ( i=0; i<=7; i++ )
		printf( "\tChannel %d = %d ( %f V ) \n", poll_list[i] & CHANMASK, databuf[i], (float)((short)databuf[i])/3276.7 );
	for ( i=8; i<=15; i++ )
		printf( "\tChannel %d = %d ( %f V ) \n", poll_list[i] & CHANMASK, databuf[i], (float)((short)databuf[i])/6553.4 );

}


//****************************** DIO Interrupts *******************************
//*  Count the interrupts occurring on a given board, from DIO channels 0-15.
//*****************************************************************************/

void IntDIO( DWORD board )
{
	time_t			time_now;
	DWORD			counts[16];			// Cached copy of interrupt counts.
	WORD			group		= 0;		// Set: group=0 => DIO channels 0-15.
	WORD			chans		= 0xFFFF;	// Set bit flags for all channels 0-15.
	WORD			enables		= 0xFFFF;	// Set bit flags for all channels 0-15.
	const int		max		= 72000;	// 20 hours.
	int				seconds;
	int				i, j;

	// Get how long you are going to monitor.
	printf ("Monitoring how many seconds [1-%i, q] : ", max );
	for (;;)
	{
		fgets (cmdbuff, MAXBUF, stdin);
		if (cmdbuff[0] == 'q') return;
		sscanf( cmdbuff, "%i", &seconds );
		if ( seconds<1 || seconds>max )
		{
			printf ("Monitoring how many seconds [1-%i, q] : ", max );
			continue;
		}
		break;
	}
	//printf("seconds = %i \n", seconds );

	// Reset all DIO interrupt counters to zero.
	memcpy( IntCounts, zeros, sizeof( zeros ) );

	// Enable event capturing and interrupts on DIO channels 0-15.
	S626_DIOCapEnableSet( board, group, chans, TRUE );
	S626_DIOIntEnableSet( board, group, enables );

	// Enable board master interrupt.
	S626_InterruptEnable( board, TRUE );

	// Repeat for the given seconds ...
	for ( j = 0; j < seconds; j++ )
	{
		// Suspend main thread for one second.
		sleep( 1 );

		// Copy, and then reset the interrupt counts. This is a critical section because
		// interrupt counters are shared by both the main thread and the interrupt thread.
		pthread_mutex_lock(&CriticalSection);		// * Start thread-safe section ------
		memcpy( counts, IntCounts, sizeof( counts ) );	// * Cache a copy of counters.
		// * when you want to measure frequency of all pulse signal inputed from DIO 0-15, 
		// * un-comment out next line: (good for <3kHz, with my PC)
		memcpy( IntCounts, zeros, sizeof( zeros ) );	// * Reset counters to zero, every one second,
		pthread_mutex_unlock(&CriticalSection);		// * Start thread-safe section ------

		// Display cached interrupt counts.
		time(&time_now);
		printf( "\n--- INTERRUPT COUNTS: second-%i --- at %s", j+1, ctime(&time_now) );
		for ( i = 0; i < 8; i++ )
			printf( "DIO %d = %d\t", i, counts[i] );
		printf("\n");
		for ( i = 8; i < 16; i++ )
			printf( "DIO %d = %d\t", i, counts[i] );
		printf("\n");
	}

	// Disable interrupts when finished the test
	S626_InterruptEnable (board, FALSE);

	printf( "\nDone with testing DIO Interrupts\n" );

}


//******************** Toggle the relay on board ********************
//*  Toggling Relay on/off, at 1/2 Hz rate.
//*******************************************************************

void ToggleRL( int brd )
{
	int		i, times;
	int		max = 36000;	// 20 hours, at 1/2 Hz rate.
	int		data;

	// Get how many times you are going to toggle relay on/off.
	printf ("Toggling relay on/off how many times [1-%i, q] : ", max );
	for (;;)
	{
		fgets (cmdbuff, MAXBUF, stdin);
		if (cmdbuff[0] == 'q') return;
		sscanf( cmdbuff, "%i", &times );
		if ( times<1 || times>max )
		{
			printf ("Toggling relay on/off how many times [1-%i, q] : ", max );
			continue;
		}
		break;
	}

	// Pre-set watchdog timeout period.
	S626_WatchdogPeriodSet (brd, 0);	// 0 -- 1/8 second period,
						// 1 -- 1/2 second period,
						// 2 --   1 second period.
	// Toggling relay on/off:
	printf("\tBoard %d - Relay ON/OFF toggling ...", brd );
	for ( i=1; i<=times; i++ )
	{
		// Turn relay on, with enabling the watchdog.
		S626_WatchdogEnableSet (brd, TRUE);
		printf("(%i)..on",i); fflush(stdout);
		usleep(1125000);	// for 1/2 Hz: 1.0s + 0.125s (timeout period, when set to 1/8)
		// Turn relay off, with disabling the watchdog.
		S626_WatchdogEnableSet (brd, FALSE);
		printf("/off.."); fflush(stdout);
		usleep(875000);		// for 1/2 Hz: 1.0s - 0.125s (timeout period, when set to 1/8)
	}
	printf(". Done.\n");

}


////////////////////////////////////////////////////////////////////
// Configure a counter xA/xB for Encoder Counter.
////////////////////////////////////////////////////////////////////

VOID CreateEncoderCounter( HBD hbd, WORD Counter )
{
	// Set operating mode for the given Counter.
	S626_CounterModeSet( hbd, Counter,
		( LOADSRC_INDX << BF_LOADSRC ) |    // Preload upon index.
		( INDXSRC_SOFT << BF_INDXSRC ) |    // Disable hardware index.
		( CLKSRC_COUNTER << BF_CLKSRC ) |   // Operating mode is Counter.
		( CLKPOL_POS  << BF_CLKPOL ) |      // Active high clock.
		//( CNTDIR_UP  << BF_CLKPOL ) |       // Count direction is Down.
		( CLKMULT_1X   << BF_CLKMULT ) |    // Clock multiplier is 1x.
		( CLKENAB_INDEX << BF_CLKENAB ) );  // Counting is initially disabled.

	// Set counter core and preload value to be mid of 2^24 (since all counters are 24-bit), so as to make test easy.
	S626_CounterPreload( hbd, Counter, 8388608 );	// 0x800000 = 2^24/2 = 8388608
	S626_CounterSoftIndex( hbd, Counter );		// Generate a index signal by software, 
							// so that the preload value got loaded when creating EncoderCounter.

	// Enable latching of accumulated counts on demand.
	S626_CounterLatchSourceSet( hbd, Counter, LATCHSRC_AB_READ );

	// Enable the counter.
	S626_CounterEnableSet( hbd, Counter, CLKENAB_ALWAYS );

}

//--------------------------------------------------------------------
// Simulate Quadrature Encoder: forward one (to be implemented)
//--------------------------------------------------------------------

VOID ForwardOne( HBD hbd, WORD Counter )
{
	//int		hdus = 0;	// with linux, hold 10ms.
	int		bank;
	WORD	data;	// ---------------------------Wiring on 626 TestBoard---------------------------------
					// DIO:  15  14  13  12  11  10  09  08  07  06 | 05  04  03  02  01  00
					//         (CNTR_1A/1B)  I-  I+  B-  B+  A-  A+ | I-  I+  A-  A+  B-  B+  (CNTR_0A/0B)
					// -----------------------------------------------------------------------------------
					// DIO:  31  30  29  28  27  26  25  24  23  22 | 21  20  19  18  17  16
					//                                                I-  I+  A-  A+  B-  B+  (CNTR_2A/2B)
					// -----------------------------------------------------------------------------------

	if ( Counter==CNTR_0A || Counter==CNTR_0B )
	{
		bank = 0;		// channels 15-0
		//usleep(hdus);
		data = 0x29;	//	A	B	I
						//	0	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x1a;	//	A	B	I
						//	0	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x16;	//	A	B	I
						//	1	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x25;	//	A	B	I
						//	1	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
	}

	if ( Counter==CNTR_2A || Counter==CNTR_2B )
	{
		bank = 1;		// channels 31-16
		//usleep(hdus);
		data = 0x29;	//	A	B	I
						//	0	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x1a;	//	A	B	I
						//	0	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x16;	//	A	B	I
						//	1	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x25;	//	A	B	I
						//	1	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
	}

	if ( Counter==CNTR_1A || Counter==CNTR_1B )
	{
		bank = 0;		// channels 15-0
		//usleep(hdus);
		data = 0x980;	//	A	B	I
						//	0	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x680;	//	A	B	I
						//	0	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x640;	//	A	B	I
						//	1	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x940;	//	A	B	I
						//	1	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
	}

}

//--------------------------------------------------------------------
// Simulate Quadrature Encoder: reverse one (to be implemented)
//--------------------------------------------------------------------

VOID ReverseOne( HBD hbd, WORD Counter )
{
	//int		hdus = 0;	// with linux, hold 10ms.
	int		bank;
	WORD	data;	// ---------------------------Wiring on 626 TestBoard---------------------------------
					// DIO:  15  14  13  12  11  10  09  08  07  06 | 05  04  03  02  01  00
					//         (CNTR_1A/1B)  I-  I+  B-  B+  A-  A+ | I-  I+  A-  A+  B-  B+  (CNTR_0A/0B)
					// --------------------------------------------------------------------------------
					// DIO:  31  30  29  28  27  26  25  24  23  22 | 21  20  19  18  17  16
					//                                                I-  I+  A-  A+  B-  B+  (CNTR_2A/2B)
					// --------------------------------------------------------------------------------

	if ( Counter==CNTR_0A || Counter==CNTR_0B )
	{
		bank = 0;		// channels 15-0
		//usleep(hdus);
		data = 0x25;	//	A	B	I
						//	1	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x16;	//	A	B	I
						//	1	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x1a;	//	A	B	I
						//	0	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x29;	//	A	B	I
						//	0	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
	}

	if ( Counter==CNTR_2A || Counter==CNTR_2B )
	{
		bank = 1;		// channels 31-16
		//usleep(hdus);
		data = 0x25;	//	A	B	I
						//	1	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x16;	//	A	B	I
						//	1	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x1a;	//	A	B	I
						//	0	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x29;	//	A	B	I
						//	0	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
	}

	if ( Counter==CNTR_1A || Counter==CNTR_1B )
	{
		bank = 0;		// channels 15-0
		//usleep(hdus);
		data = 0x940;	//	A	B	I
						//	1	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x640;	//	A	B	I
						//	1	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x680;	//	A	B	I
						//	0	0	1
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
		data = 0x980;	//	A	B	I
						//	0	1	0
		S626_DIOWriteBankSet (hbd, bank, data);
		//usleep(hdus);
	}

}


//****************************** Encoder Test *******************************
//*  Encoder test:  Encoder Test with Quadrature Encoder Simulator 
//*  Sets up a Counter xA/xB as a counter for encoder test.
//***************************************************************************/

void EncoderTest( DWORD board )
{
	WORD            Counter = CNTR_0A;	// CNTR_0A:	inputs on J5: CNTR_0A - A-,A+,B-,B+,I-,I+
						// CNTR_1A:	inputs on J5: CNTR_1A - A-,A+,B-,B+,I-,I+
						// CNTR_2A:	inputs on J5: CNTR_2A - A-,A+,B-,B+,I-,I+ 
						// CNTR_0B:	inputs on J4: CNTR_0B - A-,A+,B-,B+,I-,I+ 
						// CNTR_1B:	inputs on J4: CNTR_1B - A-,A+,B-,B+,I-,I+
						// CNTR_2B:	inputs on J4: CNTR_2B - A-,A+,B-,B+,I-,I+ 
	int		i, steps;
	char		cmd[10];
	int		max = 1000000000;
	time_t		time_now;

	// Get counter number for encoder test:
	printf ("Enter counter number for encoder test (0-0A,1-1A,2-2A,3-0B,4-1B,5-2B) [0-5, q] : ");
	for (;;)
	{
		fgets (cmdbuff, MAXBUF, stdin);
		if (cmdbuff[0] == 'q') return;
		sscanf( cmdbuff, "%d", &Counter );
		if ( Counter>CNTR_2B )
		{
			printf ("Enter counter number for encoder test (0-0A,1-1A,2-2A,3-0B,4-1B,5-2B) [0-5, q] : ");
			continue;
		}
		break;
	}
    
	// Reset Counter
	S626_CounterCapFlagsReset( board, Counter );

	// Configure counter xA/xB as a encoder counter.
	CreateEncoderCounter( board, Counter );

	for ( ; ; )
	{
		printf ("Forward/Reverse how many steps with Quadrature Encoder Simulator [F/R 1-%i, q] : ", max );
		fgets (cmdbuff, MAXBUF, stdin);
		if (cmdbuff[0] == 'q') return;
		sscanf( cmdbuff, "%s %d", cmd, &steps );
		if ( !( cmd[0]=='F' || cmd[0]=='f' ||
				cmd[0]=='R' || cmd[0]=='r' || cmd[0]=='B' || cmd[0]=='b' ) ) continue;
		if ( steps<1 || steps>max ) continue;
		// forwarding / reversing:
		if ( cmd[0]=='F' || cmd[0]=='f' ) 
		{
			time(&time_now);
			printf ("\tStarted at %s", ctime(&time_now));
			printf( "\tForwarding ..... ", steps );	fflush(stdout);
			for ( i=1; i<=steps; i++ ) ForwardOne( board, Counter );
			printf( "Forwarded %i steps.\n", steps );
		} else if ( cmd[0]=='R' || cmd[0]=='r' || cmd[0]=='B' || cmd[0]=='b') 
		{
			time(&time_now);
			printf ("\tStarted at %s", ctime(&time_now));
			printf( "\tReversing ..... ", steps );	fflush(stdout);
			for ( i=1; i<=steps; i++ ) ReverseOne( board, Counter );
			printf( "Reversed %i steps.\n", steps );
		} else return;	// never reached here
		// read and print counting:
		printf( "\tCurrent counting of Counter #%i = %d ", Counter, S626_CounterReadLatch(board,Counter)-8388608 );
		time(&time_now);
		printf ("at %s", ctime(&time_now));
	}

}


//**************************** Quadrature Test *****************************
//*  Quadrature Encoder test:  test real Quadrature Encoder  
//*  Sets up a Counter xA/xB as a counter for quadrature encoder test.
//**************************************************************************/

void QuadratureTest( DWORD board )
{
	WORD            Counter = CNTR_0A;	// CNTR_0A:	inputs on J5: CNTR_0A - A-,A+,B-,B+,I-,I+
						// CNTR_1A:	inputs on J5: CNTR_1A - A-,A+,B-,B+,I-,I+
						// CNTR_2A:	inputs on J5: CNTR_2A - A-,A+,B-,B+,I-,I+ 
						// CNTR_0B:	inputs on J4: CNTR_0B - A-,A+,B-,B+,I-,I+ 
						// CNTR_1B:	inputs on J4: CNTR_1B - A-,A+,B-,B+,I-,I+
						// CNTR_2B:	inputs on J4: CNTR_2B - A-,A+,B-,B+,I-,I+ 
	int		i, seconds;
	char		cmd[10];
	int		max = 1000000000;
	time_t		time_now;

	// Get counter number for quadrature encoder test:
	printf ("Enter counter number that will be used for quadrature encoder test (0-0A,1-1A,2-2A,3-0B,4-1B,5-2B) [0-5, q] : ");
	for (;;)
	{
		fgets (cmdbuff, MAXBUF, stdin);
		if (cmdbuff[0] == 'q') return;
		sscanf( cmdbuff, "%d", &Counter );
		if ( Counter>CNTR_2B )
		{
			printf ("Enter counter number that will be used for quadrature encoder test (0-0A,1-1A,2-2A,3-0B,4-1B,5-2B) [0-5, q] : ");
			continue;
		}
		break;
	}
    
	// Reset Counter
	S626_CounterCapFlagsReset( board, Counter );

	// Configure counter xA/xB as a encoder counter.
	CreateEncoderCounter( board, Counter );

	for ( ; ; )
	{
		printf ("How many seconds you are going to test/monitor [1-%i, q] : ", max );
		fgets (cmdbuff, MAXBUF, stdin);
		if (cmdbuff[0] == 'q') return;
		sscanf( cmdbuff, "%d", &seconds );
		if ( seconds<1 || seconds>max ) continue;
		// read and print counting/position for the given seconds:
		for ( i=1; i<=seconds; i++ )
		{
			printf( "\tCurrent position/counting of Counter #%i = %d ", 
				Counter, S626_CounterReadLatch(board,Counter)-8388608 );
			time(&time_now);
			printf ("at %s", ctime(&time_now));
			sleep(1);
		}
	}

}

