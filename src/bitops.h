/********************************************************************************
 * 
 * A collection of macros and inline functions that make operations on
 * individual bits within a higher level type easier.
 *
 ********************************************************************************/

#ifndef BITOPS_H
#define BITOPS_H

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "brg_endian.h"
#include "optimizations.h"
#include "debugging.h"

typedef unsigned long int bitfield;

#define bitsizeof(t) (8*sizeof(t))

#define LeftShift(x, s)  (( (bitfield)(x) ) << (s))
#define RightShift(x, s) (( (bitfield)(x) ) >> (s))

#define _BITMASK(n)  ( (((bitfield)1) << (n)) - 1)
    
static inline bool bitOn(bitfield bf, unsigned int bit)
{
    return !!(bf & LeftShift(1, bit));
}

static inline bool bitOff(bitfield bf, unsigned int bit)
{
    return (bf & LeftShift(1, bit)) == 0;
}

#define firstnBitsOn(bf, top_bit)			\
  ( (((bitfield)(bf)) & _BITMASK(top_bit)) == _BITMASK(top_bit))
	
/* Operations for setting and changing bitfields. */
#define setBitOn(bf, bit)				\
    do{ (bf) |= (LeftShift(1, (bit))); }while(0)

#define setBitOff(bf, bit)				\
    do{ (bf) &= (~(LeftShift(1, (bit)))); } while(0)

#define flipBit(bf, bit)			        \
    do{ (bf) ^= LeftShift(1, (bit)); } while(0)

#define flipBitToOff(bf, bit)						\
    do{									\
	assert(bitOn( (bf), (bit)));					\
	(bf) -= LeftShift(1, (bit));					\
    } while(0)


static inline int getFirstBitOn(bitfield bf)
{

#ifdef HAVE__builtin_ctzl
    return __builtin_ctzl(bf);
#else

    int c = 0;     // c will be the number of zero bits on the right,
                    // so if v is 1101000 (base 2), then c will be 3

    if (bitsizeof(bitfield) > 32 && (bf & _BITMASK(32)) == 0)
    {
	bf >>= 32;
	c += 32;
    }
    if ((bf & _BITMASK(16)) == 0) 
    {  
	bf >>= 16;  
	c += 16;
    }
    if ((bf & _BITMASK(8)) == 0) 
    {
	bf >>= 8;  
	c += 8;
    }
    if ((bf & _BITMASK(4)) == 0) 
    {  
	bf >>= 4;
	c += 4;
    }
    if ((bf & _BITMASK(2)) == 0)
    {  
	bf >>= 2;
	c += 2;
    }

    c += !(bf & 0x1);
    
    return c;
#endif    
}

static inline int getFirstBitOff(bitfield bf)
{
    return getFirstBitOn(~bf);
}

static inline int bitwise_log2(unsigned long bf)
{

#ifdef HAVE__builtin_clzl
    return bitsizeof(unsigned long) - __builtin_clzl(bf);
#else
    return (size_t)(floor(log2(bf)));
#endif
}

#endif 
