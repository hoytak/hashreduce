/************************************************************
 *
 * The HashKey manipulations are the core of the framework.
 * hashkeys.h declares all operations dealing with hash keys.
 *
 *
 ************************************************************/

#ifndef _HASHKEYS_H_
#define _HASHKEYS_H_

#include "optimizations.h"
#include "debugging.h"
#include <stdint.h>

/**************************************************
 * All constants related to the hash keys.
 **************************************************/
#define H_HASHKEY_PRIME_OFFSET  159   /* N used for modulus is 2^128 - this */
#define H_COMPONENT_SIZE (sizeof(unsigned int)*8)
#define H_64BIT          (H_COMPONENT_SIZE == 64)
#define H_32BIT          (H_COMPONENT_SIZE == 32)
#define H_NUM_COMPONENTS (128 / H_COMPONENT_SIZE)
#define H_NUM_64BIT_COMPONENTS 2
#define H_NUM_32BIT_COMPONENTS 4
#define H_NUM_8BIT_COMPONENTS  16

typedef unsigned int hashfieldtype;

/**************************************************
 * The central hash key structure.  Use unions to allow for all the
 * different ways to access the data
 **************************************************/

typedef struct {
    union {
	hashfieldtype hk[H_NUM_COMPONENTS];
	uint64_t      hk64[H_NUM_64BIT_COMPONENTS];
	uint32_t      hk32[H_NUM_32BIT_COMPONENTS];
	uint8_t       hk8[H_NUM_8BIT_COMPONENTS];
    };
} HashKey;

/* Typedefs */

typedef HashKey* restrict hk_ptr;
typedef const HashKey* restrict chk_ptr;

/* Macros to handle endianness. Need to make sure we deal with this
 * regarding the union above, as it matters.*/

#if(PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)

#define HK64I(i)        (H_NUM_64BIT_COMPONENTS - 1 - i)
#define HK32I(i)        (H_NUM_32BIT_COMPONENTS - 1 - i)
#define HK8I(i)         (H_NUM_8BIT_COMPONENTS - 1 - i)
#define HKI(i)          (H_NUM_COMPONENTS - 1 - i)

//#warning "Is little endian"
#else

#define HK64I(i)        (i)
#define HK32I(i)        (i)
#define HK8I(i)         (i)
#define HKI(i)          (i)

//#warning "Is big endian"

#endif

/************************************************************
 *
 *  Hash Key Equality checking and the like.
 *
 ************************************************************/

bool Hk_Equal(chk_ptr hk1, chk_ptr hk2);
#define Hk_EQUAL(hk1, hk2) ( ((hk1)->hk64[1] == (hk2)->hk64[1]) && ((hk1)->hk64[0] == (hk2)->hk64[0]))

bool Hk_IsZero(chk_ptr hk);
#define Hk_ISZERO(hk) ( ((hk)->hk64[0] == 0) && ((hk)->hk64[1] == 0))

/************************************************************
 * 
 *  Basic HashKey Manipulations -- Clearing, copying, etc.
 *
 ************************************************************/

void Hk_Clear(hk_ptr hk);
static inline void Hk_CLEAR(hk_ptr hk);

void Hk_Copy(hk_ptr hk_dest, chk_ptr hk_src);
static inline void Hk_COPY(hk_ptr hk_dest, chk_ptr hk_src);

/************************************************************
 * 
 *  HashKey Filling -- filling a particular hash key with keys based
 *  on various values.
 *
 ************************************************************/

/* Create field from string. */
void Hkf_FromString       (hk_ptr dest_key, const char *string);
void Hkf_FromCharBuffer   (hk_ptr dest_key, const char *string, size_t length);
void Hkf_FromIntBuffer    (hk_ptr dest_key, const unsigned int *it, size_t length);
void Hkf_FromInt          (hk_ptr dest_key, signed long x);
void Hkf_FromUnsignedInt  (hk_ptr dest_key, unsigned long x);
void Hkf_FromHashKey      (hk_ptr dest_key, chk_ptr hk);
void Hkf_FromHashKeyAndInt(hk_ptr dest_key, chk_ptr hk, signed long x);

/************************************************************
 * 
 *  Atomic operations between hash keys.
 *
 ************************************************************/

/***** Order dependent combining. *****/
void Hk_Combine(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2);

/* Update the value of the first depending on the value of the second. */
void Hk_InplaceCombine(hk_ptr dest_key, chk_ptr hk);

/* Do a Rehashing of the key hk; note this is not the Rehash function
 * below, which special cases the null hash. */
void Hk_InplaceHash(hk_ptr hk);

/* Fast functions for use with various hashing operations elsewhere.
 * These are not equivalent, in any special cases, to Hk_InplaceCombine. */
void Hk_UpdateWithFourHashes(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2,chk_ptr hk3,chk_ptr hk4);
void Hk_UpdateWithInt(hk_ptr dest_key, uint64_t v);
void Hk_UpdateWithIntArray(hk_ptr dest_key, const unsigned int* a, size_t len);

/***** Order independent combining -- the reduce function. *****/
void Hk_Reduce(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2);
static inline void Hk_REDUCE(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2);

void Hk_ReduceUpdate(hk_ptr dest_key, chk_ptr hk);
static inline void Hk_REDUCE_UPDATE(hk_ptr dest_key, chk_ptr hk);

/****** Rehashing ******/
void Hk_Rehash(hk_ptr dest_key, chk_ptr hk);
static inline void Hk_REHASH(hk_ptr dest_key, chk_ptr hk);

void Hk_InplaceRehash(hk_ptr hk);
static inline void Hk_INPLACE_REHASH(hk_ptr hk);

/***** Negatives *****/
void Hk_Negative(hk_ptr dest_key, chk_ptr hk);
static inline void Hk_NEGATIVE(hk_ptr dest_key, chk_ptr hk);

void Hk_InplaceNegative(hk_ptr hk);
static inline void Hk_INPLACE_NEGATIVE(hk_ptr hk);

/************************************************************
 * 
 *  Debug functions; these shouldn't be needed in production code.
 *
 ************************************************************/

/* Be able to test specific hash keys. */
void Hkf_FillExact(hk_ptr dest_key, const char *hash);
void Hkf_FillFromComponents(hk_ptr dest_key, uint32_t a, uint32_t b, uint32_t c, uint32_t d); 

/* Extract the 4 32bit values associated with the hash keys, pos = {0,1,2,3} */
unsigned long Hk_ExtractHashComponent(chk_ptr hk, unsigned int pos);

/* Fills the dest string with a hex representation of the given data. */
void Hk_ExtractHash(char* dest_string, chk_ptr hk);
void Hk_debug_PrintHash(chk_ptr hk);

unsigned long Hk_GetPrimeOffset();

#include "hashkey_inline.h"

#endif /* _HASHKEYS_H_ */
