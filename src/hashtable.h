/***************************************************
 * The central data structure for manipulating and running things
 * within this library.  It is accessed through a collection of macros
 * and inline functions for speed.  Also, set operations are also
 * supported.
 *
 * The hash table, given a HashComponent structure, returns another
 * structure that inherits from HashComponent; i.e. the first part of
 * the structure holds the HASH_COMPONENTS members.  Thus if this hash
 * table is to be used, you must add the HASH_COMPONENTS macro as the
 * first member of the structure.
 *
 * Later, a sequenced hash table works by embedding a sequence within
 * a hash table structure.  The keys in this hash table structure are
 * stored with info about their hash key and a marker range in which
 * they are valid.  
 *
 **************************************************/

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "hashobject.h"
#include "randfunctions.h"
#include "object.h"
#include "bitops.h"
#include <stdint.h>

/***************************************************
 * Internals of the hash table structure.
 **************************************************/

/*********************************************************************************
 *
 *  The hash table structure works with a linear probing type of
 *  operation.  This should give pretty good performance in this
 *  context.  We have a couple advantages here over regular hash
 *  tables.  First, 
 *
 *  Resizing is triggered 
 ********************************************************************************/

#define _HT_ITEMS_PER_NODE 4
#define _HT_INITIAL_LOG2_SIZE 3

typedef struct {
    uint64_t hk64;
    HashObject *obj;
} _HT_Item;

struct _HT_Independent_Node_type;

/* Keep this in size to a byte aligned boundary. */
typedef struct _HT_Node_type {
    size_t size;
    struct _HT_Independent_Node_type *next_chain;
    _HT_Item items[_HT_ITEMS_PER_NODE];
} _HT_Node;

typedef _HT_Node * _restrict_  _ht_node_rptr;
typedef const _HT_Node * _restrict_  _ht_node_crptr;

typedef struct _HT_Independent_Node_type {
    MEMORY_POOL_ITEMS;
    _HT_Node node;
} _HT_Independent_Node;

DECLARE_GLOBAL_MEMORY_POOL(_HT_Independent_Node);

/************************************************************
 * We use a skip list for handling the range markers; these structures
 * are here.  These are embedded into the hash table
 ************************************************************/

#define _HT_MSL_MAX_LEVELS 16

#ifdef DEBUG_MODE
#define _HT_MSL_NODE_DEBUG_ITEMS \
    unsigned int level;		 \
    bool is_leaf;		 \
    bool is_branch;
#else
#define _HT_MSL_NODE_DEBUG_ITEMS 
#endif

#define _HT_MSL_NODE_ITEMS			\
    MEMORY_POOL_ITEMS;				\
    markertype marker;				\
    HashKey hk;					\
    _HT_MSL_NODE_DEBUG_ITEMS;			\
    struct _HT_MSL_Node_t *next

typedef struct _HT_MSL_Node_t {
    _HT_MSL_NODE_ITEMS;
} _HT_MSL_Node;

typedef struct {
    _HT_MSL_NODE_ITEMS;
    _HT_MSL_Node *down;
} _HT_MSL_Branch;

typedef struct {
    _HT_MSL_NODE_ITEMS;
} _HT_MSL_Leaf;

typedef struct {
    MEMORY_POOL_ITEMS;
    _HT_MSL_Leaf  *first_leaf;
    _HT_MSL_Branch *start_node;
    unsigned int start_node_level;
    uint32_t cur_rand_factor;
    LCGState cur_rand_state;
} _HT_MarkerSkipList;

typedef struct _HT_MSL_NodeStack_t {
    MEMORY_POOL_ITEMS;
    _HT_MSL_Node *node;
    struct _HT_MSL_NodeStack_t *previous;
    bool is_travel_node;
} _HT_MSL_NodeStack;

/*************************************************
 * The hash table structure and various wrappers.
 **************************************************/

typedef struct {
    OBJECT_ITEMS;
    size_t size;

    _HT_Node *table;
    size_t first_element;
    size_t _table_size;
    size_t _table_grow_trigger_size;
    unsigned int _table_shift;
    unsigned int _table_log2_size;

    /* The marker skip list; may be null. */
    _HT_MarkerSkipList *marker_sl;
} HashTable;

