/**
 *  Jordan Brindza
 *  brindza@seas.upenn.edu
 *  University of Pennsylvania
 *  December 2010
 */

#include "haptics_util.h"


void PrintHexAsBin(int data) {
  int i;

  for (i = 0; i < 16; i++) {
    printf((data << i) & 0x8000 ? "1" : "0");
    if (i % 4 == 3)
      printf(" ");
  }
}


unsigned long get_time_now() {
  // Structure to receive current time.
  struct timeval tv;    
  struct timezone tz;

  // Get the current time of day.
  gettimeofday( &tv, &tz );
  
  // Compute and return the elapsed milliseconds since midnight.
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


unsigned long get_time_elapsed(unsigned long start_time) {
  // Compute the elapsed milliseconds since start_time.
  unsigned long t_elapsed = get_time_now() - start_time;
  
  // Return elapsed time, compensating for timer wraparound if necessary.
  return ( ( t_elapsed & 0x10000000 ) != 0 ) ? ( t_elapsed + 86400000 ) : t_elapsed;
}
