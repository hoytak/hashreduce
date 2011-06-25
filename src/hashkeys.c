#include "hashobject.h"
#include "randfunctions.h"
#include "bitops.h"
#include <string.h>
#include <stdlib.h>  // for size_t.
#include <stdint.h>

/* The following notice applies to the CityHash code. */

/* Copyright (c) 2011 Google, Inc. */

/* Permission is hereby granted, free of charge, to any person obtaining a copy */
/* of this software and associated documentation files (the "Software"), to deal */
/* in the Software without restriction, including without limitation the rights */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell */
/* copies of the Software, and to permit persons to whom the Software is */
/* furnished to do so, subject to the following conditions: */

/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software. */

/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN */
/* THE SOFTWARE. */

/* CityHash Version 1, by Geoff Pike and Jyrki Alakuijala */

/* This file provides CityHash64() and related functions. */

/* It's probably possible to create even faster hash functions by */
/* writing a program that systematically explores some of the space of */
/* possible hash functions, by using SIMD instructions, or by */
/* compromising on hash quality. */

/* Functions in the CityHash family are not suitable for cryptography. */

/* WARNING: This code has not been tested on big-endian platforms! */
/* It is known to work well on little-endian platforms that have a small penalty */
/* for unaligned reads, such as current Intel and AMD moderate-to-high-end CPUs. */

/* By the way, for some hash functions, given strings a and b, the hash */
/* of a+b is easily derived from the hashes of a and b.  This property */
/* doesn't hold for any hash functions in this file. */


#define UNALIGNED_LOAD64(p) (*(const uint64_t*)(p))
#define UNALIGNED_LOAD32(p) (*(const uint32_t*)(p))

#define Uint128Low64(x) ((x).hk64[0])
#define Uint128High64(x) ((x).hk64[1])
 
// Some primes between 2^63 and 2^64 for various uses.
#define k0 (0xc3a5c85c97cb3127ULL)
#define k1 (0xb492b66fbe98f273ULL)
#define k2 (0x9ae16a3b2f90404fULL)
#define k3 (0xc949d7c7509e6557ULL)

// Hash 128 input bits down to 64 bits of output.
// This is intended to be a reasonably good hash function.
static inline uint64_t Hash128to64(uint64_t xl, uint64_t xu) {
  // Murmur-inspired hashing.
  const uint64_t kMul = 0x9ddfea08eb382d69ULL;
  uint64_t a = (xl ^ xu) * kMul;
  a ^= (a >> 47);
  uint64_t b = (xu ^ a) * kMul;
  b ^= (b >> 47);
  b *= kMul;
  return b;
}

