#include "randfunctions.h"

/* See header file for specific details.

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

#define __MTRAND_M 397
#define __MTRAND_MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define __MTRAND_UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define __MTRAND_LOWER_MASK 0x7fffffffUL /* least significant r bits */

/* generates a random number on [0,0xffffffff]-interval */

void _Mtr_next_state(MTRandState* msi)
{
    const unsigned long mag01[2]={0x0UL, __MTRAND_MATRIX_A};
    /* mag01[x] = x * __MTRAND_MATRIX_A  for x=0,1 */

    unsigned long* mt = msi->mt;
    int kk;
    unsigned long y;
    /* generate __MTRAND_N words at one time */

    for (kk=0;kk<__MTRAND_N-__MTRAND_M;kk++) 
    {
	y = (mt[kk] & __MTRAND_UPPER_MASK)|(mt[kk+1]&__MTRAND_LOWER_MASK);
	mt[kk] = mt[kk+__MTRAND_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
        
    for (;kk<__MTRAND_N-1;kk++) 
    {
	y = (mt[kk]&__MTRAND_UPPER_MASK)|(mt[kk+1]&__MTRAND_LOWER_MASK);
	mt[kk] = mt[kk+(__MTRAND_M-__MTRAND_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    
    y = (mt[__MTRAND_N-1]&__MTRAND_UPPER_MASK)|(mt[0]&__MTRAND_LOWER_MASK);
    mt[__MTRAND_N-1] = mt[__MTRAND_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
	
    msi->mti = 0;
}


/* The following code from the original library isn't needed for our
 * purposes and so is commented out. */


/* /\* generates a random number on [0,0x7fffffff]-interval *\/ */
/* long genrand_int31(void) */
/* { */
/*     return (long)(genrand_int32()>>1); */
/* } */

/* /\* generates a random number on [0,1]-real-interval *\/ */
/* double genrand_real1(void) */
/* { */
/*     return genrand_int32()*(1.0/4294967295.0);  */
/*     /\* divided by 2^32-1 *\/  */
/* } */

/* /\* generates a random number on [0,1)-real-interval *\/ */
/* double genrand_real2(void) */
/* { */
/*     return genrand_int32()*(1.0/4294967296.0);  */
/*     /\* divided by 2^32 *\/ */
/* } */

/* /\* generates a random number on (0,1)-real-interval *\/ */
/* double genrand_real3(void) */
/* { */
/*     return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0);  */
/*     /\* divided by 2^32 *\/ */
/* } */

/* /\* generates a random number on [0,1) with 53-bit resolution*\/ */
/* double genrand_res53(void)  */
/* {  */
/*     unsigned long a=genrand_int32()>>5, b=genrand_int32()>>6;  */
/*     return(a*67108864.0+b)*(1.0/9007199254740992.0);  */
/* }  */
/* /\* These real versions are due to Isaku Wada, 2002/01/09 added *\/ */

/* /\* This main() outputs 2000 generated integers and reals.  *\/ */
/* /\* */
/*   int main(void) */
/* { */
/* int i; */
/*     unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4; */
/*     init_by_array(init, length); */
/*     printf("1000 outputs of genrand_int32()\n"); */
/*     for (i=0; i<1000; i++) { */
/*       printf("%10lu ", genrand_int32()); */
/*       if (i%5==4) printf("\n"); */
/*     } */
/*     printf("\n1000 outputs of genrand_real2()\n"); */
/*     for (i=0; i<1000; i++) { */
/*     printf("%10.8f ", genrand_real2()); */
/*       if (i%5==4) printf("\n"); */
/*       } */
/*     return 0; */
/*     } */
/* *\/ */
