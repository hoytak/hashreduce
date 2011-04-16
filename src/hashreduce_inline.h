#ifndef _HASHREDUCE_INLINE_H_
#define _HASHREDUCE_INLINE_H_

static inline bool  IsValid(cobj_ptr h, markertype m)
{
    return H_MarkerPointIsValid(h, m);
}

static inline vset_ptr ViewVSet(obj_ptr h)
{
    if(!H_IS_MARKED(h))
	H_GIVE_MARKER_INFO(h, Mi_New(MARKER_MINUS_INFTY, MARKER_PLUS_INFTY));

    return H_Mi(h);
}

static inline vset_ptr GetVSet(obj_ptr h)
{
    vset_ptr v = ViewVSet(h);
    O_INCREF(v);
    return v;
}

static inline void SetVSet(obj_ptr h, vset_ptr mi)
{
    H_SET_MARKER_INFO(h, mi);
}

static inline void GiveVSet(obj_ptr h, vset_ptr mi)
{
    H_GIVE_MARKER_INFO(h, mi);
}

static inline void  AddVSetInterval(obj_ptr h, markerinfo a, markerinfo b)
{
    H_ADD_MARKER_VALID_RANGE(h, a, b);
}

static inline markerinfo VSetMin(cobj_ptr v)
{
    return Mi_Min(H_Mi(v));
}

static inline markerinfo VSetMax(cobj_ptr v)
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
	fprintf(stderr, "Type of object passed to VSetUnion not MarkerInfo or HashObject.");
	return NULL;
    }
}

static inline vset_ptr VSetUnion(cobj_ptr v1, cobj_ptr v2)
{
    return Mi_Union(_GetAsVSet(v1), _GetAsVSet(v2));
}

static inline vset_ptr VSetUnionUpdate(vset_ptr v_dest, cobj_ptr v2);

static inline vset_ptr VSetIntersection(cobj_ptr v1, cobj_ptr v);
static inline vset_ptr VSetIntersectionUpdate(vset_ptr v_dest, cobj_ptr v);

static inline vset_ptr Difference(cobj_ptr v1, cobj_ptr v2);

/********************************************************************************
 *
 *  Operations involving Msets and their elements.  Note that here,
 *  querying is done by using hash objects; the keys are the only
 *  thing used in the query (except for the insert function, which
 *  inserts that key). This is done for memory management purposes.
 *
 ********************************************************************************/

typedef HashTable* mset_ptr;

static inline bool Exists(mset_ptr T, cobj_ptr h);
static inline bool ExistsAt(mset_ptr T, cobj_ptr h, markerinfo m);
static inline obj_ptr Get(mset_ptr T, cobj_ptr h);
static inline void Insert(mset_ptr T, cobj_ptr k);
static inline void Give(mset_ptr T, cobj_ptr k);  /* Like insert, but steals the reference. */

static inline obj_ptr HashAtMarker(mset_ptr T, cobj_ptr k);
static inline bool EqualAtMarker(mset_ptr T1, mset_ptr T2, markerinfo m);

/* Deal with more than 2 tables with EqualitySetUpdate, as follows: 
 
   mset_ptr accumulator = NULL; 
   vset_ptr v;

   for (T in set of Msets) {
     accumulator = EqualitySetUpdate(accumulator, T);
   }

   v = EqualitySetFinish(accumulator);
*/

static inline vset_ptr EqualityVSet(mset_ptr T1, mset_ptr T2);
static inline mset_ptr EqualityVSetUpdate(mset_ptr accumulator, mset_ptr T);
static inline vset_ptr EqualityVSetFinish(mset_ptr accumulator);

static inline vset_ptr EqualToHash(mset_ptr T, cobj_ptr h);

/* These functions provide set operations; they are to be used the
 * same way the VSetX functions are. */

static inline mset_ptr MSetUnion(mset_ptr T1, mset_ptr T2);
static inline mset_ptr MSetUnionUpdate(mset_ptr T, mset_ptr T1);

static inline mset_ptr MSetIntersection(mset_ptr T1, mset_ptr T2);
static inline mset_ptr MSetIntersectionUpdate(mset_ptr T, mset_ptr T2);

static inline mset_ptr MSetDifference(mset_ptr T1, mset_ptr T2);

static inline mset_ptr KeySet(mset_ptr T);

static inline mset_ptr MSetReduce(mset_ptr T);

/* The summarize functions work via an intermediate accumulator, which
 * may be null (in which case one is created.  The proper way of using
 * these is as follows:
 
 mset_ptr accumulator = NULL;
 for (T in collection) {
    accumulator = Summarize_Add(accumulator, T);
 }

 summary = Summarize_Finish(accumulator);
 */

static inline mset_ptr SummarizeUpdate(mset_ptr accumulator, mset_ptr ht);
static inline mset_ptr SummarizeFinish(mset_ptr accumulator);


#endif /* _HASHREDUCE_INLINE_H_ */
