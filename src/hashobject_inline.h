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

static inline bool H_MarkerIsLocked(cobj_ptr x)
{
#ifndef NDEBUG
    bool ret = O_CastC(HashObject, x)->marker_lock_count != 0;

    if(ret)
	assert(H_Mi(x) == NULL || Mi_IsDebugLocked(H_Mi(x)));

    return ret;
#else
    return false;
#endif
}

static inline bool H_IS_MARKED(cobj_ptr x)
{
    return (!Mi_VALID_EVERYWHERE(H_Mi(x)));
}

static inline void H_ReleaseMarkerLock(obj_ptr x)
{
#ifndef NDEBUG    
    HashObject *h = O_Cast(HashObject, x);
    assert(h->marker_lock_count >= 1);
    --h->marker_lock_count;			
    if(H_Mi(x) != NULL)
    {
	Mi_ReleaseDebugLock(H_Mi(x));
	assert(h->marker_lock_count == Mi_DebugLockCount(H_Mi(x)));
    }
#endif
}

static inline void H_ClaimMarkerLock(obj_ptr x)
{
#ifndef NDEBUG    
    HashObject *h = O_Cast(HashObject, x);
    ++h->marker_lock_count;
    
    if(H_Mi(x) != NULL)
    {
	Mi_ClaimDebugLock(H_Mi(x));
	assert(h->marker_lock_count == Mi_DebugLockCount(H_Mi(x)));
    }
#endif
}

static inline size_t H_MarkerLockCount(cobj_ptr x)
{
#ifndef NDEBUG    
     const HashObject *h = O_CastC(HashObject, x);
     size_t ret = h->marker_lock_count;

     if(ret != 0 && H_Mi(x) != NULL)
	 assert(Mi_DebugLockCount(H_Mi(x)) == ret);
     
     return ret;
#else
     return 0;
#endif
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
    assert(!H_MarkerIsLocked(h));

    if(likely(h->mi != NULL)) 
    {
	O_DECREF(h->mi);
	h->mi = NULL;
    }
}

static inline void H_ADD_MARKER_VALID_RANGE(obj_ptr x, markertype r_start, markertype r_end)
{
    HashObject *h = O_Cast(HashObject, x);
    assert(!H_MarkerIsLocked(h));

    if(unlikely(h->mi == NULL))
	h->mi = Mi_New(r_start, r_end);
    else
	Mi_AddValidRange(h->mi, r_start, r_end);
}

static inline void H_REMOVE_MARKER_VALID_RANGE(obj_ptr x, markertype r_start, markertype r_end)
{
    HashObject *h = O_Cast(HashObject, x);
    assert(!H_MarkerIsLocked(h));

    if(h->mi != NULL)
	Mi_RemoveValidRange(h->mi, r_start, r_end);
}

static inline void H_GIVE_MARKER_INFO(obj_ptr x, MarkerInfo *mi)
{
    HashObject *h = O_Cast(HashObject, x);

    assert(O_IsType(HashObject, h));
    assert(!H_MarkerIsLocked(h));

    if(unlikely(h->mi != NULL))
	H_CLEAR_MARKER_INFO(h);

    h->mi = mi;
}

static inline void H_SET_MARKER_INFO(obj_ptr x, MarkerInfo *mi)
{
    HashObject *h = O_Cast(HashObject, x);

    if(likely(mi != NULL))
	O_INCREF(mi);

    H_GIVE_MARKER_INFO(h, mi);
}

static inline HashObject* H_COPY(obj_ptr dest_key, cobj_ptr h)
{
    HashObject *ret_h = Hf_COPY_FROM_KEY(dest_key, H_Hash_RO(h));
    H_GIVE_MARKER_INFO(ret_h, Mi_Copy(H_Mi(h)));

#ifndef NDEBUG
    if(unlikely(!Mi_Equal(H_Mi(ret_h), H_Mi(h))))
    {
	printf("\n copied marker info not equal!");
	printf("\n copied: "); Mi_debug_printMi(H_Mi(ret_h));
	printf("\n origin: "); Mi_debug_printMi(H_Mi(h));
	abort();
    }
#endif

    return ret_h;
}

static inline HashObject* H_COPY_AS_UNMARKED(obj_ptr dest_key, cobj_ptr h)
{
    HashObject *ret_h = Hf_COPY_FROM_KEY(dest_key, H_Hash_RO(h));

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
    assert(!H_MarkerIsLocked(h));
    return &(h->__hashkey);
}

#endif /* _HASHOBJECT_STATIC_H_ */
