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
 **************************************************/

#include "hashtable.h"
#include "mtrandbasic.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/* Uncomment this line to run internal consistency checks on the hash
 * table structure for debugging purposes. */

/************************************************************
 *
 *  Part of the structure for the hash tables
 *
 ************************************************************/

/* Global memory pools for the small style hash table elements. */
static MemoryPool common_ht_node_pool = 
    STATIC_MEMORY_POOL_VALUES(sizeof(_HashTableNodeBlock));

static MemoryPool ht_msl_pool =		
    STATIC_MEMORY_POOL_VALUES(sizeof(_HT_MarkerSkipList));

static MemoryPool common_ht_msl_branch_pool = 
    STATIC_MEMORY_POOL_VALUES(sizeof(_HT_MSL_Branch));

static MemoryPool common_ht_msl_leaf_pool = 
    STATIC_MEMORY_POOL_VALUES(sizeof(_HT_MSL_Leaf));

static MemoryPool ht_msl_node_stack_pool = 
    STATIC_MEMORY_POOL_VALUES(sizeof(_HT_MSL_NodeStack));

#define _HT_IS_LARGE(ht) O_IsExactType(LargeHashTable, (ht))

#define _HT_NODE_POOL(ht)				\
    ( _HT_IS_LARGE(ht)					\
      ? (&( ((LargeHashTable*)(ht))->node_pool))	\
      : (&common_ht_node_pool))

#define _HT_MSL_BRANCH_POOL(ht)					\
    ( _HT_IS_LARGE(ht)						\
      ? (&( ((LargeHashTable*)(ht))->marker_branch_pool))	\
      : (&common_ht_msl_branch_pool))

static inline _HT_MSL_Branch* _HT_MSL_AsBranch(_HT_MSL_Node *n)
{
    assert(n == NULL || (n->is_branch && (!n->is_leaf)));
    return (_HT_MSL_Branch*)n;
}

#define _HT_MSL_LEAF_POOL(ht)					\
    ( _HT_IS_LARGE(ht)						\
      ? (&( ((LargeHashTable*)(ht))->marker_leaf_pool))		\
      : (&common_ht_msl_leaf_pool))

static inline _HT_MSL_Leaf* _HT_MSL_AsLeaf(_HT_MSL_Node *n)
{
    assert(n == NULL || ((!n->is_branch) && n->is_leaf));
    return (_HT_MSL_Leaf*)n;
}

#define _HT_FIRST_LEVEL_SIZE(ht)  (1 << ((ht)->num_first_levels *_HT_BITSIZE))

static inline void __Ht_ConstructBasic(HashTable *ht, int num_levels)
{
    ht->num_first_levels = num_levels;

    size_t node_size = _HT_FIRST_LEVEL_SIZE(ht);

    ht->nodes = (_HashTableNode*)malloc(node_size*sizeof(_HashTableNode));
    MEMCHECK(ht->nodes);
    memset(ht->nodes, 0, node_size*sizeof(_HashTableNode));
}

void _Ht_HashTable_Constructor(HashTable *ht)
{
    /* Just set it up for the first lavel */
    __Ht_ConstructBasic(ht, 1);
}

void _Ht_LargeHashTable_Constructor(LargeHashTable *lht)
{
    __Ht_ConstructBasic((HashTable*)lht, 2);
    
    MP_Init(&lht->node_pool, sizeof(_HashTableNodeBlock));
    MP_Init(&lht->marker_branch_pool, sizeof(_HT_MSL_Branch));
    MP_Init(&lht->marker_leaf_pool, sizeof(_HT_MSL_Leaf));
}

HashTable* NewHashTable()
{
    return ConstructHashTable();
}

/* A specific constructor for custom constructions.*/
HashTable* NewSizeOptimizedHashTable(size_t expected_size)
{
    if(expected_size <= 16*4)
    {
	HashTable* ht = ALLOCATEHashTable();
	__Ht_ConstructBasic(ht, 1);
	return ht;
    }
    else
    {
	LargeHashTable* lht = ALLOCATELargeHashTable();

	if(expected_size <= 16*16*4)
	    __Ht_ConstructBasic((HashTable*)lht, 2);
	else if(expected_size <= 16*16*16*4)
	    __Ht_ConstructBasic((HashTable*)lht, 3);
	else if(expected_size <= 16*16*16*16*4)
	    __Ht_ConstructBasic((HashTable*)lht, 4);
	else
	    __Ht_ConstructBasic((HashTable*)lht, 5);
	
	MP_Init(&lht->node_pool, sizeof(_HashTableNodeBlock));
	MP_Init(&lht->marker_branch_pool, sizeof(_HT_MSL_Branch));
	MP_Init(&lht->marker_leaf_pool, sizeof(_HT_MSL_Leaf));

	return (HashTable*)lht;
    }
}

/* Destructor method; called when the table is no longer needed. */
void _Ht_Destroy(HashTable *ht)
{
    /* First iterate over all the nodes and decref them. */
    
    HashTableIterator *hti = Hti_New(ht);
    HashObject *hk;

    while( (hk = Hti_Next(hti) ) != NULL)
    {
	H_RELEASE_LOCK(hk);
	O_DECREF(hk);
    }
    
    Hti_Delete(hti);
    
    /* Get rid of the first level */
    free(ht->nodes);

    /* Clear the marker skip list. */
    if(_HT_IS_LARGE(ht))
    {
	LargeHashTable *lht = (LargeHashTable*)ht;
	
	MP_Clear(&lht->node_pool);
	MP_Clear(&lht->marker_branch_pool);
	MP_Clear(&lht->marker_leaf_pool);

	if(ht->marker_sl != NULL)
	    MP_ItemFree(&ht_msl_pool, ht->marker_sl);
    }
    else if(ht->marker_sl != NULL)
    {
	
	/* Walk all the structures releasing them back to the common
	 * memory pools. */
	
	_HT_MarkerSkipList *msl = ht->marker_sl;

	if(ht->marker_sl != NULL)
	{

	    /* Clear all the branches. */

	    for(;msl->start_node_level != 0; --msl->start_node_level) 
	    {
		assert(msl->start_node != NULL);

		_HT_MSL_Node *br = (_HT_MSL_Node*)(msl->start_node);

		do{
		    _HT_MSL_Node *old_br = br;
		    br = br->next;
		    		
		    MP_ItemFree(&common_ht_msl_branch_pool, old_br);
		}while(br != NULL);
		    
		msl->start_node = (_HT_MSL_Branch*) msl->start_node->down;
	    }
 
	    /* Now clear all the nodes. */

	    assert(msl->first_leaf != NULL);

	    _HT_MSL_Leaf *leaf = msl->first_leaf;
	    
	    do{
		_HT_MSL_Leaf *old_leaf = leaf;
		leaf = (_HT_MSL_Leaf*)leaf->next;
		MP_ItemFree(&common_ht_msl_leaf_pool, old_leaf);
	    }while(leaf != NULL);

	    /* Finally clear the msl itself. */
	    MP_ItemFree(&ht_msl_pool, ht->marker_sl);
	}
    }
}

DEFINE_OBJECT(
    /* Name. */     HashTable,
    /* BaseType */  Object,
    /* construct */ _Ht_HashTable_Constructor,
    /* delete */    _Ht_Destroy,
    /* Duplicate */ NULL);

DEFINE_OBJECT(
    /* Name. */     LargeHashTable,
    /* BaseType */  HashTable,
    /* construct */ _Ht_LargeHashTable_Constructor,
    /* delete */    _Ht_Destroy,
    /* Duplicate */ NULL);

/************************************************************
 * Functions for managing internal aspects of the hash table,
 * excluding individual operations on the nodes.
 ************************************************************/

/* Node Blocks */
static inline _HashTableNodeBlock* _Ht_newNodeBlock(HashTable* ht)
{
    _HashTableNodeBlock* hnb = (_HashTableNodeBlock*)MP_ItemMalloc(_HT_NODE_POOL(ht));

    size_t i;
    
    for(i = 0; i < _HT_LEVEL_SIZE; ++i)
	assert(hnb->nodes[i].count == 0);

    return hnb;
}

static inline void _Ht_freeNodeBlock(HashTable *ht, _HashTableNodeBlock* hnb)
{
    MP_ItemFree(_HT_NODE_POOL(ht), hnb);
}

/* Marker Branch Items */
static inline _HT_MSL_Branch* _Ht_newMarkerBranch(HashTable* ht)
{
    _HT_MSL_Branch *br = (_HT_MSL_Branch*)MP_ItemMalloc(_HT_MSL_BRANCH_POOL(ht));
#ifdef DEBUG_MODE
    br->is_branch = true;
    br->is_leaf = false;
#endif
    return br;
}

static inline void _Ht_freeMarkerBranch(HashTable *ht, _HT_MSL_Node* mb)
{
    MP_ItemFree(_HT_MSL_BRANCH_POOL(ht), mb);
}

/* Marker Leaf Items */
static inline _HT_MSL_Leaf* _Ht_newMarkerLeaf(HashTable* ht)
{
    _HT_MSL_Leaf *leaf = (_HT_MSL_Leaf*)MP_ItemMalloc(_HT_MSL_LEAF_POOL(ht));
#ifdef DEBUG_MODE
    leaf->is_branch = false;
    leaf->is_leaf = true;
#endif
    return leaf;
}

static inline void _Ht_freeMarkerLeaf(HashTable *ht, _HT_MSL_Leaf* mb)
{
    MP_ItemFree(_HT_MSL_LEAF_POOL(ht), mb);
}


/************************************************************
 *
 * Boolean flag tests for the functions. 
 *
 ************************************************************/

static inline bool _Ht_HasMarkedItems(HashTable *ht)
{
    return (ht->marker_sl != NULL);
}


/*************************************************************
 *
 * Functions for retriving nodes in the hash table.
 *
 ************************************************************/

static inline void _Hn_overwriteKeyInNode(_HashTableNode* hn, HashObject* restrict old_hk, 
					  HashObject* restrict new_hk)
{
    O_DECREF(old_hk);
    O_INCREF(new_hk);
    hn->next = (void*)new_hk;
}

