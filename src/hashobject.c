
#include "hashobject.h"
#include "bitops.h"
#include <string.h>

/********************************************************************************
 *
 *  First, all the functions needed for the hash object slots.
 *  
 *********************************************************************************/

void HashObjectDelete(HashObject *h)
{
    if(H_IS_MARKED(h))
	H_ClearMarkerInfo(h);
}

DEFINE_OBJECT(
    /* Name. */            HashObject,
    /* BaseType */         Object,
    /* construction */     NULL,   /* Zeroed out is fine here. */
    /* delete */           HashObjectDelete,   
    /* Duplicate */        NULL
    );

HashObject* NewHashObject()
{
    return ConstructHashObject();
}

/************************************************************
 * 
 *  Basic Hash Object operations.
 *
 ************************************************************/

bool H_Equal(cobj_ptr x, cobj_ptr y)
{
    return H_EQUAL(x, y);
}

void H_Clear(obj_ptr x)
{
    H_CLEAR(x);
}

/*********************************************************************************
 *
 *  Functions for filling the hash keys with values.
 *
 *********************************************************************************/

HashObject* Hf_Combine(obj_ptr dest_key, cobj_ptr h1, cobj_ptr h2)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hkf_Combine(H_Hash_RW(h), H_Hash_RO(h1), H_Hash_RO(h2));
    return h;
}


HashObject* Hf_FromString(obj_ptr dest_key, const char *string)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hkf_FromString(H_Hash_RW(h), string);
    return h;
}

/* Create field from char buffer (just like a string, except length is
 * a set paraemter. */
HashObject* Hf_FromCharBuffer(obj_ptr dest_key, const char *string, size_t length)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hkf_FromCharBuffer(H_Hash_RW(h), string, length);
    return h;
}

/* From different numerical values. */
HashObject* Hf_FromInt(obj_ptr dest_key, signed long x)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hkf_FromInt(H_Hash_RW(h), x);
    return h;
}

HashObject* Hf_FromUnsignedInt(obj_ptr dest_key, unsigned long x)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hkf_FromUnsignedInt(H_Hash_RW(h), x);
    return h;
}

/* Take the hash of a single hash object. */
HashObject* Hf_FromHashObject(obj_ptr dest_key, cobj_ptr x)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hkf_FromHashKey(H_Hash_RW(h), H_Hash_RO(x));
    
    /* Does not yet take into account marker information. */

    return h;
}

/* Copy exact hash from another key. */
HashObject* H_Copy(obj_ptr dest_key, cobj_ptr src)
{
    return H_COPY(dest_key, src);
}

HashObject* Hf_CopyFromKey(obj_ptr dest_key, const HashKey *hk)
{
    return Hf_COPY_FROM_KEY(dest_key, hk);
}

HashObject* H_CopyAsUnmarked(obj_ptr dest_key, cobj_ptr src)
{
    return H_COPY_AS_UNMARKED(dest_key, src);
}

/*********************************************************************************
 * 
 *  Operations between hash objects.
 *  
 *********************************************************************************/

/***** Order independent combining -- the reduce function. *****/
HashObject* H_Reduce(obj_ptr dest_key, cobj_ptr x1, cobj_ptr x2)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hk_REDUCE(H_Hash_RW(h), H_Hash_RO(x1), H_Hash_RO(x2));
    return h;
}

HashObject* H_ReduceUpdate(obj_ptr dest_key, cobj_ptr x)
{
    HashObject *h = O_Cast(HashObject, dest_key);
    Hk_REDUCE_UPDATE(H_Hash_RW(h), H_Hash_RO(x));
    return h;
}

/****** Rehashing ******/
HashObject* H_Rehash(obj_ptr dest_key, cobj_ptr x)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hk_REHASH(H_Hash_RW(h), H_Hash_RO(x));
    H_SET_MARKER_INFO(h, H_Mi(x));
    return h;
}

HashObject* H_InplaceRehash(obj_ptr x)
{
    HashObject *h = O_Cast(HashObject, x);
    Hk_INPLACE_REHASH(H_Hash_RW(h));
    return h;
}


/***** Negatives *****/
HashObject* H_Negative(obj_ptr dest_key, cobj_ptr x)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hk_NEGATIVE(H_Hash_RW(h), H_Hash_RO(x));
    return h;
}

HashObject* H_InplaceNegative(obj_ptr x)
{
    HashObject *h = O_Cast(HashObject, x);
    Hk_INPLACE_NEGATIVE(H_Hash_RW(h));
    return h;
}

/********************************************************************************
 *
 *  HashObject marker information manipulation
 *
 ********************************************************************************/

bool H_IsMarked(cobj_ptr x)
{
    return H_IS_MARKED(x);
}

void H_AddMarkerValidRange(obj_ptr x, markertype r_start, markertype r_end)
{
    H_ADD_MARKER_VALID_RANGE(x, r_start, r_end);
}

void H_RemoveMarkerValidRange(obj_ptr x, markertype r_start, markertype r_end)
{
    H_REMOVE_MARKER_VALID_RANGE(x, r_start, r_end);
}

bool H_MarkerPointIsValid(cobj_ptr x, markertype m)
{
    const HashObject *h = O_CastC(HashObject, x);

    if(h->mi == NULL)
	return true;
    else
	return Mi_IsValid(h->mi, m);
}

void H_ClearMarkerInfo(obj_ptr x)
{
    H_CLEAR_MARKER_INFO(x);
}

void H_GiveMarkerInfo(obj_ptr x, MarkerInfo *mi)
{
    H_GIVE_MARKER_INFO(x, mi);
}

void H_SetMarkerInfo(obj_ptr x, MarkerInfo *mi)
{
    H_SET_MARKER_INFO(x, mi);
}

markertype H_RangeMin(cobj_ptr x)
{
    const HashObject *h = O_CastC(HashObject, x);
    return Mi_Min((h)->mi);
}

markertype H_RangeMax(cobj_ptr x)
{
    const HashObject *h = O_CastC(HashObject, x);
    return Mi_Max((h)->mi);
}

/********************************************************************************
 *
 *  Debug functions; shouldn't need to be used outside the main code
 *
 ********************************************************************************/

/* For testing purposes, a way to specify an exact hash string. */
HashObject* Hf_FillExact(obj_ptr dest_key, const char *hash)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hkf_FillExact(H_Hash_RW(h), hash);
    return h;
}

HashObject* Hf_FillFromComponents(obj_ptr dest_key, unsigned int a, unsigned int b, 
				  unsigned int c, unsigned int d)
{
    HashObject *h = _H_CreateOrCast(dest_key);
    Hkf_FillFromComponents(H_Hash_RW(h), a,b,c,d);
    return h;
}

unsigned long H_ExtractHashComponent(cobj_ptr x, unsigned int pos)
{
    return Hk_ExtractHashComponent(H_Hash_RO(x), pos);
}

void H_ExtractHash(char* dest_string, cobj_ptr x)
{
    Hk_ExtractHash(dest_string, H_Hash_RO(x));
}

void H_debug_print(cobj_ptr x)
{
    const HashObject *h = O_CastC(HashObject, x);
    printf("Hash Object %lxud: ", (size_t)h);
    Hk_debug_PrintHash(H_Hash_RO(h));
    printf(": \t");
    Mi_debug_printMi(H_Mi(h));
}

void H_print(cobj_ptr x)
{
    const HashObject *h = O_CastC(HashObject, x);
    Hk_Print(H_Hash_RO(h));
    printf(": \t");
    Mi_Print(H_Mi(h));
}
