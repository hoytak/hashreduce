/********************************************************************************
 * 
 * A collection of macros and inline functions that make operations on
 * individual bits within a higher level type easier.
 *
 ********************************************************************************/

#ifndef BITOPS_H
#define BITOPS_H

#include <stdint.h>
#include "brg_endian.h"

typedef unsigned long int bitfield;

static const bitfield _bit_masks[33] = {
    0x00000000,
    0x00000001,  0x00000003, 0x00000007, 0x0000000F,
    0x0000001F,  0x0000003F, 0x0000007F, 0x000000FF,
    0x000001FF,  0x000003FF, 0x000007FF, 0x00000FFF,
    0x00001FFF,  0x00003FFF, 0x00007FFF, 0x0000FFFF,
    0x0001FFFF,  0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
    0x001FFFFF,  0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
    0x01FFFFFF,  0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
    0x1FFFFFFF,  0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF};

/* We use this method of bitshifting to avoid warnings for the 64 bit
 * code when on a 32 bit compiler.  The optimizer will collapse it
 * all. */

#define bitsizeof(t) (8*sizeof(t))
#define LeftShift(x, s)  ( ((s) < 32) ? ((x) << (s)) : ( (((x) << 16) << 16) << ((s) - 32)))
#define RightShift(x, s) ( ((s) < 32) ? ((x) >> (s)) : ( (((x) >> 16) >> 16) >> ((s) - 32)))

#define _BITMASK(n)						\
    ( (bitsizeof(bitfield) == 64)					\
      ? ( ( ((n) >= 32) ? (LeftShift(_bit_masks[(n) - 32], 32)) : 0) \
	  + (_bit_masks[(n) & 0x0000001fL]))			\
      : (_bit_masks[(n)]))
    
#define bitOn(bf, bit)               ( ((bf) & LeftShift(1, bit)) != 0)
#define bitOff(bf, bit)              ( ((bf) & LeftShift(1, bit)) == 0)

#define firstnBitsOn(bf, top_bit)			\
    ( ((bf) & _BITMASK(top_bit)) == _BITMASK(top_bit))

#define clearBits(bf, bit_offset, n_bits)				\
    (bf) &= (~(_BITMASK((bit_offset) + (n_bits)) - _BITMASK(bit_offset)));

	
/* Operations for setting and changing bitfields. */

#define setBitOn(bf, bit)			\
    { (bf) |= (LeftShift(1, (bit))); }

#define setBitOff(bf, bit)			\
    { (bf) &= (~(LeftShift(1, (bit)))); }

#define setBit(bf, bit, bit_on)						\
    { if(bit_on) setBitOn((bf), (bit)); else setBitOff((bf), (bit));}

#define setBitGroup(bf, bit_offset, n_bits, bit_values)			\
    {									\
	clearBits((bf), (bit_offset), (n_bits));			\
	(bf) |= ( LeftShift((bit_values) & _BITMASK((n_bits)), bit_offset)); \
    }

#define getBitGroup(bf, bit_offset, n_bits)			\
    ( RightShift((bf), (bit_offset)) & _BITMASK((n_bits)))

static inline unsigned int b64low(uint64_t v)
{
    union {
	uint64_t v64;
	uint32_t v32[2];
    } splitter;

    splitter.v64 = v;

# if(PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    return splitter.v32[0];
# else
    return splitter.v32[1];
#endif

}

static inline unsigned int b64high(uint64_t v)
{
    union {
	uint64_t v64;
	uint32_t v32[2];
    } splitter;

    splitter.v64 = v;
# if(PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    return splitter.v32[1];
# else
    return splitter.v32[0];
#endif
}


static inline int getFirstBitOn(bitfield bf)
{
    int i;
    for(i = 0; i < bitsizeof(bitfield); ++i)
	if(bf & LeftShift(1, i)) 
	    return i;

    return -1;
}

static inline int getFirstBitOff(bitfield bf)
{
    return getFirstBitOn(~bf);
}

static inline int getFirstBitOnFromPos(bitfield bf, int first_possible_bit)
{
    int i;

    for(i = first_possible_bit; i < bitsizeof(bitfield); ++i)
    {
	if(bf & LeftShift(1, i))
	    return i;
    }
    
    return -1;
}

#endif 