// Bitwise right rotate.  Normally this will compile to a single
// instruction, especially if the shift is a manifest constant.
static inline uint64_t Rotate(uint64_t val, int shift) {
  // Avoid shifting by 64: doing so yields an undefined result.
  return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

static inline void Swap(uint64_t *x, uint64_t *y) {
    uint64_t t = *x; 
    *x = *y;
    *y = t;
}

// Equivalent to Rotate(), but requires the second arg to be non-zero.
// On x86-64, and probably others, it's possible for this to compile
// to a single instruction if both args are already in registers.
static inline uint64_t RotateByAtLeast1(uint64_t val, int shift) {
  return (val >> shift) | (val << (64 - shift));
}

static inline uint64_t ShiftMix(uint64_t val) {
  return val ^ (val >> 47);
}

static inline uint64_t HashLen16(uint64_t u, uint64_t v) {
  return Hash128to64(u, v);
}

static inline uint64_t HashLen0to16(const char *s, size_t len) {
  if (len > 8) {
    uint64_t a = UNALIGNED_LOAD64(s);
    uint64_t b = UNALIGNED_LOAD64(s + len - 8);
    return HashLen16(a, RotateByAtLeast1(b + len, len)) ^ b;
  }
  if (len >= 4) {
    uint64_t a = UNALIGNED_LOAD32(s);
    return HashLen16(len + (a << 3), UNALIGNED_LOAD32(s + len - 4));
  }
  if (len > 0) {
    uint8_t a = s[0];
    uint8_t b = s[len >> 1];
    uint8_t c = s[len - 1];
    uint32_t y = ((uint32_t)(a)) + (((uint32_t)(b)) << 8);
    uint32_t z = len + (((uint32_t)(c)) << 2);
    return ShiftMix(y * k2 ^ z * k3) * k2;
  }
  return k2;
}

// This probably works well for 16-byte strings as well, but it may be overkill
// in that case.
static inline uint64_t HashLen17to32(const char *s, size_t len) {
  uint64_t a = UNALIGNED_LOAD64(s) * k1;
  uint64_t b = UNALIGNED_LOAD64(s + 8);
  uint64_t c = UNALIGNED_LOAD64(s + len - 8) * k2;
  uint64_t d = UNALIGNED_LOAD64(s + len - 16) * k0;
  return HashLen16(Rotate(a - b, 43) + Rotate(c, 30) + d,
                   a + Rotate(b ^ k3, 20) - c + len);
}

// Return a 16-byte hash for 48 bytes.  Quick and dirty.
// Callers do best to use "random-looking" values for a and b.
static inline HashKey WeakHashLen32WithSeeds(
    uint64_t w, uint64_t x, uint64_t y, uint64_t z, uint64_t a, uint64_t b) {
  a += w;
  b = Rotate(b + a + z, 21);
  uint64_t c = a;
  a += x;
  a += y;
  b += Rotate(a, 44);
  HashKey hk;
  hk.hk64[0] = a + z;
  hk.hk64[1] = b + c;
  return hk;
}

// Return a 16-byte hash for s[0] ... s[31], a, and b.  Quick and dirty.
static inline HashKey WeakStringHashLen32WithSeeds(
    const char* s, uint64_t a, uint64_t b) {
  return WeakHashLen32WithSeeds(UNALIGNED_LOAD64(s),
                                UNALIGNED_LOAD64(s + 8),
                                UNALIGNED_LOAD64(s + 16),
                                UNALIGNED_LOAD64(s + 24),
                                a,
                                b);
}

// Return an 8-byte hash for 33 to 64 bytes.
static inline uint64_t HashLen33to64(const char *s, size_t len) {
  uint64_t z = UNALIGNED_LOAD64(s + 24);
  uint64_t a = UNALIGNED_LOAD64(s) + (len + UNALIGNED_LOAD64(s + len - 16)) * k0;
  uint64_t b = Rotate(a + z, 52);
  uint64_t c = Rotate(a, 37);
  a += UNALIGNED_LOAD64(s + 8);
  c += Rotate(a, 7);
  a += UNALIGNED_LOAD64(s + 16);
  uint64_t vf = a + z;
  uint64_t vs = b + Rotate(a, 31) + c;
  a = UNALIGNED_LOAD64(s + 16) + UNALIGNED_LOAD64(s + len - 32);
  z = UNALIGNED_LOAD64(s + len - 8);
  b = Rotate(a + z, 52);
  c = Rotate(a, 37);
  a += UNALIGNED_LOAD64(s + len - 24);
  c += Rotate(a, 7);
  a += UNALIGNED_LOAD64(s + len - 16);
  uint64_t wf = a + z;
  uint64_t ws = b + Rotate(a, 31) + c;
  uint64_t r = ShiftMix((vf + ws) * k2 + (wf + vs) * k0);
  return ShiftMix(r * k0 + vs) * k2;
}

static inline uint64_t CityHash64(const char *s, size_t len) {
  if (len <= 32) {
    if (len <= 16) {
      return HashLen0to16(s, len);
    } else {
      return HashLen17to32(s, len);
    }
  } else if (len <= 64) {
    return HashLen33to64(s, len);
  }

  // For strings over 64 bytes we hash the end first, and then as we
  // loop we keep 56 bytes of state: v, w, x, y, and z.
  uint64_t x = UNALIGNED_LOAD64(s);
  uint64_t y = UNALIGNED_LOAD64(s + len - 16) ^ k1;
  uint64_t z = UNALIGNED_LOAD64(s + len - 56) ^ k0;
  HashKey v = WeakStringHashLen32WithSeeds(s + len - 64, len, y);
  HashKey w = WeakStringHashLen32WithSeeds(s + len - 32, len * k1, k0);
  z += ShiftMix(v.hk64[1]) * k1;
  x = Rotate(z + x, 39) * k1;
  y = Rotate(y, 33) * k1;

  // Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
  len = (len - 1) & ~((size_t)(63));
  do {
    x = Rotate(x + y + v.hk64[0] + UNALIGNED_LOAD64(s + 16), 37) * k1;
    y = Rotate(y + v.hk64[1] + UNALIGNED_LOAD64(s + 48), 42) * k1;
    x ^= w.hk64[1];
    y ^= v.hk64[0];
    z = Rotate(z ^ w.hk64[0], 33);
    v = WeakStringHashLen32WithSeeds(s, v.hk64[1] * k1, x + w.hk64[0]);
    w = WeakStringHashLen32WithSeeds(s + 32, z + w.hk64[1], y);
    Swap(&z, &x);
    s += 64;
    len -= 64;
  } while (len != 0);
  return HashLen16(HashLen16(v.hk64[0], w.hk64[0]) + ShiftMix(y) * k1 + z,
                   HashLen16(v.hk64[1], w.hk64[1]) + x);
}

static inline uint64_t CityStringHash64WithSeeds(const char *s, size_t len,
                           uint64_t seed0, uint64_t seed1) {
  return HashLen16(CityHash64(s, len) - seed0, seed1);
}

static inline uint64_t CityHash64WithSeed(const char *s, size_t len, uint64_t seed) {
  return CityStringHash64WithSeeds(s, len, k2, seed);
}


// A subroutine for CityHash128().  Returns a decent 128-bit hash for strings
// of any length representable in ssize_t.  Based on City and Murmur.
static inline HashKey CityMurmur(const char *s, size_t len, uint64_t seed0, uint64_t seed1) {
  uint64_t a = seed0;
  uint64_t b = seed1;
  uint64_t c = 0;
  uint64_t d = 0;
  ssize_t l = len - 16;
  if (l <= 0) {  // len <= 16
    c = b * k1 + HashLen0to16(s, len);
    d = Rotate(a + (len >= 8 ? UNALIGNED_LOAD64(s) : c), 32);
  } else {  // len > 16
    c = HashLen16(UNALIGNED_LOAD64(s + len - 8) + k1, a);
    d = HashLen16(b + len, c + UNALIGNED_LOAD64(s + len - 16));
    a += d;
    do {
      a ^= ShiftMix(UNALIGNED_LOAD64(s) * k1) * k1;
      a *= k1;
      b ^= a;
      c ^= ShiftMix(UNALIGNED_LOAD64(s + 8) * k1) * k1;
      c *= k1;
      d ^= c;
      s += 16;
      l -= 16;
    } while (l > 0);
  }
  a = HashLen16(a, c);
  b = HashLen16(d, b);

  HashKey hk;
  hk.hk64[0] = a ^ b;
  hk.hk64[1] = HashLen16(b, a);
  
  return hk;
}

static inline HashKey CityHash128WithSeed(const char *s, size_t len, uint64_t seed0, uint64_t seed1) {
    if (likely(len < 128)) {
	return CityMurmur(s, len, seed0, seed1);
  }

  HashKey v, w;
  uint64_t x = seed0;
  uint64_t y = seed1;
  uint64_t z = len * k1;
  v.hk64[0] = Rotate(y ^ k1, 49) * k1 + UNALIGNED_LOAD64(s);
  v.hk64[1] = Rotate(v.hk64[0], 42) * k1 + UNALIGNED_LOAD64(s + 8);
  w.hk64[0] = Rotate(y + z, 35) * k1 + x;
  w.hk64[1] = Rotate(x + UNALIGNED_LOAD64(s + 88), 53) * k1;

  // This is the same inner loop as CityHash64(), manually unrolled.
  do {
    x = Rotate(x + y + v.hk64[0] + UNALIGNED_LOAD64(s + 16), 37) * k1;
    y = Rotate(y + v.hk64[1] + UNALIGNED_LOAD64(s + 48), 42) * k1;
    x ^= w.hk64[1];
    y ^= v.hk64[0];
    z = Rotate(z ^ w.hk64[0], 33);
    v = WeakStringHashLen32WithSeeds(s, v.hk64[1] * k1, x + w.hk64[0]);
    w = WeakStringHashLen32WithSeeds(s + 32, z + w.hk64[1], y);
    Swap(&z, &x);
    s += 64;
    x = Rotate(x + y + v.hk64[0] + UNALIGNED_LOAD64(s + 16), 37) * k1;
    y = Rotate(y + v.hk64[1] + UNALIGNED_LOAD64(s + 48), 42) * k1;
    x ^= w.hk64[1];
    y ^= v.hk64[0];
    z = Rotate(z ^ w.hk64[0], 33);
    v = WeakStringHashLen32WithSeeds(s, v.hk64[1] * k1, x + w.hk64[0]);
    w = WeakStringHashLen32WithSeeds(s + 32, z + w.hk64[1], y);
    Swap(&z, &x);
    s += 64;
    len -= 128;
  } while (likely(len >= 128));
  y += Rotate(w.hk64[0], 37) * k0 + z;
  x += Rotate(v.hk64[0] + z, 49) * k0;
  // If 0 < len < 128, hash up to 4 chunks of 32 bytes each from the end of s.
  size_t tail_done;
  for (tail_done = 0; tail_done < len; ) {
    tail_done += 32;
    y = Rotate(y - x, 42) * k0 + v.hk64[1];
    w.hk64[0] += UNALIGNED_LOAD64(s + len - tail_done + 16);
    x = Rotate(x, 49) * k0 + w.hk64[0];
    w.hk64[0] += v.hk64[0];
    v = WeakStringHashLen32WithSeeds(s + len - tail_done, v.hk64[0], v.hk64[1]);
  }
  // At this point our 48 bytes of state should contain more than
  // enough information for a strong 128-bit hash.  We use two
  // different 48-byte-to-8-byte hashes to get a 16-byte final result.
  x = HashLen16(x, v.hk64[0]);
  y = HashLen16(y, w.hk64[0]);

  HashKey hk;
  hk.hk64[0] = HashLen16(x + v.hk64[1], w.hk64[1]) + y;
  hk.hk64[1] = HashLen16(x + w.hk64[1], y + v.hk64[1]);

  return hk;
}

HashKey CityHash128(const char *s, size_t len) {
  if (len >= 16) {
    return CityHash128WithSeed(s + 16,
                               len - 16,
                               UNALIGNED_LOAD64(s) ^ k3,
			       UNALIGNED_LOAD64(s + 8));
  } else if (len >= 8) {
    return CityHash128WithSeed(NULL,
                               0,
                               UNALIGNED_LOAD64(s) ^ (len * k0),
			       UNALIGNED_LOAD64(s + len - 8) ^ k1);
  } else {
    return CityHash128WithSeed(s, len, k0, k1);
  }
}


/********************************************************************************
 *
 *  Basic Hash key operations that don't involve actual hashing considerations.
 *
 ********************************************************************************/

bool Hk_Equal(chk_ptr hk1, chk_ptr hk2)
{
    assert(hk1 != NULL);
    assert(hk2 != NULL);
    return Hk_EQUAL(hk1, hk2);
}

bool Hk_IsZero(chk_ptr hk)
{
    assert(hk != NULL);
    return Hk_ISZERO(hk);
}

void Hk_Clear(hk_ptr hk)
{
    Hk_CLEAR(hk);
}

void Hk_Copy(hk_ptr hk_dest, chk_ptr hk_src)
{
    Hk_COPY(hk_dest, hk_src);
}

static inline void check_hashkey_range(hk_ptr hk)
{
#ifdef NO_UINT128
    /* Ensure (pedantically) that we're less than 2^128 - 159 */
    if(unlikely(!(~hk->hk64[0])))
    {
	if(unlikely(hk->hk64[1] >= 0xFFFFFFFFFFFFFF61ull) )
	{
	    hk->hk64[0] -= 1;
	}
    }
#else
    if(unlikely(hk->hk128 >= HK_GF_PRIME))
	hk->hk128 -= k0;
#endif
}

/*******************************************************************************
 *
 *  Routines for creating hash values from other objects.
 *
 ********************************************************************************/

/* Create field from char buffer (just like a string, except length is
 * a set paraemter. */
void Hkf_FromCharBuffer(hk_ptr dest_key, const char *string, size_t length)
{
    *dest_key = CityHash128(string, length);
    check_hashkey_range(dest_key);
}

/* Create field from string. */
void Hkf_FromString(hk_ptr dest_key, const char *string)
{
    Hkf_FromCharBuffer(dest_key, string, strlen(string));
}


void Hkf_FromIntBuffer(hk_ptr dest_key, const unsigned int *it, size_t length)
{
    Hkf_FromCharBuffer(dest_key, (const char*)it, length*(sizeof(unsigned int)));
}

static inline void _Hf_FromUInt(hk_ptr dest_key, unsigned long x, unsigned long salt)
{
    unsigned long z[2] = {x, salt};
    *dest_key = CityHash128((const char*)z, 2*sizeof(unsigned long));
    check_hashkey_range(dest_key);
}

/****************************************
 *
 *   A caching scheme to make things better
 *
 ****************************************/

static HashKey* _hk_uint_lookup = NULL;

void _Hk_PopulateHKIntLookup()
{
    assert(_hk_uint_lookup == NULL);
    _hk_uint_lookup = malloc(HK_UNSIGNED_INT_LOOKUP_SIZE*sizeof(HashKey));

    size_t i, j;
    
    MTRandState* mts = Mtr_New(0);

    for(i = 0; i < HK_UNSIGNED_INT_LOOKUP_SIZE; ++i)
	for(j = 0; j < H_NUM_32BIT_COMPONENTS; ++j)
	    _hk_uint_lookup[i].hk32[j] = Mtr_Next(mts);
}

inline void Hkf_FromUnsignedInt(hk_ptr dest_key, unsigned long x)
{
    if(x < HK_UNSIGNED_INT_LOOKUP_SIZE)
    {
	if(unlikely(_hk_uint_lookup == NULL))
	    _Hk_PopulateHKIntLookup();
	
	*dest_key = _hk_uint_lookup[x];
    }
    else
	_Hf_FromUInt(dest_key, x, __LINE__);
}

void Hkf_FromInt(hk_ptr dest_key, signed long x)
{
    if(x < 0)
    {
	_Hf_FromUInt(dest_key, (unsigned long)(-x), __LINE__);
    }
    else
    {
	Hkf_FromUnsignedInt(dest_key, x);
    }
}

void Hkf_FromHashKey(hk_ptr dest_key, chk_ptr hk)
{
    /* Can use the weaker, faster version since we already have strong hashes. */
    *dest_key = WeakHashLen32WithSeeds(hk->hk64[0], hk->hk64[1], 
				       ShiftMix(hk->hk64[0]), ShiftMix(hk->hk64[1]),
				       k0*k1 + hk->hk64[0], k2*k3 + hk->hk64[1]);
}

void Hkf_FromHashKeyAndInt(hk_ptr dest_key, chk_ptr hk, signed long x)
{
    /* Can use the weaker, faster version since we already have strong hashes. */
    *dest_key = WeakHashLen32WithSeeds(hk->hk64[0], hk->hk64[1], 
				       ShiftMix(hk->hk64[0]), ShiftMix(hk->hk64[1]),
				       k0*k1*x + hk->hk64[1], k2*k3*x + hk->hk64[1]);
    check_hashkey_range(dest_key);
}

/************************************************************
 * 
 *  Atomic operations between hash keys.  The ones involving a
 *  "rehashing" are here, as they are more substantial functions.
 *
 ************************************************************/


/* Update the value of the first depending on the value of the second. */
void Hk_InplaceCombine(hk_ptr dest_key, chk_ptr hk)
{
    *dest_key = WeakHashLen32WithSeeds(dest_key->hk64[0], dest_key->hk64[1], 
				       hk->hk64[0], hk->hk64[1],
				       ShiftMix(dest_key->hk64[0])*ShiftMix(hk->hk64[0]), 
				       ShiftMix(dest_key->hk64[1])*ShiftMix(hk->hk64[1]));
    check_hashkey_range(dest_key);
}

/* Update the value of the first depending on the value of the second, along with two ints */
void Hk_InplaceCombinePlusTwoInts(hk_ptr dest_key, chk_ptr hk, int64_t s1, int64_t s2)
{
    *dest_key = WeakHashLen32WithSeeds(dest_key->hk64[0], dest_key->hk64[1], 
				       hk->hk64[0], hk->hk64[1],
				       ShiftMix(dest_key->hk64[0])*ShiftMix(hk->hk64[0]), 
				       ShiftMix(dest_key->hk64[1])*ShiftMix(hk->hk64[1]));
    check_hashkey_range(dest_key);
    
    Hk_UpdateWithTwoInts(dest_key, s1*(k0^k1), s2*(k0^k1));
}

void Hkf_Combine(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2)
{
    assert(dest_key != NULL);
    assert(hk1 != NULL);
    assert(hk2 != NULL);
    
    Hk_COPY(dest_key, hk1);
    Hk_InplaceCombine(dest_key, hk2);
}

/* Do a Rehashing of the key hk; note this is not the Rehash function
 * below, which special cases the null hash. */
void Hk_InplaceHash(hk_ptr hk)
{
    *hk = WeakHashLen32WithSeeds(hk->hk64[0], hk->hk64[1], 
				 (k2 + hk->hk64[0]) * (k3 + hk->hk64[1]), 
				 (k1 * hk->hk64[0]) ^ (k0 * hk->hk64[1]),
				 (k0*k2) ^ hk->hk64[0], 
				 (k1*k3) ^ hk->hk64[1]);
    check_hashkey_range(hk);
}

void Hk_UpdateWithInt(hk_ptr hk, int64_t v)
{
    *hk = WeakHashLen32WithSeeds(hk->hk64[0], hk->hk64[1], 
				 v + ShiftMix(hk->hk64[0]), 
				 v + ShiftMix(hk->hk64[1]),
				 (k0*k1 ^ hk->hk64[0]) * v, (k2*k3 ^ hk->hk64[1])*v);
    check_hashkey_range(hk);
}

void Hk_UpdateWithTwoInts(hk_ptr hk, int64_t v, int64_t w)
{
    *hk = WeakHashLen32WithSeeds(hk->hk64[0], hk->hk64[1], 
				 v + ShiftMix(hk->hk64[0]), 
				 w + ShiftMix(hk->hk64[1]),
				 ((k0*k1) ^ hk->hk64[0])*v, ((k2*k3) ^ hk->hk64[1])*w);
    check_hashkey_range(hk);
}

void Hk_UpdateWithIntArray(hk_ptr hk, const unsigned int *it, size_t len)
{
    HashKey a_hk;
    Hkf_FromIntBuffer(&a_hk, it, len);
    Hk_InplaceCombine(hk, &a_hk);
}

void Hkf_Reduce(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2)
{
    assert(dest_key != NULL);
    assert(hk1 != NULL);
    assert(hk2 != NULL);

    Hk_REDUCE(dest_key, hk1, hk2);
}

void Hk_ReduceUpdate(hk_ptr dest_key, chk_ptr hk)
{
    assert(dest_key != NULL);
    assert(hk != NULL);

    Hk_REDUCE_UPDATE(dest_key, hk);
}

void Hkf_Rehash(hk_ptr dest_key, chk_ptr hk)
{
    assert(dest_key != NULL);
    assert(hk != NULL);

    Hk_REHASH(dest_key, hk);
}

void Hk_InplaceRehash(hk_ptr hk)
{
    assert(hk != NULL);

    Hk_INPLACE_REHASH(hk);
}

/************************************************************
 * 
 *  Debug functions; these shouldn't be needed in production code.
 *
 ************************************************************/

void Hkf_FillExact(hk_ptr dest_key, const char *hash_string)
{
    char hash[32];

#if(PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    int i;
    for(i = 0; i < 32; ++i) 
	hash[32 - 1 - i] = hash_string[i];
#else
    int i;
    for(i = 0; i < 32; ++i) 
	hash[i] = hash_string[i];
#endif

    assert(dest_key != NULL);

    static char map[256];

#ifdef DEBUG_MODE
    memset(map, -127, 256*sizeof(char));
#endif

    map['0'] = 0;
    map['1'] = 1;
    map['2'] = 2;
    map['3'] = 3;
    map['4'] = 4;
    map['5'] = 5;
    map['6'] = 6;
    map['7'] = 7;
    map['8'] = 8;
    map['9'] = 9;
    map['a'] = 10;
    map['b'] = 11;
    map['c'] = 12;
    map['d'] = 13;
    map['e'] = 14;
    map['f'] = 15;
    map['A'] = 10;
    map['B'] = 11;
    map['C'] = 12;
    map['D'] = 13;
    map['E'] = 14;
    map['F'] = 15;

    int hi;

    for(hi = 0; hi < H_NUM_8BIT_COMPONENTS; ++hi)
    {
	int idx1 = 2*hi;
	int idx2 = 2*hi + 1;
	
	ASSERTM(map[(size_t)hash[idx1]] != -127, "Non-hexadecimal character encountered.");
	ASSERTM(map[(size_t)hash[idx2]] != -127, "Non-hexadecimal character encountered.");

	dest_key->hk8[hi] = (map[(size_t)hash[idx1]]) + ((map[(size_t)hash[idx2]]) << 4);
    }
}

void Hkf_FillFromComponents(hk_ptr dest_key, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    assert(dest_key != NULL);

    dest_key->hk32[HK32I(0)] = a;
    dest_key->hk32[HK32I(1)] = b;
    dest_key->hk32[HK32I(2)] = c;
    dest_key->hk32[HK32I(3)] = d;
}


unsigned long Hk_ExtractHashComponent(chk_ptr hk, unsigned int pos)
{
    assert(hk != NULL);

    if(pos < 4)
	return hk->hk32[HK32I(pos)];
    else
	return 0;
}

void Hk_ExtractHash(char* dest_string, chk_ptr hk)
{
    char dest[32];

    if(unlikely(hk == NULL))
    {
	dest_string[0] = 'N';
	dest_string[1] = 'U';
	dest_string[2] = 'L';
	dest_string[3] = 'L';
	memset(dest_string + 4, ' ', 28);
	return;
    }

    static const char map[16] = 
	{ '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    
    int hi;
    for(hi = 0; hi < H_NUM_8BIT_COMPONENTS; ++hi)
    {
	int idx1 = 2*hi;
	int idx2 = 2*hi + 1;

	dest[idx1] = map[getBitGroup(hk->hk8[hi], 0, 4)];
	dest[idx2] = map[getBitGroup(hk->hk8[hi], 4, 4)];
    }

#if(PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    int i;
    for(i = 0; i < 32; ++i) 
	dest_string[32 - 1 - i] = dest[i];
#else
    int i;
    for(i = 0; i < 32; ++i) 
	dest_string[i] = dest[i];
#endif
}

const char * H_HashAs8ByteString(cobj_ptr x)
{
  return (const char*)H_Hash_RO(x);
}

void Hk_debug_PrintHash(chk_ptr hk)
{
    char s[33];

    Hk_ExtractHash(s, hk);
    s[32] = '\0';
    printf("<%s> ", s);
}

unsigned long Hk_GetPrimeOffset()
{
    return H_HASHKEY_PRIME_OFFSET;
}
