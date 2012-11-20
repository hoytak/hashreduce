#ifndef _RANDFUNCTIONS_INLINE_H_
#define _RANDFUNCTIONS_INLINE_H_

static inline MTRandState* Mtr_New(uint32_t seed)
{
    MTRandState* mts = (MTRandState*)malloc(sizeof(MTRandState));

    mts->mti = 1;

    mts->mt[0]= seed & 0xffffffffUL;
    int mti;

    for (mti=1; mti<__MTRAND_N; mti++) {
        mts->mt[mti] =
	    (1812433253UL * (mts->mt[mti-1] ^ (mts->mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mts->mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }

    return mts;
}

static inline uint32_t Mtr_Next(MTRandState* mts)
{
    unsigned long y;

    if(unlikely(mts->mti >= __MTRAND_N))
	_Mtr_next_state(mts);
  
    y = mts->mt[mts->mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return (uint32_t)y;
}

static inline void Mtr_Delete(MTRandState* mts) 
{
    free(mts);
}

static inline LCGState Lcg_New(uint32_t seed)
{
    // Just make sure we don't have a weak seed with integer size truncations
    return ( ( (uint64_t) seed) + 0xcb63b83e3798bbfeull);
}

static inline uint32_t Lcg_Next(LCGState* state)
{
    (*state) = 6364136223846793005ul * (*state) + 1442695040888963407ull;
    return (uint32_t)( ((*state) >> 32) ^ (*state) );
}




#endif /* _RANDFUNCTIONS_INLINE_H_ */