static inline int _H_extractOffset(HashObject* hk, size_t level)
{
    int bitshift, local_bitshift, component;

    bitshift = level * _HT_BITSIZE;
    
    if(H_64BIT)
    {
	component = (bitshift >> 6);              /* bitshift / 64 */
	local_bitshift = (bitshift & 0x0000003f); /* bitshift % 64 */ 
    }
    else
    {
	component = (bitshift >> 5);   /* bitshift / 32 */
	local_bitshift = (bitshift & 0x0000001f); /* bitshift % 32 */ 
    }
    
    return ((H_Hash_RO(hk)->hk[component]) >> local_bitshift) & _HT_BITMASK;
}

/*****************************************
 * Optimizations for optimizing iterators.
 ****************************************/

#define HNBO_COUNT_BIT        0
#define HNBO_COUNT_SIZE       5

#define HNBO_FIRST_NODE_BIT   5
#define HNBO_FIRST_NODE_SIZE  4

#define HNBO_NODEMASK_BIT    16

#define _HNBO_localCount(hn) (getBitGroup((hn)->node_data, HNBO_COUNT_BIT, HNBO_COUNT_SIZE))
#define _HNBO_nextNode(hn, cur_node) (getFirstBitOnFromPos((hn)->node_data, (cur_node) + HNBO_NODEMASK_BIT) - HNBO_NODEMASK_BIT)
#define _HNBO_firstOffset(hn)  (getBitGroup((hn)->node_data, HNBO_FIRST_NODE_BIT, HNBO_FIRST_NODE_SIZE) )
#define _HNBO_nodeOn(hn, offset) (bitOn((hn)->node_data, (offset) + HNBO_NODEMASK_BIT))

static inline void _HNBO_setNodeOn(_HashTableNode *hn, int offset)
{
    /* The structure is packed so that:
     *
     * Bits 0-4 give the current number of nodees.
     *
     * Bits 5-8 give the index of the first nonzero entry in the
     * node table.
     *
     * Bits 16-31 give a bitmask representation of which nodes have
     * nodees and which do not.
     * 
     */

    assert(offset < 16);

    if(!_HNBO_nodeOn(hn, offset))
    {
	assert(_HNBO_localCount(hn) < 16);
	
	bool is_new_node = (_HNBO_localCount(hn) == 0);
	   
	++hn->node_data;
	
	if(is_new_node || _HNBO_firstOffset(hn) > offset)
	    setBitGroup(hn->node_data, HNBO_FIRST_NODE_BIT, HNBO_FIRST_NODE_SIZE, offset);
	
	setBitOn(hn->node_data, offset + HNBO_NODEMASK_BIT);
    }
}

void _HNBO_printOffset(_HashTableNode *hn)
{    
    printf("local_count=%lu, firstOffset=%lu, mask=", _HNBO_localCount(hn), _HNBO_firstOffset(hn));

    int j;

    for(j = 0; j < 16; ++j)
    {
	if(_HNBO_nodeOn(hn, j))
	    printf("%x,", j);
    }
    
    printf("\n");
}


static inline void _HNBO_delNode(_HashTableNode *hn, size_t offset)
{
    assert(_HNBO_nodeOn(hn, offset));
    assert(_HNBO_localCount(hn) >= 1);
    
    setBitOff(hn->node_data, offset + HNBO_NODEMASK_BIT);
    
    --hn->node_data;
    
    /* See if we need to reset the first node index. */
    if(_HNBO_firstOffset(hn) == offset )
	setBitGroup(hn->node_data, HNBO_FIRST_NODE_BIT, HNBO_FIRST_NODE_SIZE, 
		    getFirstBitOnFromPos(hn->node_data, offset + 1 + HNBO_NODEMASK_BIT));
}

/****************************************
 * The main routine for retrieving and setting items in the hash tree.
 ****************************************/

void _Ht_MSL_Init(HashTable *ht);
void _Ht_MSL_WriteKey_Marked(HashTable *ht, HashObject *h, bool);
void _Ht_MSL_WriteKey_Unmarked(HashTable *ht, HashObject *h);
void _Ht_MSL_DeleteKey_Marked(HashTable *ht, HashObject *h);
void _Ht_MSL_DeleteKey_Unmarked(HashTable *ht, HashObject *h);

inline void _Ht_debug_HashTableConsistent(HashTable *ht)
{
#if (RUN_CONSISTENCY_CHECKS == 1)
    
#warning "Running internal consistency checks; some processes may be slow." 

    HashTableIterator *hti = Hti_New(ht);
    
    HashObject *h;

    while( (h = Hti_Next(hti)) != NULL)
    {
	assert(O_IsType(HashObject, h));
	assert(O_REF_COUNT(h) >= 1);
	assert(H_LOCK_COUNT(h) >= 1);
    }
    
    Hti_Delete(hti);

    // Now step through to make sure things are working 
    _HT_MarkerSkipList *msl = ht->marker_sl;

    if(msl != NULL)
    {

	/* Now just walk through the marker list, adding the hash between
	 * each subsequent node. */

	HashObject *temp_h = ALLOCATEHashObject();
	HashObject *running_h = ALLOCATEHashObject();
	Hf_COPY_FROM_KEY(running_h, &msl->unmarked_object_hash);

	_HT_MSL_Node *next_leaf;
	_HT_MSL_Node *cur_leaf = (_HT_MSL_Node *)msl->first_leaf;

	markertype cur_m, next_m;

	do{
	    cur_m = cur_leaf->marker;
	    Hk_REDUCE_UPDATE(H_Hash_RW(running_h), &(cur_leaf->hk));

	    if(!H_Equal(Ht_HashAtMarkerPoint(temp_h, ht, cur_m), running_h))
	    {
		fprintf(stderr, "\n\n##############\nMarker point = %ld", cur_m);
		Ht_debug_print(ht);
		Ht_MSL_debug_Print(ht);
		abort();
	    }

	    next_leaf = cur_leaf->next;

	    if(next_leaf == NULL)
		next_m = MARKER_PLUS_INFTY;
	    else
		next_m = next_leaf->marker;

	    cur_leaf = next_leaf;

	}while(cur_leaf != NULL);

	O_DECREF(running_h);
    }
#endif
}


/**** needed routines that interface with the hash object stuff. ****/

static inline void _Ht_MSL_InitIfNeeded(HashTable *ht, HashObject *h)
{
    if(unlikely(ht->marker_sl == NULL && H_IS_MARKED(h)))
	_Ht_MSL_Init(ht);
}

static inline void _Ht_MSL_WriteKey(HashTable *ht, HashObject *h)
{
    if(H_IS_MARKED(h))
	_Ht_MSL_WriteKey_Marked(ht, h, false);
    else
	_Ht_MSL_WriteKey_Unmarked(ht, h);
}

static inline void _Ht_MSL_DeleteKey(HashTable *ht, HashObject *h)
{
    if(H_IS_MARKED(h))
	_Ht_MSL_DeleteKey_Marked(ht, h);
    else
	_Ht_MSL_DeleteKey_Unmarked(ht, h);
}

/**** Main function that does the heavy lifting in the hash key itself. ****/

#define _HT_GET            0
#define _HT_WRITE          1
#define _HT_WRITE_PASSIVE  2
#define _HT_POP            3

#define _HN_NextNode(cur_node, offset) (&(((_HashTableNodeBlock*)( (cur_node)->next))->nodes[offset]))
#define _HN_HashObject(cur_node)  ( (HashObject*)( (cur_node)->next) )