DECLARE_OBJECT(HashTable);

typedef HashTable * ht_ptr;
typedef HashTable * _restrict_ ht_rptr;
typedef const HashTable * _restrict_ ht_crptr;

/********************************************************************************
 *
 *  Functions for simply dealing with ranges of hashes
 *
 ********************************************************************************/

#define _HS_NODE_SIZE 2

/* Use a rope-like structure for easy manipulation. */

typedef struct {
    markertype marker;
    HashKey hk;
} _HS_Item;

typedef struct _HS_Node_t {
    MEMORY_POOL_ITEMS;
    size_t size;
    struct _HS_Node_t *next;
    _HS_Item items[_HS_NODE_SIZE];
} _HS_Node;

DECLARE_GLOBAL_MEMORY_POOL(_HS_Node);

typedef struct {
    OBJECT_ITEMS;
    _HS_Node* end_node;
    _HS_Node nodes;
    size_t size;
} HashSequence;

DECLARE_OBJECT(HashSequence);

/*********************************************************************************
 *
 *  Functions for creating and destroying hash tables.
 *
 *********************************************************************************/

HashTable* NewHashTable();

/* Note that the above macros provides NewHashTable() and NewLargeHashTable. */
HashTable* NewSizeOptimizedHashTable(size_t expected_size);

/********************************************************************************
 *
 *  Functions for filling the hash of and copying the hash table.
 *
 ********************************************************************************/

/* void Ht_FillHash(HashTable* ht); */
HashTable* Ht_Copy(ht_crptr ht); 

/********************************************************************************
 *
 *  Functions for operating on the hash table.
 *
 ********************************************************************************/

/* Ht_get Returns the given key if it's present in the hash table; if
 * it is not preset, NULL is returned.  This operation is invariant to
 * marked ranges, all possible marker values are considered valid.
 * Ht_Get() adds a reference count which the user must then release.
 * Ht_View assumes it is safe to share the reference held by the hash
 * table.
 */
HashObject* Ht_Get(ht_crptr ht, const HashObject *hk);
HashObject* Ht_GetByKey(ht_crptr ht, HashKey hk);

HashObject* Ht_View(ht_crptr ht, const HashObject *hk);
inline HashObject* Ht_ViewByKey(ht_crptr ht, HashKey hk);

/* Finds the given key in the hash table, deletes it from the tree and
 * returns it.  The caller would then own a reference to the item.
 * 
 */
HashObject* Ht_Pop(ht_rptr ht, const HashObject *hk);
HashObject* Ht_PopByKey(ht_rptr ht, HashKey hk);


/* Deletes the given key from the hash table and decrements its
 * refcount.  Returns true if the key was present and false
 * otherwise. All marker ranges are removed; use Ht_ClearMarkerRange()
 * to clear specific ranges.
 */
bool Ht_Clear(ht_rptr ht, const HashObject *hk);
bool Ht_ClearByKey(HashTable *ht, HashKey hk);

/* Adds the given key to the hash table.  Overwrites the key if it is
 * present already.  Any marker information of the key is ignored.
 *
 * This function claims one new reference count on the hashkey.  If
 * the hashtable is deleted, the reference count will not be
 * changed. If the user does not wish to use the key after setting it
 * in the hash table, use Ht_Give() instead.
 */
void Ht_Set(ht_rptr ht, HashObject *hk);

/* Same as Ht_Set, except that it is assumed that the reference
 * claimed by the user is being given over to the hash table.  Use the
 * hash table is deleted, the hash key reference count will be
 * decremented.
 */
void Ht_Give(ht_rptr ht, HashObject *hk);

/* Adds the given key to the hash table, if it is not already present.
 * If the given key is already present, then it is left alone.  The
 * HashObject remaining in the tree is returned.  Any marker information
 * is ignored.
 */
HashObject* Ht_SetDefault(HashTable *ht, HashObject *hk);

