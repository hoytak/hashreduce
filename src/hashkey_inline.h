#ifndef _HASHKEY_INLINE_H_
#define _HASHKEY_INLINE_H_

#include "debugging.h"
#include "optimizations.h"

/************************************************************
 * 
 *  Lowlevel HashKey Manipulations -- Clearing, copying, etc.
 *
 ************************************************************/

#ifdef NO_UINT128 

/********************************************************************************
 *
 *   The versions where we only have 64bit integer support.  
 *
 ********************************************************************************/

#define Hk_CLEAR(hk)					\
    do {						\
        assert((hk) != NULL);				\
	int i;						\
	for(i = 0; i < H_NUM_64BIT_COMPONENTS; ++i)	\
	  (hk)->hk64[i] = 0;				\
    }while(0)

#define Hk_ISZERO(hk) ( ((hk)->hk64[0] == 0) && ((hk)->hk64[1] == 0))

#define Hk_COPY(hk_dest, hk_src)			\
    do {						\
	assert((hk_dest) != NULL);			\
	assert((hk_src) != NULL);			\
	assert((hk_dest) != (hk_src));			\
	int i;						\
	for(i = 0; i < H_NUM_64BIT_COMPONENTS; ++i)	\
	    (hk_dest)->hk64[i] = (hk_src)->hk64[i];	\
    }while(0)

static inline void Hk_REDUCE_UPDATE(hk_ptr dest_hk, chk_ptr hk)
{
    /* performs addition mod 2^128 - 159 */

    assert(dest_hk != NULL);
    assert(hk != NULL);
    
    uint64_t zh = dest_hk->hk64[HK64I(0)];
    uint64_t zl = dest_hk->hk64[HK64I(1)];
    uint64_t xh = hk->hk64[HK64I(0)];
    uint64_t xl = hk->hk64[HK64I(1)];

    zl += xl;
    const uint64_t xh_add = xh + ((zl < xl) ? 1 : 0);
    zh += xh_add;

    if(zh < xh_add || xh_add < xh)
    {
	zl += H_HASHKEY_PRIME_OFFSET;
	if(unlikely(zl < H_HASHKEY_PRIME_OFFSET)) 
	    ++zh;
    }
    else if(zh == 0xFFFFFFFFFFFFFFFFull)  // unlikely(!(~zh)))
    {
	const uint64_t test_zl = ((uint64_t)0xFFFFFFFFFFFFFFFFull) - ((uint64_t)(H_HASHKEY_PRIME_OFFSET));

	if(unlikely(zl > test_zl))
	{
	    zl += H_HASHKEY_PRIME_OFFSET;
	    zh = 0;
	}
    }

    dest_hk->hk64[HK64I(0)] = zh;
    dest_hk->hk64[HK64I(1)] = zl;
}

static inline void Hk_INPLACE_NEGATIVE(hk_ptr hk)
{
    uint64_t u = hk->hk64[HK64I(0)];
    uint64_t l = hk->hk64[HK64I(1)];

    /* Need to compute 2^128 - H_HASHKEY_PRIME_OFFSET - hk */

    /* See if it's zero; this is special cased. */
    if(unlikely(!l && !u))
	return;

    /* First compute 2^128 - 1 - hk */

    u = ~u;
    l = ~l;

    /* Next grab more if needed */
    if(unlikely(l < H_HASHKEY_PRIME_OFFSET - 1))
    {
	l += (0xFFFFFFFFFFFFFFFFull - H_HASHKEY_PRIME_OFFSET + 2);
	u -= 1;
    }
    else
    {
	l -= H_HASHKEY_PRIME_OFFSET - 1;
    }

    hk->hk64[HK64I(0)] = u;
    hk->hk64[HK64I(1)] = l;
}