static inline HashObject* _Ht_getItem(HashTable* ht, HashObject* hk, const int mode, bool interact_with_msl)
{
    _HashTableNode *cur_hn, *hn_stack[_HT_NUMLEVELS + 1];
    _HashTableNode *last_hn = NULL;
    unsigned int register stack_top;
    /*MarkerRange range_stack[_HT_NUMLEVELS];*/

    /* All the stuff to keep track of the hashes. */
    hashfieldtype hash_buffer;
    int offset_to_current = -1, offset_to_next = -1, bits_left_in_hash_buffer, current_key_in_hash_buffer=0;
    
    assert(O_IsType(HashObject, hk));
    assert(O_IsType(HashTable, ht));

    /* Init the hash buffer stuff */
    hash_buffer = H_Hash_RO(hk)->hk[0];
    bits_left_in_hash_buffer = sizeof(hashfieldtype)*8;

    /* Get the first node in the dataset. */
    assert(ht->num_first_levels < (H_COMPONENT_SIZE / _HT_BITSIZE));

    stack_top = 0;
    hn_stack[stack_top] = cur_hn = &(ht->nodes[getBitGroup(hash_buffer, 0, ht->num_first_levels*_HT_BITSIZE)]);

    /* Advance the hash */
    bits_left_in_hash_buffer -= ht->num_first_levels*_HT_BITSIZE;
    hash_buffer >>= ht->num_first_levels*_HT_BITSIZE;
    offset_to_next = hash_buffer & _HT_BITMASK;
    hash_buffer >>= _HT_BITSIZE;
    bits_left_in_hash_buffer -= _HT_BITSIZE;


/***************************************************
 * Now for readability, define a few macros. 
 ***************************************************/

/* First, dealing with the offsets and such. */

#define ReplenishHashBuffer()						\
    do{									\
	assert(hash_buffer == 0);					\
	++current_key_in_hash_buffer;					\
	hash_buffer = H_Hash_RO(hk)->hk[current_key_in_hash_buffer];	\
									\
	bits_left_in_hash_buffer = sizeof(hashfieldtype)*8;		\
	assert(bits_left_in_hash_buffer == 32				\
	       || bits_left_in_hash_buffer == 64);			\
    }while(0)

#define __HnAlterNodeCount(_n, delta)					\
    do{									\
	_HashTableNode *n = (_n);					\
	n->count += delta;						\
    }while(0)

#define __HnStackPush(push_n, record_stack)				\
    do{									\
	__HnCountSanityCheck();						\
	_HashTableNode *__hsp_n = (push_n);				\
									\
	/* Checks */							\
	if(record_stack)						\
	{								\
	    assert(stack_top <= _HT_NUMLEVELS);				\
	    assert(hn_stack[stack_top] == cur_hn);			\
	    assert(hn_stack[stack_top] != NULL);			\
									\
	    ++stack_top;						\
	    last_hn = cur_hn;						\
	    cur_hn = hn_stack[stack_top] = __hsp_n;			\
	}								\
	else								\
	{								\
	    cur_hn = __hsp_n;						\
	}								\
									\
	/* Set the next bit offset variable. */				\
	assert(bits_left_in_hash_buffer >= _HT_BITSIZE);		\
	offset_to_current = offset_to_next;				\
	offset_to_next = hash_buffer & _HT_BITMASK;			\
	hash_buffer >>= _HT_BITSIZE;					\
	bits_left_in_hash_buffer -= _HT_BITSIZE;			\
									\
	if(unlikely(bits_left_in_hash_buffer == 0))			\
	    ReplenishHashBuffer();					\
	    								\
	__HnCountSanityCheck();						\
    }while(0)
    
#define __HnStackPushByOffset(push_n, offset, record_stack)		\
    do{									\
	_HashTableNode *hspbo_n = (push_n);				\
	__HnStackPush(_HN_NextNode(hspbo_n, offset), record_stack);	\
    }while(0)

#define __HnStackPop()							\
    do{									\
	__HnCountSanityCheck();						\
	assert(stack_top != 0);						\
	assert(stack_top <= _HT_NUMLEVELS);				\
	assert(hn_stack[stack_top] == cur_hn);				\
									\
	--stack_top;							\
	cur_hn = hn_stack[stack_top];					\
	__HnCountSanityCheck();						\
    }while(0)

#define __HnAlterPriorCounts(delta)			\
    do{							\
	__HnCountSanityCheck();				\
							\
	int i;						\
	for(i = stack_top-1; i >= 0; --i)		\
	    __HnAlterNodeCount(hn_stack[i], (delta));	\
							\
	ht->count += delta;				\
							\
	__HnCountSanityCheck();				\
    }while(0)

#define __HnInsertIntoEmptyNode(n, hashkey)				\
    do{									\
	__HnCountSanityCheck();						\
	_HashTableNode *_in = n;					\
	assert(_in->count == 0);					\
	_in->count = 1;							\
	_in->next = (void*)(hashkey);					\
	__HnCountSanityCheck();						\
    }while(0)
    

#define __HnInsertIntoEmptyNodeFromPrior(prior_hn, offset, hashkey)	\
    do{									\
	__HnCountSanityCheck();						\
	_HashTableNode *pn = (prior_hn);				\
	_HashTableNode *in = _HN_NextNode(pn, offset);			\
	assert(!_HNBO_nodeOn(pn, offset));				\
									\
	_HNBO_setNodeOn(pn, offset);					\
	assert(_HNBO_nodeOn(pn, offset));				\
									\
	__HnInsertIntoEmptyNode(in, hashkey);				\
	__HnCountSanityCheck();						\
    }while(0)

/****** Debug routines *******/

#define __HnCountSanityCheck()						\
    do{									\
	int i;								\
	for(i = 0; i <= stack_top; ++i)					\
	{								\
	    assert(hn_stack[i]->count <= ht->count+1);			\
	    if(i != 0)							\
		assert(hn_stack[i]->count <= hn_stack[i-1]->count+1);	\
	}								\
    }while(0)

#define __CheckAndReturn(ret_val, might_be_unfrozen)			\
    do{									\
	assert(might_be_unfrozen || ret_val == NULL || H_IsLocked(ret_val)); \
	__HnCountSanityCheck();						\
	_Ht_debug_HashTableConsistent(ht);				\
	return ret_val;							\
    }while(0)


/************************************************************/

    /* Check the current stack status. */ 
    assert(cur_hn == hn_stack[stack_top]);
    assert(stack_top == 0);

    _Ht_debug_HashTableConsistent(ht);

    if(mode == _HT_WRITE || mode == _HT_WRITE_PASSIVE)
    {
	_Ht_MSL_InitIfNeeded(ht, hk);

	/* Assume we should do this by default; undo if needed. */
	++ht->count;
	
	while(1)
	{
	    if(cur_hn->count == 0)
	    {
		O_INCREF(hk);
		H_CLAIM_LOCK(hk);

		if(interact_with_msl)  /* Call first, before insert. */
		    _Ht_MSL_WriteKey(ht, hk);
		
		if(last_hn != NULL)
		    _HNBO_setNodeOn(last_hn, offset_to_current);

		__HnInsertIntoEmptyNode(cur_hn, hk);
		
		__CheckAndReturn(hk, false);
	    }
	    else if (cur_hn->count == 1)
	    {
		HashObject* existing_hk = _HN_HashObject(cur_hn);

		if(unlikely(H_EQUAL(existing_hk, hk)))
		{
		    /* No change to count stuff; undo the previous
		     * count increments. */

		    __HnAlterPriorCounts(-1);

		    if(unlikely(existing_hk == hk))
			__CheckAndReturn(hk, false);

		    if(mode == _HT_WRITE)
		    {
			/* Overwrite the key. */
			if(interact_with_msl)
			    _Ht_MSL_DeleteKey(ht, existing_hk);

			O_INCREF(hk);
			H_CLAIM_LOCK(hk);

			H_RELEASE_LOCK(existing_hk);
			O_DECREF(existing_hk);

			if(interact_with_msl)
			    _Ht_MSL_WriteKey(ht, hk);

			cur_hn->next = (void*)hk;
			
			__CheckAndReturn(hk, false);
		    }
		    else /* mode == _HT_WRITE_PASSIVE */
		    {
			__CheckAndReturn(existing_hk, false);
		    }
		}
		else /* Not Equal */
		{
		    /* The current data pointer holds a data member;
		     * need to keep advancing until the two hash
		     * objects are in separate nodes.  Our hash
		     * function is strong enough that if they aren't
		     * equal, it's very unlikely they will end up being
		     * identical for long.
		     */

		    /* At this point, the tree is consistent, so write
		     * it to the MSL now; we know it will go in once
		     * we figure out where. */

		    O_INCREF(hk);
		    H_CLAIM_LOCK(hk);

		    if(interact_with_msl)
			_Ht_MSL_WriteKey(ht, hk);

		    /* Insert it. */
		    while(1)
		    {
			/* Clear the flags and get new node block. */
			cur_hn->count = 2;
			cur_hn->node_data = 0;
			cur_hn->next = _Ht_newNodeBlock(ht);
			
			assert(_HN_NextNode(cur_hn, offset_to_next)->count == 0);

			/* Node the current node. */ 
			int offset_to_next_existing = _H_extractOffset(existing_hk, stack_top + ht->num_first_levels);
			
			if(unlikely(offset_to_next_existing == offset_to_next))
			{
			    _HNBO_setNodeOn(cur_hn, offset_to_next);
			    __HnStackPushByOffset(cur_hn, offset_to_next, true);
			}
			else
			{
			  
			    __HnInsertIntoEmptyNodeFromPrior(cur_hn, offset_to_next, hk);

			    /* Reinsert the previous one. */
			    __HnInsertIntoEmptyNodeFromPrior(cur_hn, offset_to_next_existing, existing_hk);

			    __CheckAndReturn(hk, false);
			}
		    }
		}
	    }
	    else /* Count >= 2 */ 
	    {
		__HnAlterNodeCount(cur_hn, 1);
		__HnStackPushByOffset(cur_hn, offset_to_next, true);
	    }

	} /* While loop */

    } /* End if Write operations */

    else if(mode == _HT_GET)
    {
	/* Simple retreval, the easiest case. */
	while(1)
	{
	    if(unlikely(cur_hn->count == 0))
	    {
		__CheckAndReturn(NULL, false);
	    }
	    else if(cur_hn->count == 1)
	    {
		HashObject *existing_hk = _HN_HashObject(cur_hn);

		if(H_EQUAL(hk, existing_hk))
		    __CheckAndReturn(existing_hk, false);
		else
		    __CheckAndReturn(NULL, false);
	    }
	    else
	    {
		if(_HN_NextNode(cur_hn, offset_to_next) == NULL)
		{
		    __CheckAndReturn(NULL, false);
		}
		else
		{
		    __HnStackPushByOffset(cur_hn, offset_to_next, false);
		}
	    }
	}
    } /* End if read operation. */

    else if(mode == _HT_POP)
    {
	if(unlikely(ht->count == 0) )
	    return NULL;

	--ht->count;

	while(1)
	{
	    if(unlikely(cur_hn->count == 0))
	    {
		/* Undo previous stack operations */
		__HnAlterPriorCounts(1);
		__CheckAndReturn(NULL, false);
	    }
	    else if(unlikely(cur_hn->count == 1))
	    {
		HashObject *existing_hk = _HN_HashObject(cur_hn);
		assert(O_IsType(HashObject, existing_hk));

		if(likely(H_EQUAL(hk, existing_hk)))
		{
		    /* Gotta remove that key; this one's tricky. */

		    if(interact_with_msl)
			_Ht_MSL_DeleteKey(ht, existing_hk);
		    
		    cur_hn->next = NULL;
		    cur_hn->count = 0;
		
		    /* Unless this is at the base level, go back one
		     * level to see if the count is only one there.
		     * If so, then we need to move that key farther
		     * down to ensure that {count == 1}{implies that
		     * hn->next points to the hash key.
		     */ 
		    
		    if(last_hn != NULL)
		    {
			/* Delete the node holding the hash key. */
			assert(offset_to_current != -1);
			
			_HNBO_delNode(last_hn, offset_to_current);

			/* Go back */
			__HnStackPop();
			
			/* Collapse all the nodes that have only one
			 * remaining item.  (Recall that count == 1
			 * means that ->next points to the hashkey,
			 * not a _HashTableNodeBlock; so this just
			 * preserves that invariant. */

			if(unlikely(cur_hn->count == 1))
			{
			    _HashTableNode *transport_hn = _HN_NextNode(cur_hn, _HNBO_firstOffset(cur_hn));
			    assert(transport_hn->count == 1);
			    HashObject *transport_hk = (HashObject*)transport_hn->next;

			    /* So now we have this new hash key; put
			     * it in the first node that has only one
			     * node, replacing that node. */

			    while(1)
			    {
				_Ht_freeNodeBlock(ht, (_HashTableNodeBlock*)(cur_hn->next));

				/* Look ahead to see if this is an acceptable location for the count. */
				if(stack_top == 0 || hn_stack[stack_top-1]->count >= 2)
				{
				    cur_hn->next = transport_hk;
				    break;
				}
				
				__HnStackPop();
			    }
			}
		    }

		    H_RELEASE_LOCK(existing_hk);
		    __CheckAndReturn(existing_hk, true);
		} /* If equal */
		else
		{
		    __HnAlterPriorCounts(1);
		    __CheckAndReturn(NULL, false);
		}
		assert(false);
	    } 
	    else /* count >= 2 */
	    {
		__HnAlterNodeCount(cur_hn, -1);
		assert(cur_hn->next != NULL);
		__HnStackPushByOffset(cur_hn, offset_to_next, true);
	    }
	} /* end while */ 
    } /* End if pop */
    else
    {
	assert(false);
	return NULL;
    }

    /* I can C clearly now, the brain is gone... */
}

