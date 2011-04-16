#ifndef _HASHREDUCE_H_
#define _HASHREDUCE_H_

/********************************************************************************
 *
 *   The building-block functions as described in paper are declared
 *   here.  In general, these are simple wrappers for functions
 *   described elsewhere in the library, but declared here to provide
 *   an API consistent with the paper.  Many are simple wrappers so
 *   there should be no penalty to using these. 
 *
 *
 *
 *
 *   Note: To create / fill hash objects and manipulate their marker
 *   ranges, see methods from hashobject.h.  To work with marker
 *   ranges, see markerrange.h.  The rest of the atomic methods
 *   outlined in the paper are declared here.
 *
 *   
 * 
 *
 ********************************************************************************/

#include "hashobject.h"
#include "hashtable.h"
#include "markerinfo.h"

typedef mi_ptr vset_ptr;
typedef cmi_ptr cvset_ptr;


/********************************************************************************
 *
 *  Validity set operations.
 *
 ********************************************************************************/

bool  IsValid(cobj_ptr h, markertype m);

vset_ptr ViewVSet(cobj_ptr h);
vset_ptr GetVSet(cobj_ptr h);
void  SetVSet(obj_ptr h, vset_ptr mi);

void  AddVSetInterval(obj_ptr h, markerinfo a, markerinfo b);
void  ClearVSetInterval(obj_ptr h, markerinfo a, markerinfo b);

markerinfo VSetMin(cobj_ptr v);
markerinfo VSetMax(cobj_ptr v);

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

vset_ptr VSetUnion(cobj_ptr v1, cobj_ptr v2);
vset_ptr VSetUnionUpdate(vset_ptr v_dest, cobj_ptr v2);

vset_ptr VSetIntersection(cobj_ptr v1, cobj_ptr v);
vset_ptr VSetIntersectionUpdate(vset_ptr v_dest, cobj_ptr v);

vset_ptr Difference(cobj_ptr v1, cobj_ptr v2);

/********************************************************************************
 *
 *  Operations involving Msets and their elements.  Note that here,
 *  querying is done by using hash objects; the keys are the only
 *  thing used in the query (except for the insert function, which
 *  inserts that key). This is done for memory management purposes.
 *
 ********************************************************************************/

typedef HashTable* mset_ptr;
typedef HashSequence* hash_accumulator;

bool Contains(mset_ptr T, obj_ptr h);
bool ContainsAt(mset_ptr T, obj_ptr h, markerinfo m);
obj_ptr Get(mset_ptr T, obj_ptr h);
void Insert(mset_ptr T, obj_ptr k);
void Give(mset_ptr T, obj_ptr k);  /* Like insert, but steals the reference. */

obj_ptr HashAtMarker(mset_ptr T, obj_ptr k);
obj_ptr MSetHash(mset_ptr T);  /* Returns a hash of everything. */

bool EqualAtMarker(mset_ptr T1, mset_ptr T2, markerinfo m);

/* Deal with more than 2 tables with EqualitySetUpdate, as follows: 
 
   hash_accumulator accumulator = NULL; 
   vset_ptr v;

   for (T in set of Msets) {
     accumulator = EqualitySetUpdate(accumulator, T);
   }

   v = EqualitySetFinish(accumulator);
*/

vset_ptr EqualityVSet(mset_ptr T1, mset_ptr T2);
hash_accumulator EqualityVSetUpdate(hash_accumulator accumulator, mset_ptr T);
vset_ptr EqualityVSetFinish(hash_accumulator accumulator);

/* Returns a validity set on which the hash is equal to the given hash. */

vset_ptr EqualToHash(mset_ptr T, cobj_ptr h);

/* These functions provide set operations; they are to be used the
 * same way the VSetX functions are.  E.g.
*/

mset_ptr MSetUnion(mset_ptr T1, mset_ptr T2);
mset_ptr MSetUnionUpdate(mset_ptr T, mset_ptr T1);

mset_ptr MSetIntersection(mset_ptr T1, mset_ptr T2);
mset_ptr MSetIntersectionUpdate(mset_ptr T, mset_ptr T2);

mset_ptr MSetDifference(mset_ptr T1, mset_ptr T2);

mset_ptr KeySet(mset_ptr T);

mset_ptr MSetReduce(mset_ptr T);

/* The summarize functions work via an intermediate accumulator, which
 * may be null (in which case one is created.  The proper way of using
 * these is as follows:
 
 mset_ptr accumulator = NULL;
 for (T in collection) {
    accumulator = Summarize_Add(accumulator, T);
 }

 summary = Summarize_Finish(accumulator);
 */

mset_ptr SummarizeUpdate(mset_ptr accumulator, mset_ptr ht);
mset_ptr SummarizeFinish(mset_ptr accumulator);

#endif /* _HASHREDUCE_H_ */
