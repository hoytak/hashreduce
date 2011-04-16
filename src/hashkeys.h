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
#include "types.h"
#include <stdint.h>

/**************************************************
 * All constants related to the hash keys.
 **************************************************/
#define H_HASHKEY_PRIME_OFFSET  159   /* using GF(2^128 - H_HASHKEY_PRIME_OFFSET)
				       * for ring.*/
#ifndef NO_UINT128
#define HK_GF_PRIME ( ( ~( (uint128_type) 0) ) - (H_HASHKEY_PRIME_OFFSET - 1))
#endif

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
#ifndef NO_UINT128
	uint128_type hk128;
#endif
	hashfieldtype hk[H_NUM_COMPONENTS];
	uint64_t      hk64[H_NUM_64BIT_COMPONENTS];
	uint32_t      hk32[H_NUM_32BIT_COMPONENTS];
	uint8_t       hk8[H_NUM_8BIT_COMPONENTS];
    };
} HashKey;

/* Typedefs */

typedef HashKey* _restrict_ hk_ptr;
typedef const HashKey* _restrict_ chk_ptr;

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
bool Hk_IsZero(chk_ptr hk);

/************************************************************
 * 
 *  Basic HashKey Manipulations -- Clearing, copying, etc.
 *
 ************************************************************/

void Hk_Clear(hk_ptr hk);
void Hk_Copy(hk_ptr hk_dest, chk_ptr hk_src);

/************************************************************
 * 
 *  HashKey Filling -- filling a particular hash key with keys based
 *  on various values.
 *
 ************************************************************/

#define HK_UNSIGNED_INT_LOOKUP_SIZE 4096

/* Create or fill Hash Keys. */
void Hkf_FromString       (hk_ptr dest_key, const char *string);
void Hkf_FromCharBuffer   (hk_ptr dest_key, const char *string, size_t length);
void Hkf_FromIntBuffer    (hk_ptr dest_key, const unsigned int *it, size_t length);
void Hkf_FromInt          (hk_ptr dest_key, signed long x);
void Hkf_FromUnsignedInt  (hk_ptr dest_key, unsigned long x);
void Hkf_FromHashKey      (hk_ptr dest_key, chk_ptr hk);
void Hkf_FromHashKeyAndInt(hk_ptr dest_key, chk_ptr hk, signed long x);

static inline HashKey Hk_FromString(const char *string);
static inline HashKey Hk_FromCharBuffer(const char *string, size_t length);
static inline HashKey Hk_FromIntBuffer(const unsigned int *it, size_t length);
static inline HashKey Hk_FromInt(signed long x);
static inline HashKey Hk_FromUnsignedInt(unsigned long x);
static inline HashKey Hk_FromHashKey(chk_ptr hk);
static inline HashKey Hk_FromHashKeyAndInt(chk_ptr hk, signed long x);

/************************************************************
 * 
 *  Atomic operations between hash keys.
 *
 ************************************************************/

/***** Order dependent combining. *****/
void Hkf_Combine(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2);

/* Update the value of the first depending on the value of the second. */
void Hk_InplaceCombine(hk_ptr dest_key, chk_ptr hk);

/* Do a Rehashing of the key hk; note this is not the Rehash function
 * below, which special cases the null hash. */
void Hk_InplaceHash(hk_ptr hk);

/* Fast functions for use with various hashing operations elsewhere.
 * These are not equivalent, in any special cases, to Hk_InplaceCombine. */
void Hk_UpdateWithInt(hk_ptr dest_key, int64_t v);
void Hk_UpdateWithTwoInts(hk_ptr dest_key, int64_t v, int64_t w);
void Hk_UpdateWithIntArray(hk_ptr dest_key, const unsigned int* a, size_t len);

/***** Order independent combining -- the reduce function. *****/
void Hkf_Reduce(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2);
void Hk_ReduceUpdate(hk_ptr dest_key, chk_ptr hk);

/****** Rehashing ******/
void Hkf_Rehash(hk_ptr dest_key, chk_ptr hk);
void Hk_InplaceRehash(hk_ptr hk);

/***** Negatives *****/
void Hkf_Negative(hk_ptr dest_key, chk_ptr hk);
void Hk_InplaceNegative(hk_ptr hk);

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