/*****************************************
 * The functions to wrap the above.
 ****************************************/

static inline void _Ht_RunKeyAsserts(HashObject *hk)
{
    assert(hk != NULL);
    assert(O_IsType(HashObject, hk));
    assert(O_RefCount(hk) >= 1);
}

HashObject* Ht_Get(HashTable *ht, HashObject *hk)
{
    /* Returns the given key if it's present in the hash table; if it
     * is not preset, NULL is returned. 
     */

    _Ht_RunKeyAsserts(hk);
    HashObject *h = _Ht_getItem(ht, hk, _HT_GET, true);

    if(h != NULL)
    {
	assert(O_RefCount(h) >= 1);
	assert(H_LockCount(h) >= 1);
    }

    _Ht_debug_HashTableConsistent(ht);

    return h;
}

HashObject* Ht_Pop(HashTable *ht, HashObject *hk)
{
    /* Finds the given key in the hash table, deletes it from the tree
     * and returns it.  The caller would then own a reference to the
     * item.
     */

    _Ht_RunKeyAsserts(hk);
    HashObject *h = _Ht_getItem(ht, hk, _HT_POP, true);

    if(h != NULL)
    {
	assert(O_RefCount(h) >= 1);
	assert(O_RefCount(h) >= H_LockCount(h) + 1);
    }

    _Ht_debug_HashTableConsistent(ht);

    return h;
}

bool Ht_Clear(HashTable *ht, HashObject *hk)
{
    /* Deletes the given key from the hash table.  Returns true if the
     * key was present and false otherwise. 
     */
    
    _Ht_RunKeyAsserts(hk);

    hk = _Ht_getItem(ht, hk, _HT_POP, true);

    _Ht_debug_HashTableConsistent(ht);

    if(hk != NULL)
    {
	O_DECREF(hk);
	return true;
    }
    else
    {
	return false;
    }
}

void Ht_Set(HashTable *ht, HashObject *hk)
{
    /* Adds the given key to the hash table.  Overwrites the key if it
     * is present already.  Increments the reference count.
     */

    _Ht_RunKeyAsserts(hk);

    _Ht_getItem(ht, hk, _HT_WRITE, true);

    assert(O_RefCount(hk) >= 1);
    assert(H_LockCount(hk) >= 1);

    _Ht_debug_HashTableConsistent(ht);

}

void Ht_Give(HashTable *ht, HashObject *hk)
{
    /* Adds the given key to the hash table.  Overwrites the key if it
     * is present already.
     *
     * Need to protect against regiving, i.e. doing a get and then a
     * give.  Thus the assumption is that the reference count does not
     * change using this function.
     */

    size_t prev_ref_count = O_REF_COUNT(hk);

    _Ht_RunKeyAsserts(hk);
    _Ht_getItem(ht, hk, _HT_WRITE, true);

    assert(H_LOCK_COUNT(hk) >= 1);

    if(O_REF_COUNT(hk) > prev_ref_count)
	O_DECREF(hk);
    
    assert(O_REF_COUNT(hk) == prev_ref_count);

    _Ht_debug_HashTableConsistent(ht);

}

HashObject* Ht_SetDefault(HashTable *ht, HashObject *hk)
{
    _Ht_RunKeyAsserts(hk);
    HashObject *h = _Ht_getItem(ht, hk, _HT_WRITE_PASSIVE, true);
    
    assert(O_RefCount(h) >= 1);
    assert(H_LockCount(h) >= 1);

    _Ht_debug_HashTableConsistent(ht);

    return h;
}

size_t Ht_Size(HashTable *ht)
{ 
    return Ht_SIZE(ht);
}

bool Ht_Contains(HashTable *ht, HashObject *hk)
{
    _Ht_debug_HashTableConsistent(ht);

    return (_Ht_getItem(ht, hk, _HT_GET, true) == NULL) ? false : true;
}

void Ht_AddMarkerRangeToExistingKey(HashTable *ht, HashObject *hk, markertype r_start, markertype r_end)
{
    HashObject *k = Ht_Pop(ht, hk);
    H_AddMarkerValidRange(k, r_start, r_end);
    Ht_Give(ht, k);
}

/********************************************************************************
 *
 *  Some debugging related routines.
 *
 ********************************************************************************/

void _Ht_print_node(_HashTableNode *hn, size_t indent, size_t cap, size_t node_idx)
{
    int i, t;
    static char hkstring[33];

    if(cap == 0) return;

    if(hn->count == 1)
    {
	for(t=0;t < indent; ++t)
	    printf(" ");

	H_ExtractHash(hkstring, _HN_HashObject(hn));
	hkstring[32] = '\0';

	printf("%lx: ptr=0x%lx, count=%lu\n", 
	       (long unsigned int)node_idx, (long unsigned int)hn, (long unsigned int)hn->count);

	for(t=0;t < indent; ++t)
	    printf(" ");

	printf("---> %s -- ", hkstring);
	Mi_debug_printMi(_HN_HashObject(hn)->mi);
    }
    else
    {

	for(t=0;t < indent; ++t)
	    printf(" ");
	    
	printf("%lx: ptr=0x%lx, count=%ld, local_count=%ld, first=%x, mask=", 
	       (long unsigned int)node_idx, (long unsigned int)hn, 
	       (long)hn->count, (long)_HNBO_localCount(hn), (int)_HNBO_firstOffset(hn));

	int j;

	for(j = 0; j < 16; ++j)
	{
	    if(_HNBO_nodeOn(hn, j))
		printf("%x,", j);
	}
	
	printf("\n");

	for(i = 0; i < _HT_LEVEL_SIZE; ++i)
	{
	    if(hn->next == NULL)
	    {
		printf("COUNT ERROR\n");
		continue;
	    }

	    _HashTableNode* nhn = _HN_NextNode(hn, i);
	
	    if(nhn == NULL)
	    {
		printf("ERROR\n");
		continue;
	    }
    
	    if(nhn->count == 0) continue;
	    	    
	    _Ht_print_node(nhn, indent + 2, cap -1, i);
	}
    }
}

void Ht_debug_print(HashTable *ht)
{
    size_t i;
    
    printf("Printing Hash Table with %lu nodes.\n\n", (long unsigned int)ht->count);

    for(i = 0; i < _HT_FIRST_LEVEL_SIZE(ht); ++i)
    {
	_HashTableNode* nhn = &(ht->nodes[i]);
	    
	if(nhn->count == 0) continue;

	_Ht_print_node(nhn, 0, 100, i);
    }
}

/********************************************************************************
 *
 *  Internal hash keys functions
 *
 ********************************************************************************/

static inline void _Ht_FillHKFromCurrentTree(HashTable *ht, HashKey *hk)
{
    HashTableIterator *hti = Hti_New(ht);
    HashObject *h;

    Hk_CLEAR(hk);

    while( (h = Hti_Next(hti)) != NULL)
	Hk_REDUCE_UPDATE(hk, H_Hash_RO(h));

    Hti_Delete(hti);
}
    

/********************************************************************************
 *
 *  Functions for handling the skip list to handle the marker stuff.
 *
 ********************************************************************************/

/* Invariants in the skip list:
 * 
 * The hash recorded at each node is the hash of all the leaves just
 * below it to (not including) the leaf below the hash at the next
 * node. 
 */

/* Here's a few macros for debugging purposes. */
#ifdef DEBUG_MODE

#define SetNodeLevel(_node, _level) (_node)->level = (_level)

#else

#define SetNodeLevel(_node, _level)

#endif


/****************************************
 *
 * Node stack operations for easily moving on the skip list
 *
 ****************************************/

static inline _HT_MSL_NodeStack* _Ht_MSL_BS_New(_HT_MSL_Node *node)
{
    _HT_MSL_NodeStack *ns = 
	(_HT_MSL_NodeStack *) MP_ItemMalloc(&ht_msl_node_stack_pool);

    ns->node = node;
    return ns;
}

#define _Ht_MSL_BS_Delete(msl_bs_ptr)				\
    do{								\
	_HT_MSL_NodeStack *nbs, *bs = (msl_bs_ptr);		\
	while(bs != NULL)					\
	{							\
	    nbs = bs->previous;					\
	    MP_ItemFree(&ht_msl_node_stack_pool, bs);		\
	    bs = nbs;						\
	}							\
    }while(0)


#define _Ht_MSL_BS_Pop(msl_bs_ptr_ptr)					\
    do{									\
	_HT_MSL_NodeStack *prev_bs = (*(msl_bs_ptr_ptr))->previous;	\
	MP_ItemFree(&ht_msl_node_stack_pool, (*(msl_bs_ptr_ptr)));	\
	*msl_bs_ptr_ptr = prev_bs;					\
    }while(0)								

static inline void _Ht_MSL_BS_Push(_HT_MSL_NodeStack **ns_ptr, _HT_MSL_Node* node)
{
    _HT_MSL_NodeStack *next_bs = (_HT_MSL_NodeStack*)MP_ItemMalloc(&ht_msl_node_stack_pool);
    next_bs->node = node;
    next_bs->previous = *ns_ptr;
    *ns_ptr = next_bs;
}

#define _Ht_MSL_BS_SetTravelNodeFlag(msl_bs_ptr, _is_travel_node)	\
    do{									\
	(msl_bs_ptr)->is_travel_node = (_is_travel_node);		\
    }while(0)

#define _Ht_MSL_BS_IsTravelNode(msl_bs_ptr)	\
    (!!((msl_bs_ptr)->is_travel_node))

#define _Ht_MSL_BS_IsBottomNode(msl_bs_ptr)	\
    (!!((msl_bs_ptr->previous == NULL)))

static inline _HT_MSL_Node* _Ht_MSL_BS_CurNode(_HT_MSL_NodeStack *msl_bs_ptr)
{
    assert(msl_bs_ptr->node != NULL);
    return msl_bs_ptr->node;
}

