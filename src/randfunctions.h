/* An implementation of the Mersenne-Twister pseudo-random number
 * generator and an implementation of Knuth's LCG random number
 * generator.  This one is good enough for the skip lists, which is
 * what it is intended for.
 *
 * The MT code was adapted from the code covered by the following
 * license.  The main modification was to convert several of the
 * functions to inline functions and statically initialize the state
 * (the equivalent state if seeded with 0).  This provides enough
 * randomness for our purposes.
 */

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.
   
   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

#ifndef _RANDBASIC_H
#define _RANDBASIC_H

#include "optimizations.h"
#include <stdint.h>
#include <stdlib.h>

/* Period parameters */  
#define __MTRAND_N 624

typedef struct {
    unsigned int mti;
    unsigned long mt[__MTRAND_N];
} MTRandState;

static inline MTRandState* Mtr_New(uint32_t seed);
static inline uint32_t Mtr_Next(MTRandState*);

void _Mtr_next_state(MTRandState*);

typedef uint64_t LCGState;
    
/* Note that the state is returned by value.  This is for pure, brute
 * speed. */
static inline LCGState Lcg_New(uint32_t seed);
static inline uint32_t Lcg_Next(LCGState* state);

#include "randfunctions_inline.h"

#endif
