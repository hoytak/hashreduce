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
#include "object.h"
#include "bitops.h"
#include <stdint.h>

/***************************************************
 * Internals of the hash table structure.
 **************************************************/

#define _HT_BITMASK          0x0000000f
#define _HT_BITSIZE          4

#define _HT_LEVEL_SIZE       16 /* (1 << _HT_BITSIZE)  */
#define _HT_NUMLEVELS        32 /* (128 / _HT_BITSIZE) */

/* The hash table nodes. */
typedef struct {
    size_t count;
    
    /* We recycle the following two data items for efficient memory
     * management.
     * 
     * next: If count == 1; it points to the data hash key; if count >
     * 1; it points to the next level of nodes in the hash tree. 
     */

    bitfield node_data;

    void *next;
} _HashTableNode;

typedef struct {
    MEMORY_POOL_ITEMS;
    MarkerRange containment_range;
    _HashTableNode nodes[_HT_LEVEL_SIZE];
} _HashTableNodeBlock;

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
    HashKey unmarked_object_hash;
    _HT_MSL_Leaf  *first_leaf;
    _HT_MSL_Branch *start_node;
    unsigned int start_node_level;
    uint32_t cur_rand_factor;
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

/* We have two types of hash table; one that is small and size
 * optimized, and another that is larger and optimized for memory
 * caching and holding large amounts of data. The main difference is
 * in whether it uses a global memory pool or has its own.  The latter
 * takes more space but has much improved memory caching
 * performance. */

#define _HASH_TABLE_BASIC_COMPONENTS				\
								\
    OBJECT_ITEMS;						\
								\
    size_t count;						\
    unsigned int num_first_levels;				\
								\
    _HashTableNode *nodes;					\
    								\
    /* The skip list to handle the marker informations */	\
    _HT_MarkerSkipList *marker_sl;


/* Two versions, one optimized for memory and the other for holding
 * enormous amounts of data. */

typedef struct {
    _HASH_TABLE_BASIC_COMPONENTS;
} HashTable;

DECLARE_OBJECT(HashTable);

typedef struct {
    _HASH_TABLE_BASIC_COMPONENTS;
    
    MemoryPool node_pool;
    MemoryPool marker_branch_pool;
    MemoryPool marker_leaf_pool;
    MemoryPool marker_holli_pool;
    
} LargeHashTable;

DECLARE_OBJECT(LargeHashTable);


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

void Ht_FillHash(HashTable* ht);
HashTable* Ht_Copy(HashTable* ht); 


/********************************************************************************
 *
 *  Functions for operating on the hash table.
 *
 ********************************************************************************/

/* Ht_get Returns the given key if it's present in the hash table; if
 * it is not preset, NULL is returned.  This operation is invariant to
 * marked ranges, all possible marker values are considered valid.
 */
HashObject* Ht_Get(HashTable *ht, HashObject *hk);


/* Finds the given key in the hash table, deletes it from the tree and
 * returns it.  The caller would then own a reference to the item.
 * 
 */
HashObject* Ht_Pop(HashTable *ht, HashObject *hk);


/* Deletes the given key from the hash table and decrements its
 * refcount.  Returns true if the key was present and false
 * otherwise. All marker ranges are removed; use Ht_ClearMarkerRange()
 * to clear specific ranges.
 */
bool Ht_Clear(HashTable *ht, HashObject *hk);

/* Adds the given key to the hash table.  Overwrites the key if it is
 * present already.  Any marker information of the key is ignored.
 *
 * This function claims one new reference count on the hashkey.  If
 * the hashtable is deleted, the reference count will not be
 * changed. If the user does not wish to use the key after setting it
 * in the hash table, use Ht_Give() instead.
 */
void Ht_Set(HashTable *ht, HashObject *hk);

/* Same as Ht_Set, except that it is assumed that the reference
 * claimed by the user is being given over to the hash table.  Use the
 * hash table is deleted, the hash key reference count will be
 * decremented.
 */