#define _Ht_MSL_BS_CurBranch(msl_bs_ptr)		\
    (_HT_MSL_AsBranch(_Ht_MSL_BS_CurNode(msl_bs_ptr)))

#define _Ht_MSL_BS_CurLeaf(msl_bs_ptr)			\
    (_HT_MSL_AsLeaf(_Ht_MSL_BS_CurNode(msl_bs_ptr)))

#define _Ht_MSL_BS_NextNode(msl_bs_ptr)		\
    (_Ht_MSL_BS_CurNode(msl_bs_ptr)->next)

#define _Ht_MSL_BS_NextLeaf(msl_bs_ptr)				\
    (_HT_MSL_AsLeaf(_Ht_MSL_BS_CurNode(msl_bs_ptr)->next))

#define _Ht_MSL_BS_NextBranch(msl_bs_ptr)			\
    (_HT_MSL_AsBranch(_Ht_MSL_BS_CurNode(msl_bs_ptr)->next))

static inline _HT_MSL_Node* _Ht_MSL_BS_DownNode(_HT_MSL_NodeStack *msl_bs_ptr)
{
    assert(_Ht_MSL_BS_CurBranch(msl_bs_ptr)->down != NULL);
    return _Ht_MSL_BS_CurBranch(msl_bs_ptr)->down;
}

static inline void _HT_MSL_BS_Prepend(
    _HT_MSL_NodeStack **ns_ptr, _HT_MSL_Node *node, bool is_travel)
{
    assert((*ns_ptr)->previous == NULL);
    (*ns_ptr)->previous = _Ht_MSL_BS_New(node);
    (*ns_ptr) = (*ns_ptr)->previous;
    (*ns_ptr)->is_travel_node = is_travel;
}

static inline void _Ht_MSL_BS_MoveForward(_HT_MSL_NodeStack **msl_bs_ptr_ptr)
{
    _HT_MSL_Node *next_node = _Ht_MSL_BS_NextNode(*(msl_bs_ptr_ptr)); 
    _Ht_MSL_BS_SetTravelNodeFlag(*(msl_bs_ptr_ptr), true);		
    _Ht_MSL_BS_Push(msl_bs_ptr_ptr, next_node);				
}

static inline void _Ht_MSL_BS_MoveDown(_HT_MSL_NodeStack **msl_bs_ptr_ptr)
{
    _HT_MSL_Node *down_node = _Ht_MSL_BS_DownNode(*(msl_bs_ptr_ptr));	
    _Ht_MSL_BS_SetTravelNodeFlag(*(msl_bs_ptr_ptr), false);		
    _Ht_MSL_BS_Push((msl_bs_ptr_ptr), down_node);			
}

void _Ht_MSL_Init(HashTable *ht)
{
    _HT_MarkerSkipList *msl = ht->marker_sl;
    assert(msl == NULL);

    msl = ht->marker_sl = (_HT_MarkerSkipList*)MP_ItemMalloc(&ht_msl_pool);
    
    /* Now set the unmarked_object_hash with the current hash tree's
     * hashes, since they are all marker-invariant at this point. */ 
    _Ht_FillHKFromCurrentTree(ht, &msl->unmarked_object_hash);

    /* Now set an initial stack with a single empty node at minus
     * infinity.  This avoids a bookkeeping headache with a changing
     * first node.*/
    
    msl->first_leaf = _Ht_newMarkerLeaf(ht);
    msl->first_leaf->marker = MARKER_MINUS_INFTY;
    /* Rest of leaf zeroed, that's what we want. */
    
    msl->start_node = _Ht_newMarkerBranch(ht);
    msl->start_node->marker = MARKER_MINUS_INFTY;
    msl->start_node->down = (_HT_MSL_Node*)msl->first_leaf;
    /* Rest of node zeroed, that's what we want. */

    msl->start_node_level = 1;
    SetNodeLevel(msl->start_node, msl->start_node_level);
}

static inline unsigned int __Ht_MSL_NewEntryHeight(_HT_MarkerSkipList *msl)
{
    /* Returns the height of a new node in the skip list.  There is a
     * 1/4 probability that each node is sent to the next level up. */
    
    unsigned int height = 0;
    register unsigned long r = msl->cur_rand_factor;

    while(1)
    {
	if(unlikely(r == 0)) 
	    r = genrand_uint32();
	
	if( unlikely( (r & 0x00000003) ) )
	{
	    ++height;
	    r >>= 2;
	    assert( (r & 0xc0000000) == 0);
	    continue;
	}
	else
	{
	    r >>= 2;
	    assert( (r & 0xc0000000) == 0);
	    break;
	}
    }

    msl->cur_rand_factor = r;
    return (height < _HT_MSL_MAX_LEVELS) ? height : _HT_MSL_MAX_LEVELS;
}

static inline bool __Ht_MSL_AdvanceNodeStack(
    _HT_MSL_NodeStack **ns_ptr, unsigned int *cur_level_ptr, 
    markertype query)
{
    /* Attempts to advance the node stack.  Returns true on success.
     *
     * If non_travel_node_update_hk != NULL, then every node before
     * traveling down, excluding the top current node and excluding the
     * leaf, is updated with the given hash.
     */

    _HT_MSL_Node *cur_node = _Ht_MSL_BS_CurNode(*ns_ptr);
    _HT_MSL_Node *next_node = cur_node->next;
    
    if(next_node == NULL || next_node->marker > query)
    {
	if(likely(*cur_level_ptr > 0))
	{
	    assert((*ns_ptr)->node->level == *cur_level_ptr);
	    _Ht_MSL_BS_MoveDown(ns_ptr);
	    --(*cur_level_ptr);
	    assert((*ns_ptr)->node->level == *cur_level_ptr);
	    return true;
	}
	else
	{
	    return false;
	}
    }
    else
    {
	_Ht_MSL_BS_MoveForward(ns_ptr);
	assert((*ns_ptr)->node->level == *cur_level_ptr);
	return true;
    }
}

static inline void __Ht_MSL_BackupNodeStack(
    _HT_MSL_NodeStack **ns_ptr, unsigned int *cur_level_ptr, 
    const HashKey *non_travel_node_update_hk)
{
    assert( (*ns_ptr)->previous != NULL);

    assert((*ns_ptr)->node->level == *cur_level_ptr);

    _Ht_MSL_BS_Pop(ns_ptr);

    assert(*ns_ptr != NULL);

    if(!(*ns_ptr)->is_travel_node )
	++(*cur_level_ptr);

    assert((*ns_ptr)->node->level == *cur_level_ptr);

    /* Update the hash if needed. */
    if(unlikely(non_travel_node_update_hk != NULL 
		&& !(*ns_ptr)->is_travel_node))
    {
	Hk_REDUCE_UPDATE(&(_Ht_MSL_BS_CurNode(*ns_ptr)->hk), non_travel_node_update_hk);
	/* DEBUG-LOC */
    }
}


