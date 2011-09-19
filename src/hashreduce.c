#include "hashreduce.h"

bool IsValid(cobj_ptr h, markertype m)
{
    return H_MarkerPointIsValid(h, m);
}

vset_ptr ViewVSet(obj_ptr h)
{
    if(!H_IS_MARKED(h))
	H_GIVE_MARKER_INFO(h, Mi_New(MARKER_MINUS_INFTY, MARKER_PLUS_INFTY));

    return H_Mi(h);
}

vset_ptr GetVSet(obj_ptr h)
{
    vset_ptr v = ViewVSet(h);
    O_INCREF(v);
    return v;
}

void SetVSet(obj_ptr h, vset_ptr mi)
{
    H_SET_MARKER_INFO(h, mi);
}

void GiveVSet(obj_ptr h, vset_ptr mi)
{
    H_GIVE_MARKER_INFO(h, mi);
}

void  AddVSetInterval(obj_ptr h, markertype a, markertype b)
{
    H_ADD_MARKER_VALID_RANGE(h, a, b);
}

markertype VSetMin(cobj_ptr v)
{
    return Mi_Min(H_Mi(v));
}

markertype VSetMax(cobj_ptr v)
{
    return Mi_Max(H_Mi(v));
}

/********************************************************************************
 *
 *  Set-type update methods.  These work in one of two ways.  The
 *  first of these ways is to work directly with two objects.  This
 *  returns an object with ref-count of 1 that holds the result.  The
 *  second method, to be used with 3 or more sets, is with the update
 *  methods.  The proper way of dealing with these is as follows:

   vset_ptr vset = NULL;

   for (v in set of VSets or hash objects) {
       vset = VSetUnionUpdate(vset, v); 
   }

 *  vset now contains the union of all vsets.
 * 
 ********************************************************************************/

static inline cvset_ptr _GetAsVSet(cobj_ptr v)
{
    if(unlikely(v == NULL))
	return NULL;

    if(O_IsType(MarkerInfo, v))
	return O_CastC(MarkerInfo, v);
    else if(O_IsType(HashObject, v))
	return H_Mi(O_CastC(HashObject, v));
    else
    {
	fprintf(stderr, "Type of object passed to VSetUnion not Markertype or HashObject.");
	return NULL;
    }
}

vset_ptr VSetUnion(cobj_ptr v1, cobj_ptr v2)
{
    return Mi_Union(_GetAsVSet(v1), _GetAsVSet(v2));
}

vset_ptr VSetUnionUpdate(vset_ptr v_dest, cobj_ptr v)
{
    return Mi_UnionUpdate(v_dest, _GetAsVSet(v));
}

vset_ptr VSetIntersection(cobj_ptr v1, cobj_ptr v)
{
    return Mi_Intersection(_GetAsVSet(v1), _GetAsVSet(v));
}

vset_ptr VSetIntersectionUpdate(vset_ptr v_dest, cobj_ptr v)
{
    return Mi_IntersectionUpdate(v_dest, _GetAsVSet(v));
}

vset_ptr Difference(cobj_ptr v1, cobj_ptr v2)
{
    return Mi_Difference(_GetAsVSet(v1), _GetAsVSet(v2));
}

bool Contains(mset_ptr T, obj_ptr h)
{
    if(!O_IsType(HashObject, h))
	return false;

    return Ht_Contains(T, O_Cast(HashObject, h));
}


bool ExistsAt(mset_ptr T, obj_ptr h, markertype m)
{
    if(!O_IsType(HashObject, h))
	return false;

    return Ht_ContainsAt(T, O_Cast(HashObject, h), m);
}

obj_ptr Get(mset_ptr T, obj_ptr h)
{
    if(!O_IsType(HashObject, h))
	return NULL;

    return Ht_Get(T, O_Cast(HashObject, h));
}

obj_ptr View(mset_ptr T, obj_ptr h)
{
    if(!O_IsType(HashObject, h))
	return NULL;

    return Ht_View(T, O_Cast(HashObject, h));
}


void Insert(mset_ptr T, obj_ptr h)
{
    Ht_Set(T, O_Cast(HashObject, h));
}

void Give(mset_ptr T, obj_ptr k)  /* Like insert, but steals the reference. */
{
    Ht_Give(T, O_Cast(HashObject, k));
}

obj_ptr HashAtMarker(mset_ptr T, markertype m)
{
    return Ht_HashAtMarkerPoint(NULL, T, m);
}

bool EqualAtMarker(mset_ptr T1, mset_ptr T2, markertype m)
{
    return Ht_EqualAtMarker(T1, T2, m);
}

vset_ptr EqualityVSet(mset_ptr T1, mset_ptr T2)
{
    return Ht_EqualitySet(T1, T2);
}

hash_accumulator EqualityVSetUpdate(hash_accumulator accumulator, mset_ptr T)
{
    return Ht_EqualitySetUpdate(accumulator, T); 
}

vset_ptr EqualityVSetFinish(hash_accumulator accumulator)
{
    return Ht_EqualitySetFinish(accumulator);
}

/* Returns a validity set on which the hash is equal to the given hash. */

vset_ptr EqualToHash(mset_ptr T, cobj_ptr h)
{
    return Ht_EqualToHash(T, *H_Hash_RO(h));
}

/* These functions provide set operations; they are to be used the
 * same way the VSetX functions are.  E.g.
*/

mset_ptr MSetUnion(mset_ptr T1, mset_ptr T2)
{
    return Ht_Union(T1, T2);
}

mset_ptr MSetUnionUpdate(mset_ptr T, mset_ptr T1)
{
    return Ht_UnionUpdate(T, T1);
}

mset_ptr MSetIntersection(mset_ptr T1, mset_ptr T2)
{
    return Ht_Intersection(T1, T2);
}

mset_ptr MSetIntersectionUpdate(mset_ptr T, mset_ptr T2)
{
    return Ht_IntersectionUpdate(T, T2);
}

mset_ptr MSetDifference(mset_ptr T1, mset_ptr T2)
{
    return Ht_Difference(T1, T2);
}

mset_ptr KeySet(mset_ptr T)
{
    return Ht_KeySet(T);
}

mset_ptr MSetReduce(mset_ptr T)
{
    return Ht_ReduceTable(T);
}

/* The summarize functions work via an intermediate accumulator, which
 * may be null (in which case one is created.  The proper way of using
 * these is as follows:
 
 HashSequence accumulator = NULL;
 for (T in collection) {
    accumulator = Summarize_Add(accumulator, T);
 }

 summary = Summarize_Finish(accumulator);
 */

hash_accumulator SummarizeUpdate(hash_accumulator accumulator, mset_ptr T)
{
    return Ht_Summarize_Update(accumulator, T);
}

mset_ptr SummarizeFinish(hash_accumulator accumulator)
{
    return Ht_Summarize_Finish(accumulator);
}
