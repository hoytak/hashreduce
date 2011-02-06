#ifndef _HASHKEY_INLINE_H_
#define _HASHKEY_INLINE_H_

#include "debugging.h"
#include "optimizations.h"

/************************************************************
 * 
 *  Basic HashKey Manipulations -- Clearing, copying, etc.
 *
 ************************************************************/

static inline void Hk_CLEAR(hk_ptr hk)
{
    assert(hk != NULL);
    int i;
    for(i = 0; i < H_NUM_64BIT_COMPONENTS; ++i)
	hk->hk64[i] = 0;
}

static inline void Hk_COPY(hk_ptr hk_dest, chk_ptr hk_src)
{
    assert(hk_dest != NULL);
    assert(hk_src != NULL);
    assert(hk_dest != hk_src);
    
    int i;
    for(i = 0; i < H_NUM_64BIT_COMPONENTS; ++i)
	(hk_dest)->hk64[i] = (hk_src)->hk64[i];
}

/************************************************************
 * 
 *  Atomic operations between hash keys.
 *
 ************************************************************/

static inline void Hk_REDUCE_UPDATE(hk_ptr dest_key, chk_ptr hk)
{
    /* performs addition mod 2^128 - 151 */

    assert(dest_key != NULL);
    assert(hk != NULL);
    
    uint64_t zh = dest_key->hk64[HK64I(0)];
    uint64_t zl = dest_key->hk64[HK64I(1)];
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

    dest_key->hk64[HK64I(0)] = zh;
    dest_key->hk64[HK64I(1)] = zl;
}

static inline void Hk_REDUCE(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2)
{
    assert(dest_key != NULL);
    assert(hk1 != NULL);
    assert(hk2 != NULL);
    
    Hk_COPY(dest_key, hk1);
    Hk_REDUCE_UPDATE(dest_key, hk2);
}

/* These should only need to be written once, and it will be the same
 * value, so this should still be threadsafe. */ 

/*
static HashKey neg_of_rehashed_null;
static bool    neg_of_rehashed_null_set = false;
*/

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

static inline void Hk_REHASH(hk_ptr dest_key, chk_ptr hk)
{
    assert(dest_key != NULL);
    assert(hk != NULL);

    Hk_COPY(dest_key, hk);
    Hk_INPLACE_REHASH(dest_key);
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


static inline void Hk_NEGATIVE(hk_ptr dest_key, chk_ptr hk)
{
    Hk_COPY(dest_key, hk);
    Hk_INPLACE_NEGATIVE(dest_key);
}



#endif /* _HASHKEY_INLINE_H_ */