void __Ht_MSL_InsertValue(HashTable *ht, _HT_MSL_NodeStack **ns_ptr, 
			  unsigned int *cur_level_ptr,
			  const HashKey *insert_hk, 
			  const HashKey *remove_hk, 
			  const markertype loc, 
			  const unsigned int new_height)
{
    /* Function does what it says; the only unexpected thing is that
     * the resulting node stack always points to the element
     * immediately before the top of the stack above the leaf at loc,
     * at the level of the top of the stack.  Thus this function does
     * not completely insert the hash of the new tree -- other hashes
     * may be inserted in pairs in such a way that this one is
     * canceled out.
     */

    _HT_MSL_Leaf *left_leaf = _Ht_MSL_BS_CurLeaf(*ns_ptr);
    _HT_MSL_Leaf *right_leaf = _HT_MSL_AsLeaf(left_leaf->next);

    assert(*ns_ptr != NULL);
    assert(ht->marker_sl != NULL);
    assert(left_leaf->marker <= loc);
    assert(right_leaf == NULL || right_leaf->marker > loc);

    if(unlikely(left_leaf->marker == loc))
    {
	/* Add this one into the current leaf's value. */

	/* Back the node stack up the stack on this leaf. Update all,
	 * including this leaf. */

	assert(_Ht_MSL_BS_CurNode(*ns_ptr) == (void*)left_leaf);
	
	Hk_REDUCE_UPDATE(&(_Ht_MSL_BS_CurNode(*ns_ptr)->hk), insert_hk);
	
	while(1)
	{
	    __Ht_MSL_BackupNodeStack(ns_ptr, cur_level_ptr, NULL);
	    
	    if(likely(_Ht_MSL_BS_CurNode(*ns_ptr)->marker != loc))
		return;	
	    else
		Hk_REDUCE_UPDATE(&(_Ht_MSL_BS_CurNode(*ns_ptr)->hk), insert_hk);
	 
	    if((*ns_ptr)->previous == NULL)  /* Can occur at -inf node. */
		return;
	}
    }
    else
    {
	_HT_MarkerSkipList *msl = ht->marker_sl;

	/* Create the new leaf. */
	_HT_MSL_Leaf *new_leaf = _Ht_newMarkerLeaf(ht);
    
	new_leaf->marker = loc;
	/* The hash will be updated later. */

	/* Insert it in the leaf sequence. */
	left_leaf->next = (_HT_MSL_Node*)new_leaf;
	new_leaf->next  = (_HT_MSL_Node*)right_leaf;

	if(likely(new_height == 0))
	{
	    assert((*ns_ptr)->node->marker < loc);
	    Hk_COPY(&(new_leaf->hk), insert_hk);

	    return;  /* No changes needed, happens 75% of the time. */
	}

	/* First, add it to the original node stack. */
	assert(*cur_level_ptr == 0);
	assert(_Ht_MSL_BS_CurNode(*ns_ptr) == (void*)left_leaf);
	_Ht_MSL_BS_MoveForward(ns_ptr);
	assert(_Ht_MSL_BS_CurNode(*ns_ptr) == (void*)new_leaf);

	/* See if we need to raise the overall level of the skip list. */
	if(unlikely(msl->start_node_level < new_height))
	{
	    /* Need to raise the starting level.  This might be a bit
	     * of tricky bookkeeping regarding the hashes; however,
	     * they are always added in pairs so the net effect is
	     * that the hash of all new nodes is zero under the XOR
	     * operations. YAY!!!! */ 

	    /* Add new nodes onto the current start node so it's
	     * height is equal. */

	    _HT_MSL_Branch *cur_top_node = msl->start_node;

	    assert(cur_top_node->level == msl->start_node_level);

	    /* Find the start of the node stack so we can prepend to
	     * it, as we'll need that info to punch up the stack above
	     * the new leaf. */
	    _HT_MSL_NodeStack *ns_start = (*ns_ptr);
	    
	    while(ns_start->previous != NULL)
		ns_start = ns_start->previous;

	    assert(ns_start->node == (void*)cur_top_node);

	    /* Figure out the hash of all of these nodes. */
	    HashKey hk;
	    _HT_MSL_Branch *n = cur_top_node;

	    Hk_COPY(&hk, &(n->hk) );

	    n = (_HT_MSL_Branch *)(n->next);

	    while(n != NULL)
	    {
		Hk_REDUCE_UPDATE(&hk, &n->hk);
		n = (_HT_MSL_Branch *)(n->next);
	    }
		
	    /* Now build up the nodes at the start. */
	    while(1)
	    {
		_HT_MSL_Branch *first_update_node = _Ht_newMarkerBranch(ht);
		
		first_update_node->marker = cur_top_node->marker;
		first_update_node->down = (_HT_MSL_Node*)cur_top_node;
		Hk_COPY(&first_update_node->hk, &hk);
		SetNodeLevel(first_update_node, msl->start_node_level + 1);
		
		_HT_MSL_BS_Prepend(&ns_start, (_HT_MSL_Node*)first_update_node, false);

		++msl->start_node_level;
		    
		cur_top_node = first_update_node;

		if(likely(msl->start_node_level == new_height))
		    break;
	    }

	    msl->start_node = cur_top_node;
	} /* End raise first level. */

	/* Now need to push the level up; this is done in part by
	 * unwinding the stack.  We know at this point that
	 * new_height>=1, so at least one step is needed. */

	_HT_MSL_Node *cur_stack_node = (_HT_MSL_Node*)new_leaf;
	assert(*cur_level_ptr == 0);

	while(1)
	{
	    /* Find the first node at the previous level; since it
	     * ends at a leaf, there will always be one. */

	    do{
		__Ht_MSL_BackupNodeStack(ns_ptr, cur_level_ptr, NULL);
	    }while( (*ns_ptr)->is_travel_node);

	    assert(cur_stack_node->level == *cur_level_ptr - 1);
	    
	    _HT_MSL_Branch *upper_left_node = _Ht_MSL_BS_CurBranch( (*ns_ptr) );

	    assert(upper_left_node->level == *cur_level_ptr);
	    
	    _HT_MSL_Branch *upper_right_node = _HT_MSL_AsBranch(upper_left_node->next);

	    _HT_MSL_Branch *upper_stack_node = _Ht_newMarkerBranch(ht);
	    upper_stack_node->marker = loc;
	    upper_stack_node->down = cur_stack_node;

#ifdef DEBUG_MODE
	    upper_stack_node->level = *cur_level_ptr; 
#endif
	    /* We want the hash key clear, which is the case at this
	     * point. */
	    
	    upper_left_node->next  = (_HT_MSL_Node*)upper_stack_node;
	    upper_stack_node->next = (_HT_MSL_Node*)upper_right_node;

	    /* Get the hash for the current node, and also update the
	       upper_left_node with the combined hashes of all the
	       keys that will no longer be in it's domain.  Thus need
	       to get the address of the node below the upper right
	       one in order to know when to stop collecting hash
	       changes for the upper left node. */
	    
	    _HT_MSL_Node *lower_right_stop_node = 
		(likely(upper_right_node != NULL)) ? upper_right_node->down : NULL;
	    
	    /* A few checks to make sure we've got what we want. */
	    assert(upper_right_node == NULL 
		   || upper_right_node->level == *cur_level_ptr);

	    assert(lower_right_stop_node == NULL
		   || lower_right_stop_node->level == *cur_level_ptr - 1);

	    _HT_MSL_Node *n = cur_stack_node->next;

	    /* Update the current hash with the ones to the lower and
	     * lower right of it, on the same level. */

	    Hk_COPY(&upper_stack_node->hk, &cur_stack_node->hk);

	    while(n != lower_right_stop_node)
	    {
		Hk_REDUCE_UPDATE(&upper_stack_node->hk, &n->hk);
		assert(lower_right_stop_node == NULL || n->next != NULL);
		n = n->next;
	    }

	    /* Subtract this hash from the upper left node, as it
	     * comprises all the leaves no longer under its domain.
	     * Need to cancel out the current along with it, though,
	     * as it shouldn't be a factor. */

	    HashKey removal_hash;
	    Hk_NEGATIVE(&removal_hash, &upper_stack_node->hk);

	    Hk_REDUCE_UPDATE(&upper_left_node->hk, &removal_hash);
	    Hk_REDUCE_UPDATE(&cur_stack_node->hk, insert_hk);

	    /* Are we done? (inserted upper_stack_node at cur_level + 1) */
	    if(likely(*cur_level_ptr == new_height))
	    {
		/* Now it goes over to the top of this stack. */
		Hk_REDUCE_UPDATE(&(upper_stack_node->hk), insert_hk);
		assert((*ns_ptr)->node->marker < loc);
		(*ns_ptr)->is_travel_node = true;

		break;
	    }
	    else
	    {
		/* Prepare for the next iteration. */
		cur_stack_node = (_HT_MSL_Node*)upper_stack_node;
	    }
	}
    }
}

/* The main interface functions. */
void _Ht_MSL_WriteKey_Marked(HashTable *ht, HashObject *h, bool switch_add_sub_flags)
{
    /* Plan: 
     *
     * Add in this hash object at all the marker ranges of the hash
     * object, in reverse.  This allows us to effectively use the
     * marker values in the node stack to know where to stop unwinding
     * and dive down for the next entry.
     *
     * Thus for each marker, we:
     *
     * Run down the list first trying to add the key in at the marker
     * range end.
     *
     * Then unwind the node stack until we find the departure point
     * for the first node, changing the hashkeys in the non-travel
     * nodes on the way back up (travel nodes are where you next
     * travel across instead of moving down).
     *
     * Run down the list from this node to insert the first key,
     * changing the hash of all non-travel nodes.
     *
     * Note that the hashes of the nodes above the point where these
     * depart don't need to be changed, as x ^ x = 0.
     *
     * Unwind the list until the next hash doesn't need to be changed.
     *
     */

    /* should be checked at previous level */
    assert(H_IS_MARKED(h));
    assert(ht->marker_sl != NULL);
    assert(ht->marker_sl->first_leaf != NULL);

    _HT_MarkerSkipList *msl = ht->marker_sl;

    /* Plan: 
     *
     * Run down the list first trying to add the key in at the seconde
     * location sub_loc.
     *
     * Then unwind the node stack until we find the departure point
     * for the first node, changing the hashkeys in the non-travel
     * nodes on the way back up (travel nodes are where you next
     * travel across instead of moving down).
     *
     * Run down the list from this node to insert the first key,
     * changing the hash of all non-travel nodes.
     *
     * Note that the hashes of the nodes above the point where these
     * depart don't need to be changed, as x ^ x = 0.
     */
    
    /* Create the start of the node stack at the beginning. */
    _HT_MSL_NodeStack *ns = _Ht_MSL_BS_New((_HT_MSL_Node*)msl->start_node);
    unsigned int cur_level = msl->start_node_level;
    
    /* Set up the iterations. */
    MarkerRange *current_mr, *next_mr;

    MarkerRevIterator *miri = NewReversedMarkerIterator(h->mi);

    HashKey addition_hk, removal_hk;
    
    if(unlikely(Hk_ISZERO(H_Hash_RO(h))))
       return;

    if(switch_add_sub_flags)
    {
	Hk_COPY(&removal_hk, H_Hash_RO(h));
	Hk_NEGATIVE(&addition_hk, H_Hash_RO(h));
    }
    else
    {
	Hk_COPY(&addition_hk, H_Hash_RO(h));
	Hk_NEGATIVE(&removal_hk, H_Hash_RO(h));
    }

    next_mr = Miri_Next(miri);
    assert(H_IS_MARKED(h));
    assert(next_mr != NULL);   /* This case should be caught by H_IsMarked. */

    HashKey temp;
    Hk_Reduce(&temp, &addition_hk, &removal_hk);
    assert(Hk_IsZero(&temp));

    while(1)
    {
	/* Increment the iteration. */
	current_mr = next_mr;
	next_mr = Miri_Next(miri);
	
	markertype add_loc = current_mr->start;
	markertype sub_loc = current_mr->end;

	/*
	printf("\nInserting ");
	Hk_debug_PrintHash(&addition_hk);
	printf(" at marker %ld ", add_loc);

	printf("and ");
	Hk_debug_PrintHash(&removal_hk);
	printf(" at marker %ld\n", sub_loc);
	*/

	assert(add_loc < sub_loc);

	unsigned int new_h1 = __Ht_MSL_NewEntryHeight(msl);
	unsigned int new_h2 = __Ht_MSL_NewEntryHeight(msl);
	
	/* Decide on a threshhold level and marker location
	 * before/above which adding the same hash key to two places
	 * in the tree won't affect the node hashes, as they cancel
	 * each other out.  */

	unsigned int threshhold_level = new_h1 > new_h2 ? new_h1 : new_h2;
	markertype threshhold_marker = add_loc;

	unsigned int alt_threshhold_level = 0;

	/* Advance to the leaf. */
	do{
	    /* Catch the upper left point in the stack where the hash
	     * might possibly change. */
	    if(ns->node->marker <= add_loc)
	    {
		alt_threshhold_level = cur_level;
		threshhold_marker = ns->node->marker;
	    }

	}while(__Ht_MSL_AdvanceNodeStack(&ns, &cur_level, sub_loc));

	if(alt_threshhold_level > threshhold_level)
	    threshhold_level = alt_threshhold_level;
	
	__Ht_MSL_InsertValue(ht, &ns, &cur_level, &removal_hk, &addition_hk, sub_loc, new_h1);

	// printf("########################################\n");
	// printf("After inserting r_hk at sub_loc\n");

       	// Ht_MSL_debug_Print(ht);

	/* Now unwind the stack to the first node with a marker value
	 * less than add_loc, rerun the stack from there.  We need to
	 * add in the hash of the key to each vertical traverse in
	 * this process. */
	
	assert(ns->node->marker <= sub_loc);

	while( (_Ht_MSL_BS_CurNode(ns)->marker >= threshhold_marker
		|| cur_level <= threshhold_level)
	       && ns->previous != NULL)
	{
	    __Ht_MSL_BackupNodeStack(&ns, &cur_level, &removal_hk);
	}

	// printf("########################################\n");
	// printf("After backing up\n");

       	// Ht_MSL_debug_Print(ht);

	assert(ns->node->level == cur_level);
	assert(ns->node->marker <= add_loc);

	/* Advance down to the base; we can't update the hash at this
	 * point, since InsertValue may modify it. */
	while(__Ht_MSL_AdvanceNodeStack(&ns, &cur_level, add_loc));

	/* Insert this value. */
	__Ht_MSL_InsertValue(ht, &ns, &cur_level, &addition_hk, &removal_hk, add_loc, new_h2);

	// printf("########################################\n");
	// printf("After inserting add_hk at add_loc\n");

       	// Ht_MSL_debug_Print(ht);


	/* Go back updating the nodes to the same point as before. */
	while( (_Ht_MSL_BS_CurNode(ns)->marker >= threshhold_marker
		|| cur_level <= threshhold_level)
	       && ns->previous != NULL)
	{
	    __Ht_MSL_BackupNodeStack(&ns, &cur_level, &addition_hk);
	}

	// printf("########################################\n");
	// printf("After backing up second time\n");
 
	// Ht_MSL_debug_Print(ht);

	if(next_mr == NULL)
	    break;

	assert(next_mr->end < add_loc);

	/* Another key to do, so go back up the stack until we're at
	 * the next pivot point. */

	while(next_mr->start < _Ht_MSL_BS_CurNode(ns)->marker)
	    __Ht_MSL_BackupNodeStack(&ns, &cur_level, NULL);

    }

    /* We're done, just delete the node stack and iterator. */
    _Ht_MSL_BS_Delete(ns);
    Miri_Delete(miri);
}

