/***************************************************
 * The HashObject data structure is the key component within the
 * ibdgraph library.  It is the base structure for all items stored in
 * hash tables, hash collections, etc.
 *
 * Manipulations on hash key structures is provided.  These include
 * forming new hash keys as combinations of other hash keys.
 * 
 * Standardized Types
 * ===================
 *
 * Every HashObject has an assigned type as definied by a structure of
 * functions and flags that determine how it behaves and define
 * particular operations such as deleting the structure, recomputing
 * the hash function, adding items, deleting items, etc.  If these
 * pointers are zero, then they are ignored and the corresponding
 * operations are not allowed for the data structure.
 *
 *
 * Memory management
 * ===================
 *
 * There are two modes of memory management available for hash key
 * items.  The first is the ownership mode, and the second is a
 * "hands-off" mode.  
 *
 * Under the ownership model, it is assumed that the hashkey
 * infrastructure owns the hashkey item, and the item is kept alive
 * only as long as it is contained within a hashkey based data
 * structure.  Internally, refcounting is used to handle this, with an
 * object being garbage collected by a specified function when the
 * refcount hits zero.  In this model, manually freeing a hashkey
 * structure will likely cause a segfault.
 * 
 * Under the hands-off model, no memory management is performed, and
 * it is up to the user of these data structures to ensure that given
 * pointers are valid as long as they are contained in a hashkey data
 * structure.  
 *
 **************************************************/

#ifndef HASHOBJECTS_H
#define HASHOBJECTS_H

#include "object.h"
#include "markerinfo.h"
#include "errorhandling.h"
#include "hashkeys.h"

#include <stdbool.h>
#include <stdlib.h>

#define HASHOBJECT_ITEMS						\
									\
    /* Declare the base structure object items; */			\
    OBJECT_ITEMS;							\
    									\
    /* Hash field components. */					\
    HashKey __hashkey;							\
									\
    /* A counter of all the things (e.g. hash tables) that lock this	\
     * structure. */							\
    size_t marker_lock_count;						\
									\
    /* Marker Info stuff; possibly null denoting invariance. */		\
    MarkerInfo *mi
    
/********************************************************************************
 *
 *  The basic hash object structure.  
 *
 ********************************************************************************/

typedef struct {
    HASHOBJECT_ITEMS;
} HashObject;

DECLARE_OBJECT(HashObject);

HashObject* NewHashObject();
 
/********************************************************************************
 *
 *  Basic hash object operations.
 *
 ********************************************************************************/

/* Equality testing between any two objects with the hash field. */
bool H_Equal(cobj_ptr x, cobj_ptr y);
static inline bool H_EQUAL(cobj_ptr x, cobj_ptr y);

/* Clear the hash (set to 00000000000....), drops marker info. */
void H_Clear(obj_ptr x);
static inline void H_CLEAR(obj_ptr x);

/* Copies the hash and marker information over. */
HashObject* H_Copy(obj_ptr dest_key, cobj_ptr src);
static inline HashObject* H_COPY(obj_ptr dest_key, cobj_ptr hk);

/* These throw away the marker information. */
HashObject* H_CopyAsUnmarked(obj_ptr dest_key, cobj_ptr src);
static inline HashObject* H_COPY_AS_UNMARKED(obj_ptr dest_key, cobj_ptr hk);

/********************************************************************************
 *
 *  Locking operations
 *
 * The following are debug routines and enabled only when compiled in
 * debug mode.  It ensures that a hash object's marker information
 * does not change in one hash table when it is part of a hash table.
 * These checks don't happen when not in debug mode. 
 *********************************************************************************/ 

static inline bool H_MarkerIsLocked(cobj_ptr x);

/* Claims a lock on the object.  This simply increments the lock count
 * and a reference. */
static inline void H_ClaimMarkerLock(obj_ptr x);

/* Releases a lock on the object and decrements a reference.  When the
 * lock count gets down to 0, it can be manipulated. */
static inline void H_ReleaseMarkerLock(obj_ptr x);

/* Returns the number of objects holding a lock on that object. */
static inline size_t H_MarkerLockCount(cobj_ptr x);


/*********************************************************************************
 *
 *  Functions for filling the hash keys with values.
 *
 *********************************************************************************/

/* All of these functions fill the destination object with hash keys
 * of a certain type.  
 *
 * All functions return a pointer the hash key.  If dest_key is NULL,
 * then a new hash key is created and filled. 
 *
 */

/* Create a new hash key which is an order dependent combination of
 * two other hash keys. */
HashObject* Hf_Combine(obj_ptr dest_key, cobj_ptr h1, cobj_ptr h2);

/* Create field from string. */
HashObject* Hf_FromString(obj_ptr dest_key, const char *string);