static inline void Hk_INPLACE_REHASH(hk_ptr hk)
{
    /* The following is the "theoretical" version; in practice, just
     * special case 0.
    
    if(unlikely(!neg_of_rehashed_null_set))
    {
	HashKey dest_hk;
	Hk_CLEAR(&dest_hk);
	Hk_InplaceHash(&dest_hk);
	Hk_INPLACE_NEGATIVE(&dest_hk);
	Hk_COPY(&neg_of_rehashed_null, &dest_hk);
	neg_of_rehashed_null_set = true;
    }

    Hk_InplaceHash(hk);
    Hk_REDUCE_UPDATE(hk, &neg_of_rehashed_null);
    */

    const uint64_t u = hk->hk64[HK64I(0)];
    const uint64_t l = hk->hk64[HK64I(1)];

    if(likely((!! l) || (!! u)))
	Hk_InplaceHash(hk);
}

#define Hk_EQUAL(hk1, hk2) \
    ( ((hk1)->hk64[1] == (hk2)->hk64[1]) && ((hk1)->hk64[0] == (hk2)->hk64[0]))

#define Hk_LT(hk1, hk2)						\
    ( ((hk1)->hk64[HK64I(0)] < (hk2)->hk64[HK64I(0)])		\
    || (((hk1)->hk64[HK64I(0)] == (hk2)->hk64[HK64I(0)])	\
	&& ((hk1)->hk64[HK64I(1)] < (hk2)->hk64[HK64I(1)])))

#define Hk_GT(hk1, hk2)						\
    ( ((hk1)->hk64[HK64I(0)] > (hk2)->hk64[HK64I(0)])		\
    || (((hk1)->hk64[HK64I(0)] == (hk2)->hk64[HK64I(0)])	\
	&& ((hk1)->hk64[HK64I(1)] > (hk2)->hk64[HK64I(1)])))

#define Hk_LEQ(hk1, hk2)  (!Hk_GT((hk1),(hk2)))
#define Hk_GEQ(hk1, hk2)  (!Hk_LT((hk1),(hk2)))

#else

/********************************************************************************
 *
 *   The versions where we have 128bit integer support.  
 *
 ********************************************************************************/

#define Hk_CLEAR(hk)					\
    do {						\
	(hk)->hk128 = 0;				\
    }while(0)

#define Hk_ISZERO(hk) ( ! ((hk)->hk128 ) )

#define Hk_COPY(hk_dest, hk_src)			\
    do {						\
	assert((hk_dest) != NULL);			\
	assert((hk_src) != NULL);			\
	assert((hk_dest) != (hk_src));			\
	assert((hk_src)->hk128 < HK_GF_PRIME);		\
	(hk_dest)->hk128 = (hk_src)->hk128;		\
	assert(Hk_EQUAL(hk_dest, hk_src));		\
    }while(0)

#define Hk_REDUCE_UPDATE(dest_hk, hk)					\
    do {								\
	assert((dest_hk) != NULL);					\
	assert((hk) != NULL);						\
	assert((hk)->hk128 < HK_GF_PRIME);				\
	assert((dest_hk)->hk128 < HK_GF_PRIME);				\
	(dest_hk)->hk128 += (hk)->hk128;				\
	(dest_hk)->hk128 += (((dest_hk)->hk128 < (hk)->hk128)		\
			     ? H_HASHKEY_PRIME_OFFSET : 0);		\
	if(unlikely((dest_hk)->hk128 >= HK_GF_PRIME))			\
	    (dest_hk)->hk128 -= HK_GF_PRIME;				\
	assert((dest_hk)->hk128 < HK_GF_PRIME);				\
    }while(0)

#define Hk_INPLACE_NEGATIVE(hk)						\
    do {								\
	assert((hk)->hk128 < HK_GF_PRIME);				\
	(hk)->hk128 = likely((hk)->hk128) ? (HK_GF_PRIME - (hk)->hk128) : 0; \
    }while(0)

#define Hk_INPLACE_REHASH(hk)				\
    do{							\
	if(likely(!!((hk)->hk128)))			\
	    Hk_InplaceHash(hk);				\
    }while(0)

#define Hk_EQUAL(hk1, hk2) \
    ( ((hk1)->hk64[1] == (hk2)->hk64[1]) && ((hk1)->hk64[0] == (hk2)->hk64[0]))