void _Ht_MSL_DeleteKey_Marked(HashTable *ht, HashObject *h)
{
    _Ht_MSL_WriteKey_Marked(ht, h, true);
}

/************************************************************
 *
 *  Interacting with the unmarked tree hash.
 *
 ************************************************************/

void _Ht_MSL_WriteKey_Unmarked(HashTable *ht, HashObject *h)
{
    if(ht->marker_sl != NULL)
    {
	Hk_REDUCE_UPDATE(&ht->marker_sl->unmarked_object_hash, H_Hash_RO(h));
    }
}

void _Ht_MSL_DeleteKey_Unmarked(HashTable *ht, HashObject *h)
{
    HashKey hk;
    Hk_NEGATIVE(&hk, H_Hash_RO(h));

    /* Same operation as writing. */
    if(ht->marker_sl != NULL)
    {
	Hk_REDUCE_UPDATE(&ht->marker_sl->unmarked_object_hash, &hk);
    }
}

/************************************************************
 *
 *  Retrieval functions
 *
 *************************************************************/

/* Retrieve the hash at a certain marker point. */
void _Ht_MSL_HashAtMarkerPoint(HashKey *hk_dest, HashTable *ht, markertype loc)
{

    if(unlikely(loc == MARKER_PLUS_INFTY))
    {
	Hk_CLEAR(hk_dest);
	return;
    }

    _HT_MarkerSkipList *msl = ht->marker_sl;

    if(unlikely(msl == NULL))
    {
	_Ht_MSL_Init(ht);
	Hk_COPY(hk_dest, &(ht->marker_sl->unmarked_object_hash));
	return;
    }

    Hk_COPY(hk_dest, &(ht->marker_sl->unmarked_object_hash));

    /* Just travel down the tree to the node before this one. */
    _HT_MSL_NodeStack *ns = _Ht_MSL_BS_New((_HT_MSL_Node*)msl->start_node);
    unsigned int cur_level = msl->start_node_level;

    while(__Ht_MSL_AdvanceNodeStack(&ns, &cur_level, loc));
    
    assert(ns->node->marker <= loc);

    /* Include the tip leaf, and then all travel nodes past that. */

    Hk_REDUCE_UPDATE(hk_dest, &(_Ht_MSL_BS_CurNode(ns)->hk));
    _Ht_MSL_BS_Pop(&ns);

    while(ns != NULL)
    {
	if(ns->is_travel_node)
	    Hk_REDUCE_UPDATE(hk_dest, &(_Ht_MSL_BS_CurNode(ns)->hk));	    

	_Ht_MSL_BS_Pop(&ns);
    }
}

/****************************************
 *
 *  HashTable functions to go with the above. 
 *
 ****************************************/

HashObject* Ht_HashAtMarkerPoint(HashObject *h_dest, HashTable *ht, markertype m)
{
    if(h_dest == NULL)
	h_dest = ConstructHashObject();

    _Ht_MSL_HashAtMarkerPoint(H_Hash_RW(h_dest), ht, m);

    return h_dest;
}

bool Ht_ContainsAt(HashTable *ht, HashObject *_h, markertype m)
{
    HashObject *h = Ht_Get(ht, _h);

    if(h == NULL) 
	return false;
    else
	return H_MarkerPointIsValid(h, m);
}


/****************************************
 *
 *  Debug routines 
 *
 ****************************************/

void Ht_MSL_debug_Print(HashTable *ht)
{
    printf("\n");
    _HT_MarkerSkipList *msl = ht->marker_sl;
    
    if(msl == NULL)
	return;

    int h = msl->start_node_level;
    
    _HT_MSL_Branch *br = msl->start_node;
    _HT_MSL_Branch *next_br;

    char s[33];

    while(h > 0)
    {
	next_br = (_HT_MSL_Branch*)br->down;
	_HT_MSL_Node *l = (_HT_MSL_Node*)msl->first_leaf;
	
	printf("%d: \t", h);

	while(br != NULL)
	{
	    while(l->marker != br->marker)
	    {
		printf(" %ld %s |", l->marker, "   ");
		l = l->next;
	    }

	    Hk_ExtractHash(s, &(br->hk));
	    s[3] = '\0';
	    printf(" %ld %s |", br->marker, s);
	    br = (_HT_MSL_Branch*)br->next;
	    l = l->next;
	}
	
	printf("\n");
	--h;
	br = next_br;
    }
    
    printf("0: \t");

    _HT_MSL_Node *l = (_HT_MSL_Node*)msl->first_leaf;
    
    while(l != NULL)
    {
	Hk_ExtractHash(s, &(l->hk));
	s[3] = '\0';
	printf(" %ld %s |", l->marker, s);
	l = l->next;
    }

    printf("\n");
    fflush(stdout);
}

void Ht_MSL_debug_PrintNodeStack(_HT_MSL_NodeStack *ns)
{
    printf("NodeStack %x:", (size_t)ns);

    while(ns != NULL)
    {
	if(ns->node == NULL)
	    printf("(NULL:%c) ", ns->is_travel_node ? 'T' : 'D');
	else
	{
#ifdef DEBUG_MODE	    
	    printf("(%d  %ld  %c) ", 
		   ns->node->level, 
		   ns->node->marker, 
		   ns->is_travel_node ? 'T' : 'D');
#else
	    printf("(--  %ld  %c) ", 
		   ns->node->marker, 
		   ns->is_travel_node ? 'T' : 'D');
#endif
	}

	ns = ns->previous;
    }

    printf("\n");
    fflush(stdout);
}



/********************************************************************************
 * 
 *  Iteration functions to go through all nodes in the tree.
 *
 *********************************************************************************/

/* Creates a new iterator.  Calling next on a new iterator returns the
 * first element. */
HashTableIterator* Hti_New(HashTable *ht)
{
    HashTableIterator *hti = malloc(sizeof(HashTableIterator));
    memset(hti, 0, sizeof(HashTableIterator));

    hti->ht = ht;

    return hti;
}

/* Deletes the current iterator.  This needs to be called after every
 * iteration has finished.  */

void Hti_Delete(HashTableIterator *hti)
{
    free(hti);
}

/* Iterates through the hash key structure; returns NULL after the
 * last element has been returned. */
HashObject *Hti_Next(HashTableIterator *hti)
{
    bool advance_along_base;

    if(unlikely(hti->ht->count == 0))
    {
	return NULL;
    }

    if(unlikely(hti->count == 0))
    {
	/* We're at the starting iteration. */
	advance_along_base = true;
	hti->top_level_pos = 0;
    }
    else
    {
	/* We're at a previous node; descend as needed. */
	
	while(1)
	{
	    /* Advance down one node and test if it's done. */
	    
	    if(hti->stack_top == 0)
	    {
		/* This node is at the base; so advance along the base. */
		++hti->top_level_pos;
		advance_along_base = true;
		break;
	    }
	    else
	    {
		--hti->stack_top;
	    }

	    if(_HNBO_localCount(hti->hn_stack[hti->stack_top]) != hti->num_stack[hti->stack_top])
	    {
		advance_along_base = false;
		
		/* Ready the node. */
		hti->loc_stack[hti->stack_top] = 
		    _HNBO_nextNode(hti->hn_stack[hti->stack_top], 
				     hti->loc_stack[hti->stack_top] + 1);
		
		++hti->num_stack[hti->stack_top];
		
		break;
	    }
	    else
	    {
		/* We're done with this node, drop down again. */
		continue;
	    }
	}
    }

    /* Now advance along the base if needed. */
    if(advance_along_base)
    {
	assert(hti->stack_top == 0);

	while(1)
	{
	    if(hti->top_level_pos == _HT_FIRST_LEVEL_SIZE(hti->ht))
	    {
		/* We're done completely. */
		assert(hti->count == hti->ht->count);
		return NULL;
	    }

	    if(hti->ht->nodes[hti->top_level_pos].count != 0)
	    {
		/* Ready the node. */
		_HashTableNode *hn = &(hti->ht->nodes[hti->top_level_pos]);
		hti->hn_stack[0]  = hn;
		hti->loc_stack[0] = _HNBO_firstOffset(hn);
		hti->num_stack[0] = 1;
		
		break;
	    }
	    
	    ++hti->top_level_pos;
	}
    }

    /* Now the stack_top node and location are poised to advance up to
     * the next value node. */

    while(1)
    {
	
	if(hti->hn_stack[hti->stack_top]->count == 1)
	{
	    ++hti->count;
	    return _HN_HashObject(hti->hn_stack[hti->stack_top]);
	}
	else
	{
	    _HashTableNode *next_hn = _HN_NextNode(hti->hn_stack[hti->stack_top], 
						   hti->loc_stack[hti->stack_top]);

	    ++hti->stack_top;
	    hti->hn_stack[hti->stack_top] = next_hn;
	    hti->loc_stack[hti->stack_top] = _HNBO_firstOffset(next_hn);
	    hti->num_stack[hti->stack_top] = 1;

	    continue;
	}
    }
}