/* Returns the size of the given hash table. Note that a macro
 * version, Ht_SIZE, is also provided.*/
size_t Ht_Size(ht_crptr ht);
static inline size_t Ht_SIZE(ht_crptr ht);

/* Returns true if the hash table contains the given key and false
 * otherwise.  Note: the computational cost of this query is identical
 * to that of retrieving; if the resulting key is used, it is better
 * to attempt a retrieval and test for null.  
 */
bool Ht_Contains(ht_crptr ht, const HashObject *hk);
bool Ht_ContainsByKey(ht_crptr ht, HashKey hk);

/* A convenience method. Returns true if the object is in the hash
 * table and is valid at a certain marker point. */
bool Ht_ContainsAt(ht_crptr ht, const HashObject *hk, markertype m);
bool Ht_ContainsAtByKey(ht_crptr ht, HashKey hk, markertype m);

/* This adds a new valid range [r_start, r_end) to an existing key, if
 * it exists.  Otherwise, it sets the marker range of hk to [r_start,
 * r_end) and inserts it.  A pointer to whichever one was inserted is
 * returned.  
 */

HashObject* Ht_InsertValidRange(ht_rptr ht, HashObject *hk, 
				markertype r_start, markertype r_end);

/********************************************************************************
 *
 *  Operations involving marked objects. 
 *
 ********************************************************************************/

/* Returns/fills a hash key at the given point in the hash table.
 * This is an accurate hash of the state of the HashTable at that
 * point and can be used for equality testing and the like.
 */

HashObject* Ht_HashAtMarkerPoint(HashObject *h_dest, ht_rptr ht, markertype m);

HashObject* Ht_HashOfMarkerRange(HashObject *h_dest, HashTable *ht, 
				 markertype start, markertype end);

HashObject* Ht_HashOfEverything(HashObject *h_dest, ht_rptr ht);

bool Ht_EqualAtMarker(ht_rptr ht1, ht_rptr ht2, markertype m);

/* Merges graphs along each marker value.  Currently used for testing
 * whether ibd graphs are equal along nodes. Steals a reference for
 * the previous accumulator. */

HashSequence *Ht_Summarize_Update(HashSequence *ht_accumulator, ht_rptr ht);
HashTable *Ht_Summarize_Finish(HashSequence *ht_accumulator);

HashTable *Ht_ReduceTable(HashTable *ht);

/* Returns a MarkerInfo object that denotes where the marked part of a
 * hashtable is non-zero.
 */

MarkerInfo *Ht_MarkerInfoUnion(HashTable *ht);

/* Returns a marker info set that tells where the two (or more) tables
 * are equal.
 */

MarkerInfo* Ht_EqualitySet(HashTable *ht1, HashTable *ht2);

HashSequence* Ht_EqualitySetUpdate(HashSequence *accumulator, HashTable *ht);

MarkerInfo* Ht_EqualitySetFinish(HashSequence *accumulator);

MarkerInfo* Ht_EqualToHash(HashTable *ht, HashObject *h);

/* Set operations over hash tables. */ 

ht_rptr Ht_Intersection(ht_crptr ht1, ht_crptr ht2);
ht_rptr Ht_IntersectionUpdate(ht_rptr ht_accumulator, ht_crptr ht2);

ht_rptr Ht_Union(ht_crptr ht1, ht_crptr ht2);
ht_rptr Ht_UnionUpdate(ht_rptr ht_accumulator, ht_crptr ht2);

ht_rptr Ht_Difference(ht_crptr ht1, ht_crptr ht2);

/* Swaps the content of the two hash tables. */
void Ht_Swap(ht_rptr ht1, ht_rptr ht2);

/* The marker information works by using a marker skip list that is a
 * dynamically updated cache.  This cache is created as needed; it is
 * not used until functions using reduce operations are called.
 * However, the marker information on the keys can't be changed when
 * this cache is present.  Call this function to ensure that this
 * cache is not present, allowing marker keys to be changed
 * quickly. */
static inline void Ht_ClearMarkerCache(ht_rptr ht);