/* Create field from char buffer (just like a string, except length is
 * a set paraemter. */
HashObject* Hf_FromCharBuffer(obj_ptr dest_key, const char *string, size_t length);

/* From different numerical values. */
HashObject* Hf_FromInt(obj_ptr dest_key, signed long x);
HashObject* Hf_FromUnsignedInt(obj_ptr dest_key, unsigned long x);

/* Take the hash of a single hash key. */
HashObject* Hf_FromHashObject(obj_ptr dest_key, cobj_ptr src);

/* Copy exact hash from another key. */

HashObject* Hf_CopyFromKey(obj_ptr dest_key, chk_ptr hk);
static inline HashObject* Hf_COPY_FROM_KEY(obj_ptr dest_key, chk_ptr hk);

/*********************************************************************************
 * 
 *  Operations between hash objects.  Note that these simply operate
 *  on the hash of the dest_key.  If dest_key is NULL, a new hash
 *  object is created and returned, otherwise dest_key is returned.
 *
 *  These atomic operations are merely wrappers around the same
 *  operations on HashKey in hashkey.h.
 *
 *********************************************************************************/

/***** Order independent combining -- the reduce function. *****/
HashObject* H_Reduce(obj_ptr dest_key, cobj_ptr h1, cobj_ptr h2);
HashObject* H_ReduceUpdate(obj_ptr dest_key, cobj_ptr h);

/****** Rehashing ******/
HashObject*  H_Rehash(obj_ptr dest_key, cobj_ptr h);
HashObject*  H_InplaceRehash(obj_ptr h);

/***** Negatives *****/
HashObject* H_Negative(obj_ptr dest_key, cobj_ptr h);
HashObject* H_InplaceNegative(obj_ptr h);

/********************************************************************************
 *
 *  HashObject marker information manipulation
 *
 ********************************************************************************/

static inline MarkerInfo* H_Mi(cobj_ptr h);

bool H_IsMarked(cobj_ptr x);
static inline bool H_IS_MARKED(cobj_ptr x);

void H_AddMarkerValidRange(obj_ptr x, markertype r_start, markertype r_end);
static inline void H_ADD_MARKER_VALID_RANGE(obj_ptr x, markertype r_start, markertype r_end);

void H_RemoveMarkerValidRange(obj_ptr x, markertype r_start, markertype r_end);
static inline void H_REMOVE_MARKER_VALID_RANGE(obj_ptr x, markertype r_start, markertype r_end);

bool H_MarkerPointIsValid(cobj_ptr x, markertype m);

void H_ClearMarkerInfo(obj_ptr x);
static inline void H_CLEAR_MARKER_INFO(obj_ptr x);

/* Steals the refernce for mi */
void H_GiveMarkerInfo(obj_ptr x, MarkerInfo *mi);
static inline void H_GIVE_MARKER_INFO(obj_ptr x, MarkerInfo *mi);

/* Increments the refernce for mi */
void H_SetMarkerInfo(obj_ptr x, MarkerInfo *mi);
static inline void H_SET_MARKER_INFO(obj_ptr x, MarkerInfo *mi);

/* Steals the refernce for mi */
void H_GiveMarkerInfo(obj_ptr x, MarkerInfo *mi);

markertype H_RangeMin(cobj_ptr x);

markertype H_RangeMax(cobj_ptr x);

/********************************************************************************
 *
 * Methods for accessing the hash keys of objects.
 *
 * These methods should always be used as they check debug flags when
 * debug mode is on.  There are readonly (RO) and read-write (RW)
 * versions; when only read-only access is needed, use RO.
 *
 ********************************************************************************/

static inline const HashKey* H_Hash_RO(cobj_ptr x);
static inline HashKey* H_Hash_RW(obj_ptr x);

/********************************************************************************
 *
 *  Debug functions; shouldn't need to be used outside the library
 *
 ********************************************************************************/

HashObject* Hf_FillExact(obj_ptr dest_key, const char *hash);
HashObject* Hf_FillFromComponents(obj_ptr dest_key, unsigned int a, unsigned int b, 
				  unsigned int c, unsigned int d);

/* Extract the 4 32bit values associated with the hash keys, pos = {0,1,2,3} */
unsigned long H_ExtractHashComponent(cobj_ptr x, unsigned int pos);

/* Fills the dest string with a hex representation of the given hash. */
void H_ExtractHash(char* dest_string, cobj_ptr x);

const char * H_HashAs8ByteString(cobj_ptr x);

void H_debug_print(cobj_ptr x);
void H_print(cobj_ptr x);

/* Add in the definitions for the static member functions.*/
#include "hashobject_inline.h"

#endif
