#ifndef RANDOM_H
#define RANDOM_H
//
// Derived from the SimpleRandom library - MIT License
// https://github.com/cmcqueen/simplerandom/blob/master/c/lecuyer/lfsr113.c
//

/* 
   32-bits Random number generator U[0,1): lfsr113
   Author: Pierre L'Ecuyer,
   Source: http://www.iro.umontreal.ca/~lecuyer/myftp/papers/tausme2.ps 
   ---------------------------------------------------------
*/
#include <stdint.h>

uint32_t Random_uint32(void);
void Random_Reseed(void);

#endif