/********************************************************************************
 *
 *  Iteration over the hash table. 
 *
 *********************************************************************************/

/* The following structure should never be accessed directly; only
 * through the accompaning functions.
 */

/* This first one is for internal use. */
typedef struct {
    size_t number_left;
    _ht_node_crptr current_base_node, next_node;
    unsigned int next_index;
#ifndef NDEBUG
    _ht_node_crptr debug_current_node;
    unsigned int debug_current_index;
    ht_crptr ht;
#endif
} _HashTableInternalIterator;

typedef struct {
    _HashTableInternalIterator hti;
    ht_rptr ht;
} HashTableIterator;

/* Creates a new iterator.  Calling next on a new iterator returns the
 * first element. */
HashTableIterator* Hti_New(ht_rptr ht);

/* Iterates through the hash key structure; returns false AFTER the
 * last element has been returned (still true on last element). */
bool Hti_Next(HashObject** h_dest, HashTableIterator *hti);
static inline bool Hti_NEXT(HashObject** h_dest, HashTableIterator *hti);

/* Deletes the current iterator.  This needs to be called after every
 * iteration has finished.  */
void Hti_Delete(HashTableIterator *hti);

typedef struct {
    HashObject **next;
    size_t left; 
    HashObject **object_buffer;
    size_t size;
} HashTableBufferedIterator;

/* Creates a new iterator.  Calling next on a new iterator returns the
 * first element. */
HashTableBufferedIterator* Htib_New(ht_crptr ht);

/* Iterates through the hash key structure; returns NULL after the
 * last element has been returned. */
bool Htib_Next(HashObject** h_dest, HashTableBufferedIterator *htib);
static inline bool Htib_NEXT(HashObject** h_dest, HashTableBufferedIterator *htib);

/* Deletes the current iterator.  This needs to be called after every
 * iteration has finished.  */
void Htib_Delete(HashTableBufferedIterator *htib);

/**********************************************************************
 *
 *  HashSequence Stuff
 *
 **********************************************************************/

HashSequence* Hs_FromHashTable(HashTable *ht);

HashSequence* Hs_HashTableIntersectionUpdate(HashSequence* hs, HashTable *ht);

HashSequence* Hs_HashTableReduceUpdate(HashSequence* hs, HashTable *ht);

MarkerInfo* Hs_NonZeroSet(HashSequence* hs);

HashTable* Hs_ToHashTable(HashSequence* hs);

/***********************************************************************
 *
 *  Iteration over hash validity sequences. 
 *
 **********************************************************************/

typedef struct {
    HashKey hk;
    markertype start, end;
} HashValidityItem;

typedef struct {
    MEMORY_POOL_ITEMS;
    HashTable *ht;
    _HT_MSL_Node *next;
    HashValidityItem current_item;
} HashTableMarkerIterator;
    

HashTableMarkerIterator* Htmi_New(HashTable *ht);

static inline bool Htmi_NEXT(HashValidityItem* dest, HashTableMarkerIterator *htmi);

bool Htmi_Next(HashValidityItem* dest, HashTableMarkerIterator *htmi);

inline void Htmi_Finish(HashTableMarkerIterator* htmi); 


/************************************************************
 *
 * Hash sequence iterator
 *
 ************************************************************/

typedef struct {
    MEMORY_POOL_ITEMS;
    HashSequence *hs;
    _HS_Node *next_unit;
    size_t next_index;
    HashValidityItem current_item;
} HashSequenceIterator;

inline HashSequenceIterator* Hsi_New(HashSequence *hs);

inline bool Hsi_Next(HashValidityItem* dest, HashSequenceIterator* hsi);

inline void Hsi_Finish(HashSequenceIterator* hsi);

/************************************************************
 *
 *  Debug routines.
 *
 ************************************************************/

void Ht_debug_Print(HashTable *ht);
void Ht_MSL_debug_Print(HashTable *ht);
void Ht_MSL_debug_PrintNodeStack(_HT_MSL_NodeStack *ns);

#include "hashtable_inline.h"

#endif