/********************************************************************************
 * 
 *  An alternate version of the previous iterator that buffers things;
 *  this is for doing operations on the hash tree that manipulate it.
 *
 *********************************************************************************/

HashTableBufferedIterator* Htib_New(HashTable *ht)
{
    HashTableBufferedIterator *htib = (HashTableBufferedIterator*)malloc(sizeof(HashTableBufferedIterator));

    htib->count = Ht_SIZE(ht);
    htib->position = 0;
    htib->object_buffer = (HashObject**)malloc(sizeof(HashObject*)*(htib->count + 1));

    assert(htib->object_buffer != NULL);

    HashTableIterator* hti = Hti_New(ht);

    HashObject *h;

    size_t i = 0;

    while( (h = Hti_Next(hti)) != NULL)
    {
	O_INCREF(h);
	assert(i < htib->count);
	htib->object_buffer[i] = h;
	++i;
    }

    assert(i == htib->count);

    Hti_Delete(hti);

    return htib;
}

/* Iterates through the hash key structure; returns NULL after the
 * last element has been returned. */
HashObject *Htib_Next(HashTableBufferedIterator *htib)
{
    if(htib->position == htib->count)
    {
	return NULL;
    }
    else
    {
	HashObject *ret = htib->object_buffer[htib->position];
	++htib->position;
	return ret;
    }
}

/* Deletes the current iterator.  This needs to be called after every
 * iteration has finished.  */
void Htib_Delete(HashTableBufferedIterator *htib)
{
    size_t i;

    for(i = 0; i < htib->count; ++i)
	O_DECREF(htib->object_buffer[i]);

    free(htib->object_buffer);
    free(htib);
}


/********************************************************************************
 *
 *  Various other operations 
 *
 ********************************************************************************/

HashTable* Ht_Copy(HashTable* ht)
{
    HashTable *new_ht = NewSizeOptimizedHashTable(Ht_Size(ht));
    HashObject *hk;
    HashTableIterator *hti = Hti_New(ht);

    /* Could be optimized a lot. */
    
    while( (hk = Hti_Next(hti)) != NULL)
	Ht_Set(new_ht, hk);

    Hti_Delete(hti);

    return new_ht;
}

/********************************************************************************
 *
 *  Hash Table Merging Operations.
 *
 ********************************************************************************/

HashTable *Ht_ReduceTable(HashTable *ht)
{
    assert(ht != NULL);

    _HT_MarkerSkipList *msl = ht->marker_sl;
    HashTable *rht = NewHashTable();

    if(unlikely(msl == NULL))
    {
	_Ht_debug_HashTableConsistent(rht);
	_Ht_debug_HashTableConsistent(ht);

	_Ht_MSL_Init(ht);
	HashObject *h = ALLOCATEHashObject();
	Hf_COPY_FROM_KEY(h, &(ht->marker_sl->unmarked_object_hash));
	H_AddMarkerValidRange(h, MARKER_MINUS_INFTY, MARKER_PLUS_INFTY);
	Ht_Give(rht, h);

	return rht;
    }

    /* Now just walk through the marker list, adding the hash between
     * each subsequent node. */

#if (RUN_CONSISTENCY_CHECKS == 1)
    HashObject *temp_h = ALLOCATEHashObject();
#endif

    HashObject *running_h = ALLOCATEHashObject();
    Hf_COPY_FROM_KEY(running_h, &msl->unmarked_object_hash);

    _HT_MSL_Node *next_leaf;
    _HT_MSL_Node *cur_leaf = (_HT_MSL_Node *)msl->first_leaf;

    markertype cur_m, next_m;

    do{
	cur_m = cur_leaf->marker;
	Hk_REDUCE_UPDATE(H_Hash_RW(running_h), &(cur_leaf->hk));

#if (RUN_CONSISTENCY_CHECKS == 1)

	if(!H_Equal(Ht_HashAtMarkerPoint(temp_h, ht, cur_m), running_h))
	{
	    fprintf(stderr, "\n\n##############\nMarker point = %ld", cur_m);
	    Ht_debug_print(ht);
	    Ht_MSL_debug_Print(ht);
	    abort();
	}
#endif
	next_leaf = cur_leaf->next;

	if(next_leaf == NULL)
	    next_m = MARKER_PLUS_INFTY;
	else
	    next_m = next_leaf->marker;

	/* See if there is a key present with the current hash.  If
	 * so, then add this one to it.  If not, then create a new
	 * one. */

	HashObject *h = Ht_Pop(rht, running_h);
	    
	if(likely(h == NULL))
	{
	    h = ALLOCATEHashObject();
	    Hk_COPY(H_Hash_RW(h), H_Hash_RO(running_h));
	}
	else
	{
	    assert(O_REF_COUNT(h) == 1);
	    assert(!H_IsLocked(h));
	}

	H_AddMarkerValidRange(h, cur_m, next_m);
	Ht_Give(rht, h);
       
	cur_leaf = next_leaf;

#if (RUN_CONSISTENCY_CHECKS == 1)
	_Ht_debug_HashTableConsistent(rht);
	_Ht_debug_HashTableConsistent(ht);
#endif

    }while(cur_leaf != NULL);
    
#if (RUN_CONSISTENCY_CHECKS == 1)
    _Ht_debug_HashTableConsistent(rht);
    _Ht_debug_HashTableConsistent(ht);

    O_DECREF(temp_h);
#endif

    O_DECREF(running_h);

    return rht;
}

void __Ht_HAMV_SetKeyInRHT(HashTable *ht_accumulator, HashObject *h)
{
    HashObject *present_hash = Ht_Pop(ht_accumulator, h);

    if(present_hash == NULL)
    {
	if(!Mi_IsEmpty(h->mi))
	    Ht_Give(ht_accumulator, h);
    }
    else
    {
	/* Need to do some tricky business to work with possible
	 * duplication. */
	
	//assert(O_RefCount(present_hash) == 2); /* One ours + one in iterator buffer. */
	assert(!H_IsLocked(present_hash));
    
	MarkerInfo *mi_present = present_hash->mi;
	MarkerInfo *mi_new = h->mi;

	MarkerInfo *mi_single = Mi_SymmetricDifference(mi_present, mi_new);
	MarkerInfo *mi_double = Mi_Intersection(mi_present, mi_new);

	assert(!Mi_ISEMPTY(mi_present));
	assert(!Mi_ISEMPTY(mi_new));
	    
	assert(! (Mi_ISEMPTY(mi_single) && Mi_ISEMPTY(mi_double)) );

	if(likely(!Mi_ISEMPTY(mi_single)))
	{
	    H_GiveMarkerInfo(present_hash, mi_single);
	    Ht_Give(ht_accumulator, present_hash);
	}

	if(unlikely(!Mi_ISEMPTY(mi_double)))
	{
	    /* Need to recursively add it to ensure that we aren't
	     * overwriting something again. */

	    HashObject *h_double = H_Reduce(NULL, h, h);
	    H_GiveMarkerInfo(h_double, mi_double);
	    __Ht_HAMV_SetKeyInRHT(ht_accumulator, h_double);

	}
    }
}

HashTable *Ht_Summarize_Add(HashTable *ht_accumulator, HashTable *ht)
{
    /* Adds a new hash table to a hashing along accumulator values.
     * 
     */

    HashTable *marker_ht = Ht_ReduceTable(ht);

    if(unlikely(ht_accumulator == NULL))
    {
	ht_accumulator = NewHashTable();
    }

    assert(O_RefCount(marker_ht) == 1);

    HashTableIterator *hti = Hti_New(marker_ht);
    HashObject *h;

    while( (h = Hti_Next(hti)) != NULL)
	__Ht_HAMV_SetKeyInRHT(ht_accumulator, H_Rehash(NULL, h));

    O_DECREF(marker_ht);
    
    Hti_Delete(hti);

    return ht_accumulator;
}

HashTable* Ht_Summarize_Finish(HashTable *ht_accumulator)
{
    HashTable *marker_ht = Ht_ReduceTable(ht_accumulator);

    HashTable *dest_ht = NewHashTable();

    HashObject *h;

    HashTableIterator *hti = Hti_New(marker_ht);

    while( (h = Hti_Next(hti)) != NULL)
	Ht_Give(dest_ht, H_Rehash(NULL, h));

    O_DECREF(marker_ht);
    
    Hti_Delete(hti);
    O_DECREF(ht_accumulator);

    return dest_ht;
}

HashObject* Ht_HashOfEverything(HashObject* h_dest, HashTable *ht)
{
    if(h_dest == NULL)
	h_dest = NewHashObject();

    H_Clear(h_dest);

    _HT_MarkerSkipList *msl = ht->marker_sl;

    if(unlikely(msl == NULL))
    {
	_Ht_MSL_Init(ht);
	Hf_COPY_FROM_KEY(h_dest, &(ht->marker_sl->unmarked_object_hash));
	return h_dest;
    }

    /* Now just walk through the marker list, adding the hash between
     * each subsequent node. */

    Hf_COPY_FROM_KEY(h_dest, &msl->unmarked_object_hash);

    _HT_MSL_Node *cur_leaf = (_HT_MSL_Node *)(msl->first_leaf);

    assert(cur_leaf != NULL);

    do{
	if(!Hk_ISZERO(&(cur_leaf->hk)))
	{
	    Hk_InplaceCombine(H_Hash_RW(h_dest), &(cur_leaf->hk));
	    Hk_UpdateWithInt(H_Hash_RW(h_dest), cur_leaf->marker);
	}

	cur_leaf = cur_leaf->next;
    }while(cur_leaf != NULL && cur_leaf->marker != MARKER_PLUS_INFTY);

    return h_dest;
}
