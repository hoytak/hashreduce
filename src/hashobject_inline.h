#ifndef _HASHOBJECT_INLINE_H_
#define _HASHOBJECT_INLINE_H_

#include "debugging.h"

static inline bool H_EQUAL(cobj_ptr x, cobj_ptr y)
{
    if(unlikely(x == y))
	return true;
    else if (x == NULL || y == NULL)
	return false;

    const HashObject *hx = O_CastC(HashObject, x);
    const HashObject *hy = O_CastC(HashObject, y);

    return Hk_EQUAL(H_Hash_RO(hx), H_Hash_RO(hy));
}

static inline void H_CLEAR(obj_ptr x)
{
    HashObject *h = O_Cast(HashObject, x);

    Hk_CLEAR(H_Hash_RW(h));
    if(H_IS_MARKED(h)) 
	H_ClearMarkerInfo(h);
}

/********************************************************************************
 *
 *  Locking operations
 *
 ********************************************************************************/

static inline bool H_IS_LOCKED(cobj_ptr x)
{
    return O_CastC(HashObject, x)->lock_count != 0;
}

static inline bool H_IS_MARKED(cobj_ptr x)
{
    return !Mi_ISEMPTY(O_CastC(HashObject, x)->mi);
}

static inline void H_RELEASE_LOCK(obj_ptr x)
{
    HashObject *h = O_Cast(HashObject, x);
    assert(h->lock_count >= 1);
    --h->lock_count;			
}

static inline void H_CLAIM_LOCK(obj_ptr x)
{
    HashObject *h = O_Cast(HashObject, x);
    ++h->lock_count;
}

static inline size_t H_LOCK_COUNT(cobj_ptr x)
{
     const HashObject *h = O_CastC(HashObject, x);
     return h->lock_count;
}
/*********************************************************************************
 *
 *  Functions for filling the hash keys with values.
 *
 *********************************************************************************/

static inline HashObject *_H_CreateOrCast(obj_ptr x)
{
    if(x == NULL)
    {
	return ConstructHashObject();
    }
    else
    {
	return O_Cast(HashObject, x);
    }
}

static inline HashObject* Hf_COPY_FROM_KEY(obj_ptr dest_key, chk_ptr hk)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hk_COPY(H_Hash_RW(h), hk);
    return h;
}

static inline void H_CLEAR_MARKER_INFO(obj_ptr x)
{
    HashObject *h = O_Cast(HashObject, x);

    assert(O_IsType(HashObject, h));
    assert(!H_IS_LOCKED(h));

    if(likely(h->mi != NULL)) 
    {
	O_DECREF(h->mi);
	h->mi = NULL;
    }
}


static inline void H_GIVE_MARKER_INFO(obj_ptr x, MarkerInfo *mi)
{
    HashObject *h = O_Cast(HashObject, x);

    assert(O_IsType(HashObject, h));
    assert(!H_IS_LOCKED(h));

    if(unlikely(h->mi != NULL))
	H_CLEAR_MARKER_INFO(h);

    h->mi = mi;
}


static inline void H_SET_MARKER_INFO(obj_ptr x, MarkerInfo *mi)
{
    HashObject *h = O_Cast(HashObject, x);

    if(likely(mi != NULL))
	O_INCREF(mi);

    H_GiveMarkerInfo(h, mi);
}


static inline HashObject* H_COPY(obj_ptr dest_key, cobj_ptr h)
{
    HashObject *ret_h = Hf_COPY_FROM_KEY(dest_key, H_Hash_RO(h));
    H_SET_MARKER_INFO(ret_h, H_Mi(h));

    return ret_h;
}

static inline MarkerInfo* H_Mi(cobj_ptr h)
{
    return O_Cast(HashObject, h)->mi;
}

/*************************************************************
 * 
 * Methods for accessing the hash keys of hash objects.
 *
 ************************************************************/

/* Have to do the H_Hash_RO in such a way that constness is preserved
 * through the cast.  C sucks for this, C++ rocks. :-/ */

static inline const HashKey* H_Hash_RO(cobj_ptr x)
{
    return &(O_CastC(HashObject, x)->__hashkey);
}

static inline HashKey* H_Hash_RW(obj_ptr x)
{
    HashObject *h = O_Cast(HashObject, x);
    assert(!H_IS_LOCKED(h));
    return &(h->__hashkey);
}

#endif /* _HASHOBJECT_STATIC_H_ */