/* #define Hk_LT(hk1, hk2)						\ */
/*     ( ((hk1)->hk64[HK64I(0)] < (hk2)->hk64[HK64I(0)])		\ */
/*     || (((hk1)->hk64[HK64I(0)] == (hk2)->hk64[HK64I(0)])	\ */
/* 	&& ((hk1)->hk64[HK64I(1)] < (hk2)->hk64[HK64I(1)]))) */

/* #define Hk_GT(hk1, hk2)						\ */
/*     ( ((hk1)->hk64[HK64I(0)] > (hk2)->hk64[HK64I(0)])		\ */
/*     || (((hk1)->hk64[HK64I(0)] == (hk2)->hk64[HK64I(0)])	\ */
/* 	&& ((hk1)->hk64[HK64I(1)] > (hk2)->hk64[HK64I(1)]))) */


#define Hk_LT(hk1, hk2)	((hk1)->hk128 < (hk2)->hk128)
#define Hk_GT(hk1, hk2)	((hk1)->hk128 > (hk2)->hk128)
#define Hk_LEQ(hk1, hk2)  (!Hk_GT((hk1),(hk2)))
#define Hk_GEQ(hk1, hk2)  (!Hk_LT((hk1),(hk2)))

#endif /* Use uint128. */


/********************************************************************************
 *
 *  Now some routines that tie all of these together.
 *
 ********************************************************************************/


#define Hk_REDUCE(dest_hk, hk1, hk2)					\
    do {								\
	assert((dest_hk) != NULL);					\
	assert((hk1) != NULL);						\
	assert((hk2) != NULL);						\
	Hk_COPY((dest_hk), (hk1));					\
	Hk_REDUCE_UPDATE((dest_hk), (hk2));				\
    }while(0)


#define Hk_REHASH(dest_hk, hk)						\
    do {								\
	assert((dest_hk) != NULL);					\
	assert((hk) != NULL);						\
	Hk_COPY((dest_hk), (hk));					\
	Hk_INPLACE_REHASH(dest_hk);					\
    }while(0)

#define Hk_NEGATIVE(dest_hk, hk)					\
    do {								\
	assert((dest_hk) != NULL);					\
	assert((hk) != NULL);						\
	Hk_COPY((dest_hk), (hk));					\
	Hk_INPLACE_NEGATIVE(dest_hk);					\
    }while(0)

/* comparing 128bit integers with equality seems to be broken in gcc;
 * it's not really working for me. */

/* #define Hk_EQUAL(hk1, hk2) ((hk1)->hk128 == (hk2)->hk128) */
#define Hk_EQUAL(hk1, hk2) ( ((hk1)->hk64[1] == (hk2)->hk64[1]) && ((hk1)->hk64[0] == (hk2)->hk64[0]))


/********************************************************************************
 *
 *   Wrapper methods for the above when things are done using keys as values.
 *
 ********************************************************************************/

static inline HashKey Hk_FromString(const char *string)
{
    HashKey hk;
    Hkf_FromString(&hk, string);
    return hk;
}

static inline HashKey Hk_FromCharBuffer(const char *string, size_t length)
{
    HashKey hk;
    Hkf_FromCharBuffer(&hk, string, length);
    return hk;
}

static inline HashKey Hk_FromIntBuffer(const unsigned int *it, size_t length)
{
    HashKey hk;
    Hkf_FromIntBuffer(&hk, it, length);
    return hk;
}

static inline HashKey Hk_FromInt(signed long x)
{
    HashKey hk;
    Hkf_FromInt(&hk, x);
    return hk;
}

static inline HashKey Hk_FromUnsignedInt(unsigned long x)
{
    HashKey hk;
    Hkf_FromUnsignedInt(&hk, x);
    return hk;
}

static inline HashKey Hk_FromHashKey(chk_ptr h)
{
    HashKey hk;
    Hkf_FromHashKey(&hk, h);
    return hk;
}

static inline HashKey Hk_FromHashKeyAndInt(chk_ptr hk, signed long x)
{
    HashKey _hk;
    Hkf_FromHashKeyAndInt(&_hk, hk, x);
    return _hk;
}


#endif /* _HASHKEY_INLINE_H_ */