void Ht_Give(HashTable *ht, HashObject *hk);

/* Adds the given key to the hash table, if it is not already present.
 * If the given key is already present, then it is left alone.  The
 * HashObject remaining in the tree is returned.  Any marker information
 * is ignored.
 */
HashObject* Ht_SetDefault(HashTable *ht, HashObject *hk);

/* Returns the size of the given hash table. Note that a macro
 * version, Ht_SIZE, is also provided.*/
size_t Ht_Size(HashTable *ht);
static inline size_t Ht_SIZE(HashTable *ht) {return ht->count;}

/* Returns true if the hash table contains the given key and false
 * otherwise.  Note: the computational cost of this query is identical
 * to that of retrieving; if the resulting key is used, it is better
 * to attempt a retrieval and test for null.  
 */
bool Ht_Contains(HashTable *ht, HashObject *hk);

/* Adds a new valid range to an existing key.  Slow for now.
 */

void Ht_AddMarkerRangeToExistingKey(HashTable *ht, HashObject *hk, markertype r_start, markertype r_end);


/********************************************************************************
 *
 *  Operations involving marked objects. 
 *
 ********************************************************************************/

/* Returns/fills a hash key at the given point in the hash table.
 * This is an accurate hash of the state of the HashTable at that
 * point and can be used for equality testing and the like.
 */

HashObject* Ht_HashAtMarkerPoint(HashObject *h_dest, HashTable *ht, markertype m);

HashObject* Ht_HashOfEverything(HashObject* h_dest, HashTable *ht);

/* A convenience method. Returns true if the object is in the hash
 * table and is valid at a certain marker point. */

bool Ht_ContainsAt(HashTable *ht, HashObject *h, markertype loc);

/********************************************************************************
 *
 *  Iteration over the hash table. 
 *
 *********************************************************************************/

/* The following structure should never be accessed directly; only
 * through the accompaning functions.
 */

typedef struct {
    HashTable *ht;
    _HashTableNode* hn_stack[_HT_NUMLEVELS];
    size_t stack_top, top_level_pos;
    size_t loc_stack[_HT_NUMLEVELS];
    size_t num_stack[_HT_NUMLEVELS];
    size_t count;
} HashTableIterator;


/* Creates a new iterator.  Calling next on a new iterator returns the
 * first element. */
HashTableIterator* Hti_New(HashTable *ht);

/* Iterates through the hash key structure; returns NULL after the
 * last element has been returned. */
HashObject *Hti_Next(HashTableIterator *hti);

/* Deletes the current iterator.  This needs to be called after every
 * iteration has finished.  */
void Hti_Delete(HashTableIterator *hti);


typedef struct {
    HashObject **object_buffer;
    size_t position,count;
} HashTableBufferedIterator;


/* Creates a new iterator.  Calling next on a new iterator returns the
 * first element. */
HashTableBufferedIterator* Htib_New(HashTable *ht);

/* Iterates through the hash key structure; returns NULL after the
 * last element has been returned. */
HashObject *Htib_Next(HashTableBufferedIterator *htib);

/* Deletes the current iterator.  This needs to be called after every
 * iteration has finished.  */
void Htib_Delete(HashTableBufferedIterator *htib);


/********************************************************************************
 *
 *  Hash Table Merging Operations.
 *
 ********************************************************************************/


/* Merges graphs along each marker value.  Currently used for testing
 * whether dgl graphs are equal along nodes. Steals a reference for the previous accumulator*/
HashTable *Ht_Summarize_Add(HashTable *ht_accumulator, HashTable *ht);
HashTable *Ht_Summarize_Finish(HashTable *ht_accumulator);
HashTable *Ht_ReduceTable(HashTable *ht);

/************************************************************
 *
 *  Debug routines.
 *
 ************************************************************/

void Ht_debug_print(HashTable *ht);
void Ht_MSL_debug_Print(HashTable *ht);
void Ht_MSL_debug_PrintNodeStack(_HT_MSL_NodeStack *ns);



#endif
