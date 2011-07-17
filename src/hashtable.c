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
#include "randfunctions.h"
#include "utilities.h"
#include "types.h"
#include "bitops.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#ifdef RUN_CONSISTENCY_CHECKS

#warning "Running internal consistency checks as part of hashtable operations; some processes may be slow." 
#endif


/* Uncomment this line to run internal consistency checks on the hash
 * table structure for debugging purposes. */

/************************************************************
 *
 *  Part of the structure for the hash tables
 *
 ************************************************************/

/* Global memory pools for the small style hash table elements. */
LOCAL_MEMORY_POOL(_HT_Independent_Node);
LOCAL_MEMORY_POOL(_HT_MarkerSkipList);
LOCAL_MEMORY_POOL(_HT_MSL_Branch);
LOCAL_MEMORY_POOL(_HT_MSL_Leaf);
LOCAL_MEMORY_POOL(_HT_MSL_NodeStack);

static inline void _Ht_NonTable_Setup(HashTable *ht)
{
    /* Nothing yet, as it's all cleared to zero. */
}

static inline size_t _Ht_NextGrowthTrigger(unsigned int log2_size)
{
    return (1 << (log2_size + 1));
}

static void _Ht_Table_Setup(HashTable *ht, unsigned int log2_size)
{
    ht->_table_log2_size = log2_size;
    ht->_table_shift = 64 - log2_size;
    ht->_table_size = (1 << (ht->_table_log2_size));
    ht->_table_grow_trigger_size = _Ht_NextGrowthTrigger(ht->_table_log2_size);
    ht->table = (_HT_Node*)malloc(sizeof(_HT_Node)*(ht->_table_size));
    CHECK_MALLOC(ht->table);
    memset(ht->table, 0, sizeof(_HT_Node)*(ht->_table_size));
}

void _Ht_HashTable_Constructor(HashTable *ht)
{
    /* Just set it up for the first lavel */

    _Ht_Table_Setup(ht, _HT_INITIAL_LOG2_SIZE);
    _Ht_NonTable_Setup(ht);
}

/* A specific constructor for custom constructions.*/
HashTable* NewSizeOptimizedHashTable(size_t expected_size)
{
    size_t log2_size = bitwise_log2(expected_size);

    HashTable *ht = ALLOCATEHashTable();

    _Ht_Table_Setup(ht, max(log2_size, _HT_INITIAL_LOG2_SIZE));
    _Ht_NonTable_Setup(ht);

    return ht;
}

HashTable* NewHashTable()
{
    return NewSizeOptimizedHashTable(1 << _HT_INITIAL_LOG2_SIZE);
}

#ifdef RUN_CONSISTENCY_CHECKS
void _Ht_debug_HashTableConsistent(ht_crptr ht);
#else
inline void _Ht_debug_HashTableConsistent(ht_crptr ht) {}
#endif

/* Destructor method; called when the table is no longer needed. */
static void _Ht_Table_DeallocateChain(_HT_Independent_Node *);
void _Ht_MSL_Drop(HashTable *ht);

void _Ht_Destroy(HashTable *ht)
{
    /* First iterate over all the nodes and decref them. */

    _Ht_debug_HashTableConsistent(ht);

    if(ht->marker_sl != NULL)
	_Ht_MSL_Drop(ht);

    _HashTableInternalIterator hti;
    _Hti_INIT(ht, &hti);
    HashObject *h = NULL;

    while(_Hti_NEXT(&h, &hti))
    {
	assert(O_RefCount(h) >= 1);
	O_DECREF(h);
    }
    
    /* See if any of the nodes need deleting. */
    size_t i;
    for(i= 0; i < ht->_table_size; ++i)
	if(unlikely(ht->table[i].next_chain != NULL))
	    _Ht_Table_DeallocateChain(ht->table[i].next_chain);

    free(ht->table);
}

DEFINE_OBJECT(
    /* Name. */     HashTable,
    /* BaseType */  Object,
    /* construct */ _Ht_HashTable_Constructor,
    /* delete */    _Ht_Destroy);

/************************************************************
 * Functions for managing internal aspects of the hash table,
 * excluding individual operations on the nodes.
 ************************************************************/

static size_t _Ht_Table_Index(ht_crptr ht, uint64_t hk64)
{
    size_t idx = (size_t)(hk64 >> (ht->_table_shift));

    /* fprintf(stderr, "Size of table = %ld, shift = %d, new idx = %ld \n",  */
    /* 	   ht->_table_size, ht->_table_shift, idx); */

    assert(idx < ht->_table_size);
    return idx;
}

static inline _HT_Item _Ht_Table_MakeItem(HashObject *h)
{
    _HT_Item hi;
    hi.hk64 = H_Hash_RO(h)->hk64[HK64I(0)];
    hi.obj = h;
    return hi;
}

/* Have to go through more gymnastics on this front to prevent
 * corruption of the marker skip list when the same key is inserted
 * twice. */

typedef struct {
    HashObject *h;
    HashObject *replaced;
    bool was_inserted;
} _HT_Table_Insert_Return;

static inline void _Ht_Table_AppendUnique(_ht_node_rptr node, const _HT_Item hi)
{
    /* printf("\nInserting >>> "); */
    /* H_debug_print(hi.obj); */

    while(unlikely(node->size == _HT_ITEMS_PER_NODE))
    {
	assert(node->items[_HT_ITEMS_PER_NODE-1].hk64 <= hi.hk64);

	if(node->next_chain == NULL)
	{
	    node->next_chain = Mp_New_HT_Independent_Node();
	    node->next_chain->node.size = 1;
	    node->next_chain->node.items[0] = hi;
	    return;
	}

	node = &(node->next_chain->node);
    }

    assert(node->size < _HT_ITEMS_PER_NODE);

#ifndef NDEBUG
    if(node->size != 0)
    {
	if(Hk_GEQ(H_Hash_RO(node->items[node->size - 1].obj), H_Hash_RO(hi.obj)))
	    fprintf(stderr, "Error in item ordering; object \n%llx >= \n%llux\n", 
		    (long long unsigned int)
		    (H_Hash_RO(node->items[node->size - 1].obj)->hk64[HK64I(0)]), 
		    (long long unsigned int)
		    (H_Hash_RO(hi.obj)->hk64[HK64I(0)]));
	assert(Hk_LT(H_Hash_RO(node->items[node->size - 1].obj), H_Hash_RO(hi.obj)));
    }
#endif


    node->items[node->size] = hi;
    ++(node->size);
}


_HT_Table_Insert_Return _Ht_Table_InsertIntoOverflowNode(
    _ht_node_rptr node, const _HT_Item hi, const bool overwrite);


static inline _HT_Table_Insert_Return _Ht_Table_InsertIntoNode(
    _ht_node_rptr node, const _HT_Item hi, 
    const bool overwrite)
{
    assert(node->size <= _HT_ITEMS_PER_NODE);

    _HT_Table_Insert_Return ret;
    ret.h = hi.obj;
    ret.replaced = NULL;
    ret.was_inserted = true;

    /* Get the right location in the node list. */ 
    if(node->size == 0)
    {
	++node->size;
	node->items[0] = hi;
	return ret;
    }

    unsigned int insert_pos = 0;

    while(insert_pos < node->size && node->items[insert_pos].hk64 < hi.hk64)
    	++insert_pos;

    if(insert_pos == _HT_ITEMS_PER_NODE)
    	return _Ht_Table_InsertIntoOverflowNode(node, hi, overwrite);

    /* Now see if it's replacing one or the 64bit hash version conflicts. */
    if(unlikely(hi.hk64 == node->items[insert_pos].hk64))
    {
	if(likely(H_EQUAL(hi.obj, node->items[insert_pos].obj)))
	{
	    if(overwrite)
	    {
		ret.replaced = node->items[insert_pos].obj;
		node->items[insert_pos] = hi;
		ret.was_inserted = true;
		return ret;
	    }
	    else
	    {
		ret.h = node->items[insert_pos].obj;
		ret.was_inserted = false;
		return ret;
	    }
	}
	else 
	{
	    /* A definite corner case; occurs naturally with probability ~ 2^-64 */
	    assert(!H_Equal(hi.obj, node->items[insert_pos].obj));

	    for(;insert_pos != node->size 
		    && Hk_LT(H_Hash_RO(node->items[insert_pos].obj), H_Hash_RO(hi.obj));
		++insert_pos);

	    /* Did we run off the end?  If so, punt this one to the next node. */
	    if(insert_pos == node->size)
	    {
		if(node->size == _HT_ITEMS_PER_NODE)
		    return _Ht_Table_InsertIntoOverflowNode(node, hi, overwrite);
	    }

	    /* Deal with the case where it's equal; deal locally with
	     * this node, and we're done */
	    if(H_EQUAL(hi.obj, node->items[insert_pos].obj))
	    {
		if(overwrite)
		{
		    ret.replaced = node->items[insert_pos].obj;
		    node->items[insert_pos] = hi;
		    ret.was_inserted = true;
		    return ret;
		}
		else
		{
		    ret.h = node->items[insert_pos].obj;
		    ret.was_inserted = false;
		    return ret;
		}
	    }
	}
    }

    if(unlikely(node->size == _HT_ITEMS_PER_NODE))
	_Ht_Table_InsertIntoOverflowNode(node, node->items[_HT_ITEMS_PER_NODE-1], overwrite);
    else
	++node->size;

    unsigned int copy_dest;
    
    for(copy_dest = node->size - 1; copy_dest != insert_pos; --copy_dest)
	node->items[copy_dest] = node->items[copy_dest - 1];
	
    node->items[insert_pos] = hi;

    if(insert_pos != 0)
	assert(Hk_LT(H_Hash_RO(node->items[insert_pos-1].obj), 
		     H_Hash_RO(node->items[insert_pos].obj)));

    if(insert_pos + 1 != node->size)
	assert(Hk_LT(H_Hash_RO(node->items[insert_pos].obj), 
		     H_Hash_RO(node->items[insert_pos+1].obj)));

    assert(node->size <= _HT_ITEMS_PER_NODE);

    return ret;
}

_HT_Table_Insert_Return _Ht_Table_InsertIntoOverflowNode(
    _ht_node_rptr node, const _HT_Item hi, const bool overwrite)
{
    if(node->next_chain == NULL)
	node->next_chain = Mp_New_HT_Independent_Node();
	    
    return _Ht_Table_InsertIntoNode(&(node->next_chain->node), hi, overwrite);
}

void _Ht_Table_Grow(HashTable *ht);

static inline void _Ht_Table_GrowIfNeeded(HashTable *ht)
{
    if(unlikely(ht->size >= ht->_table_grow_trigger_size))
	_Ht_Table_Grow(ht);
}

static inline _HT_Table_Insert_Return _Ht_Table_Insert(HashTable *ht, HashObject *h, bool overwrite)
{
    _Ht_Table_GrowIfNeeded(ht);

    _HT_Item hi = _Ht_Table_MakeItem(h);
    size_t idx = _Ht_Table_Index(ht, hi.hk64);

    return _Ht_Table_InsertIntoNode(&(ht->table[idx]), hi, overwrite);
}

void _Ht_Table_Grow(HashTable *ht)
{
    _HT_Node* _restrict_ src_table = ht->table;
    const size_t src_size = ht->_table_size;

    _Ht_Table_Setup(ht, ht->_table_log2_size + 1);

    size_t i;
    unsigned int j;

    for(i = 0; i < src_size; ++i)
    {
	assert(src_table[i].size <= _HT_ITEMS_PER_NODE);
	
	for(j = 0; j < src_table[i].size; ++j)
	{
	    _HT_Item hi = src_table[i].items[j];
	    size_t idx = _Ht_Table_Index(ht, hi.hk64);
	    assert(idx < ht->_table_size);

	    _Ht_Table_AppendUnique(&(ht->table[idx]), hi);
	}

	if(unlikely(src_table[i].next_chain != NULL))
	{
	    _HT_Independent_Node *inode = src_table[i].next_chain;
	    
	    do{
		for(j = 0; j < inode->node.size; ++j)
		{
		    _HT_Item hi = inode->node.items[j];
		    size_t idx = _Ht_Table_Index(ht, hi.hk64);
		    _Ht_Table_AppendUnique(&(ht->table[idx]), hi);
		}

		inode = inode->node.next_chain;
	    }while(unlikely(inode != NULL));

	    _Ht_Table_DeallocateChain(src_table[i].next_chain);
	}
    }

    free(src_table);

    _Ht_debug_HashTableConsistent(ht);
}


void _Ht_Table_DeallocateChain(_HT_Independent_Node * _restrict_ inode)
{
    assert(inode != NULL);

    do{
	_HT_Independent_Node *next_node = inode->node.next_chain;
	Mp_Free_HT_Independent_Node(inode);
	inode = next_node;
    }while(unlikely(inode != NULL));
}

/* Now look at simply finding items. */
static inline bool _Ht_Table_Find(
    _ht_node_rptr *target_node, unsigned int *node_idx, 
    _ht_node_rptr *base_node,    HashObject **target_h, 
    const HashTable *ht, const HashKey hk)
{
    size_t idx = _Ht_Table_Index(ht, hk.hk64[HK64I(0)]);

    assert(idx < ht->_table_size);
    
    _ht_node_rptr node = &(ht->table[idx]);

    *base_node = NULL;
    *target_h = NULL;

    uint64_t hk64 = hk.hk64[HK64I(0)];

HT_TABLE_FIND_RESTART:;

    unsigned int pos = 0;

    while(pos < _HT_ITEMS_PER_NODE && node->items[pos].hk64 < hk64)
    	++pos;

    if(pos == _HT_ITEMS_PER_NODE)
    {
	if(node->next_chain != NULL)
	{
	    *base_node = node;
	    node = &(node->next_chain->node);
	    goto HT_TABLE_FIND_RESTART;
	}
	else
	    return false;
    }

    if(node->items[pos].hk64 != hk64 || node->items[pos].obj == NULL)
	return false;

    /* Now get to test if things are really equal. */
    if(likely(Hk_EQUAL(H_Hash_RO(node->items[pos].obj), &hk)))
    {
	*target_node = node;
	*target_h = node->items[pos].obj;
	*node_idx = pos;
	return true;
    }
    else
    {
	/* First make sure it's not in any of the other equal nodes. */
	while((++pos) < _HT_ITEMS_PER_NODE && node->items[pos].hk64 == hk64)
	{
	    if(node->items[pos].obj == NULL)
		return false;

	    if(likely(Hk_EQUAL(H_Hash_RO(node->items[pos].obj), &hk)))
	    {
		*target_node = node;
		*target_h = node->items[pos].obj;
		*node_idx = pos;
		return true;
	    }
	}

	if(unlikely(pos == _HT_ITEMS_PER_NODE))
	{
	    /* It may be in the next level. */
	    if(node->next_chain != NULL)
	    {
		*base_node = node;
		node = &(node->next_chain->node);
		goto HT_TABLE_FIND_RESTART;
	    }
	}

	return false;
    }
}

void _Ht_Table_SlideFromChainedNode(_ht_node_rptr node);

/* Returns true if the node is now empty, otherwise false. */
static inline bool _Ht_Table_ClearFromNode(_ht_node_rptr node, unsigned int idx)
{
    assert(node->size <= _HT_ITEMS_PER_NODE);
    assert(node->size >= 1);
    assert(idx < node->size);
    
    unsigned int i;
    for(i = idx; i + 1 < node->size; ++i)
	node->items[i] = node->items[i+1];

    if(unlikely(node->next_chain != NULL))
    {
	_Ht_Table_SlideFromChainedNode(node);
	return false;
    }
    else
    {
	assert(node->size != 0);
	--node->size;
	node->items[node->size].hk64 = 0;
	node->items[node->size].obj = NULL;
	return (node->size == 0);
    }
}

void _Ht_Table_SlideFromChainedNode(_ht_node_rptr node)
{
    assert(node->size == _HT_ITEMS_PER_NODE);
    assert(node->next_chain != NULL);
    assert(node->next_chain->node.size >= 1);

    node->items[_HT_ITEMS_PER_NODE - 1] = node->next_chain->node.items[0];

    if(_Ht_Table_ClearFromNode(&(node->next_chain->node), 0))
    {
	Mp_Free_HT_Independent_Node(node->next_chain);
	node->next_chain = NULL;
    }
}

static inline void _Ht_Table_Delete(_ht_node_rptr node, 
				    _ht_node_rptr base_node,
				    unsigned int idx, HashObject *h)
{
    if(_Ht_Table_ClearFromNode(node, idx) && unlikely(base_node != NULL))
    {
	assert(base_node->next_chain != NULL);
	assert(node == &(base_node->next_chain->node));

	Mp_Free_HT_Independent_Node(base_node->next_chain);
	base_node->next_chain = NULL;
    }
}

/********************************************************************************
 *
 *  Now interface with the marker branch.  Since this slows things
 *  down, only create it when needed. 
 *
 ********************************************************************************/


/****************************************
 *
 * Node stack operations for easily moving on the skip list
 *
 ****************************************/

static inline _HT_MSL_Branch* _HT_MSL_AsBranch(_HT_MSL_Node *n)
{
    assert(n == NULL || (n->is_branch && (!n->is_leaf)));
    return (_HT_MSL_Branch*)n;
}

static inline _HT_MSL_Leaf* _HT_MSL_AsLeaf(_HT_MSL_Node *n)
{
    assert(n == NULL || ((!n->is_branch) && n->is_leaf));
    return (_HT_MSL_Leaf*)n;
}

static inline _HT_MSL_Branch* _Ht_newMarkerBranch(HashTable* ht)
{
    _HT_MSL_Branch *br = Mp_New_HT_MSL_Branch();
#ifdef DEBUG_MODE
    br->is_branch = true;
    br->is_leaf = false;
#endif
    return br;
}

static inline void _Ht_freeMarkerBranch(HashTable *ht, _HT_MSL_Node* mb)
{
    Mp_Free_HT_MSL_Branch(_HT_MSL_AsBranch(mb));
}

/* Marker Leaf Items */
static inline _HT_MSL_Leaf* _Ht_newMarkerLeaf(HashTable* ht)
{
    _HT_MSL_Leaf *leaf = Mp_New_HT_MSL_Leaf();

#ifdef DEBUG_MODE
    leaf->is_branch = false;
    leaf->is_leaf = true;
#endif
    return leaf;
}

static inline void _Ht_freeMarkerLeaf(HashTable *ht, _HT_MSL_Leaf* mb)
{
    Mp_Free_HT_MSL_Leaf(mb);
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

static inline _HT_MSL_NodeStack* _Ht_MSL_BS_New(_HT_MSL_Node *node)
{
    _HT_MSL_NodeStack *ns = Mp_New_HT_MSL_NodeStack();

    ns->node = node;
    return ns;
}

#define _Ht_MSL_BS_Delete(msl_bs_ptr)			\
    do{							\
	_HT_MSL_NodeStack *nbs, *bs = (msl_bs_ptr);	\
	while(bs != NULL)				\
	{						\
	    nbs = bs->previous;				\
	    Mp_Free_HT_MSL_NodeStack(bs);		\
	    bs = nbs;					\
	}						\
    }while(0)


#define _Ht_MSL_BS_Pop(msl_bs_ptr_ptr)					\
    do{									\
	_HT_MSL_NodeStack *prev_bs = (*(msl_bs_ptr_ptr))->previous;	\
	Mp_Free_HT_MSL_NodeStack(*(msl_bs_ptr_ptr));			\
	*msl_bs_ptr_ptr = prev_bs;					\
    }while(0)

static inline void _Ht_MSL_BS_Push(_HT_MSL_NodeStack **ns_ptr, _HT_MSL_Node* node)
{
    _HT_MSL_NodeStack *next_bs = Mp_New_HT_MSL_NodeStack();
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

static inline unsigned int __Ht_MSL_NewEntryHeight(_HT_MarkerSkipList *msl)
{
    /* Returns the height of a new node in the skip list.  There is a
     * 1/4 probability that each node is sent to the next level up. */
    
    unsigned int height = 0;
    unsigned long r = msl->cur_rand_factor;

    while(1)
    {
	if(unlikely(r == 0)) 
	    r = Lcg_Next(&(msl->cur_rand_state));
	
	if( unlikely( ! (r & 0x00000003) ) )
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

    if(!(*ns_ptr)->is_travel_node)
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
void _Ht_MSL_Write(HashTable *ht, const HashKey *hk, const MarkerInfo *mi,
		   bool switch_add_sub_flags)
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
    MarkerRange current_mr = MARKER_RANGE_DEFAULT;
    MarkerRange next_mr = MARKER_RANGE_DEFAULT;

    MarkerRevIterator *miri = Miri_New(mi);

    HashKey addition_hk, removal_hk;
    
    if(unlikely(Hk_ISZERO(hk)))
	return;
    
    if(switch_add_sub_flags)
    {
	Hk_COPY(&removal_hk, hk);
	Hk_NEGATIVE(&addition_hk, hk);
    }
    else
    {
	Hk_COPY(&addition_hk, hk);
	Hk_NEGATIVE(&removal_hk, hk);
    }

    Miri_Next(&next_mr, miri);
    /* assert(next_mr != NULL);  This case should be caught by H_IsMarked. */

    HashKey temp;
    Hk_REDUCE(&temp, &addition_hk, &removal_hk);
    assert(Hk_IsZero(&temp));

    while(1)
    {
	/* Increment the iteration. */
	current_mr = next_mr;
	bool next_mr_okay = Miri_Next(&next_mr, miri);
	
	markertype add_loc = current_mr.start;
	markertype sub_loc = current_mr.end;

	/* printf("\nInserting "); */
	/* Hk_debug_PrintHash(&addition_hk); */
	/* printf(" at marker %ld ", add_loc); */

	/* printf("and "); */
	/* Hk_debug_PrintHash(&removal_hk); */
	/* printf(" at marker %ld\n", sub_loc); */

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

	if(unlikely(!next_mr_okay))
	    break;

	assert(next_mr.end < add_loc);

	/* Another key to do, so go back up the stack until we're at
	 * the next pivot point. */

	while(next_mr.start < _Ht_MSL_BS_CurNode(ns)->marker)
	    __Ht_MSL_BackupNodeStack(&ns, &cur_level, NULL);

    }

    /* We're done, just delete the node stack and iterator. */
    _Ht_MSL_BS_Delete(ns);
    Miri_Delete(miri);
}

/* Writes a single pair for when a range is modified.  Useful for
 * building up hash tables from sequential markers.  A simplified
 * version of the _Ht_MSL_Write(...) function. */

void _Ht_MSL_WritePair(HashTable *ht, const HashKey *hk, markertype add_loc, markertype sub_loc,
		       bool switch_add_sub_flags)
{
    /* should be checked at previous level */
    assert(ht->marker_sl != NULL);
    assert(ht->marker_sl->first_leaf != NULL);

    _HT_MarkerSkipList *msl = ht->marker_sl;
    
    /* Create the start of the node stack at the beginning. */
    _HT_MSL_NodeStack *ns = _Ht_MSL_BS_New((_HT_MSL_Node*)msl->start_node);
    unsigned int cur_level = msl->start_node_level;
    
    HashKey addition_hk, removal_hk;
    
    if(unlikely(Hk_ISZERO(hk)))
	return;

    if(switch_add_sub_flags)
    {
	Hk_COPY(&removal_hk, hk);
	Hk_NEGATIVE(&addition_hk, hk);
    }
    else
    {
	Hk_COPY(&addition_hk, hk);
	Hk_NEGATIVE(&removal_hk, hk);
    }

    HashKey temp;
    Hk_REDUCE(&temp, &addition_hk, &removal_hk);
    assert(Hk_IsZero(&temp));

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

    assert(ns->node->level == cur_level);
    assert(ns->node->marker <= add_loc);

    /* Advance down to the base; we can't update the hash at this
     * point, since InsertValue may modify it. */
    while(__Ht_MSL_AdvanceNodeStack(&ns, &cur_level, add_loc));

    /* Insert this value. */
    __Ht_MSL_InsertValue(ht, &ns, &cur_level, &addition_hk, &removal_hk, add_loc, new_h2);

    /* Go back updating the nodes to the same point as before. */
    while( (_Ht_MSL_BS_CurNode(ns)->marker >= threshhold_marker
	    || cur_level <= threshhold_level)
	   && ns->previous != NULL)
    {
	__Ht_MSL_BackupNodeStack(&ns, &cur_level, &addition_hk);
    }

    /* We're done, just delete the node stack and iterator. */
    _Ht_MSL_BS_Delete(ns);
}

static inline void _Ht_MSL_WriteKey(HashTable *ht, HashObject *h)
{
    if(ht->marker_sl != NULL)
    {
	H_ClaimMarkerLock(h);

	if(Mi_VALID_ANYWHERE(H_Mi(h)))
	    _Ht_MSL_Write(ht, H_Hash_RO(h), H_Mi(h), false);
    }
}

static inline void _Ht_MSL_DeleteKey(HashTable *ht, HashObject *h)
{
    if(ht->marker_sl != NULL)
    {
	H_ReleaseMarkerLock(h);

	if(Mi_VALID_ANYWHERE(H_Mi(h)))
	    _Ht_MSL_Write(ht, H_Hash_RO(h), H_Mi(h), true);
    }
}

void _Ht_MSL_Init(HashTable *ht)
{
    _HT_MarkerSkipList *msl = ht->marker_sl;
    assert(msl == NULL);

    msl = ht->marker_sl = Mp_New_HT_MarkerSkipList();
    
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
    msl->cur_rand_state = Lcg_New(0);
    msl->cur_rand_factor = Lcg_Next(&(msl->cur_rand_state));
    SetNodeLevel(msl->start_node, msl->start_node_level);

    /* Now go through and insert all the values. */
    _HashTableInternalIterator hti;
    HashObject *h;

    _Hti_INIT(ht, &hti);
    
    while(_Hti_NEXT(&h, &hti))
	_Ht_MSL_WriteKey(ht, h);
}

/**** needed routines that interface with the hash object stuff. ****/
void _Ht_MSL_Drop(HashTable *ht)
{
    /* Drops the current msl if needed. */

    _HT_MarkerSkipList *msl = ht->marker_sl;

    if(likely(msl != NULL))
    {
#ifndef NDEBUG	
	{
	    _HashTableInternalIterator hti;
	    _Hti_INIT(ht, &hti);
	    HashObject *h;
    
	    while(_Hti_NEXT(&h, &hti))
		H_ReleaseMarkerLock(h);
	}
#endif

	/* Walk all the structures releasing them back to the common
	 * memory pools. */

	/* Clear all the branches. */

	do {
	    assert(msl->start_node != NULL);

	    _HT_MSL_Node *br = (_HT_MSL_Node*)(msl->start_node);

	    do{
		_HT_MSL_Node *old_br = br;
		br = br->next;
		Mp_Free_HT_MSL_Branch(_HT_MSL_AsBranch(old_br));
	    }while(br != NULL);
		    
	    msl->start_node = (_HT_MSL_Branch*) msl->start_node->down;
	} while(msl->start_node != NULL);
 
	/* Now clear all the nodes. */

	assert(msl->first_leaf != NULL);

	_HT_MSL_Leaf *leaf = msl->first_leaf;
	    
	do{
	    _HT_MSL_Leaf *old_leaf = leaf;
	    leaf = (_HT_MSL_Leaf*)leaf->next;
	    Mp_Free_HT_MSL_Leaf(old_leaf);
	}while(leaf != NULL);

	/* Finally clear the msl itself. */
	Mp_Free_HT_MarkerSkipList(ht->marker_sl);
	ht->marker_sl = NULL;
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
    Hk_CLEAR(hk_dest);

    if(unlikely(loc == MARKER_PLUS_INFTY))
	return;

    if(unlikely(ht->marker_sl == NULL))
	_Ht_MSL_Init(ht);

    _HT_MarkerSkipList *msl = ht->marker_sl;

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

HashValidityItem _Htmi_NewForRangeHashing(HashTable *ht, HashTableMarkerIterator* htmi, 
					  markertype m);

void _Ht_MSL_HashOfMarkerRange(HashKey *hk_dest, HashTable *ht, markertype start, markertype end)
{
    Hk_CLEAR(hk_dest);

    if(unlikely(start == MARKER_PLUS_INFTY))
	return;

    HashTableMarkerIterator htmi;
    HashValidityItem hvi = _Htmi_NewForRangeHashing(ht, &htmi, start);

    do{
	Hk_InplaceCombinePlusTwoInts(
	    hk_dest, &(hvi.hk), 
	    max(start, hvi.start), 
	    min(end, hvi.end));

    } while(Htmi_NEXT(&hvi, &htmi) && hvi.start < end);
}

/*****************************************
 * The functions to wrap the above.
 ****************************************/


static inline HashObject* _Ht_Insert(ht_rptr ht, HashObject *h, 
				     bool overwrite, bool claim_reference)
{
    /* printf("Adding new hash key: \n"); */
    /* H_debug_print(h); */

    assert(h != NULL);
    assert(O_RefCount(h) >= 1);

    _HT_Table_Insert_Return r = _Ht_Table_Insert(ht, h, overwrite);

    if(likely(r.was_inserted))
    {
	assert(r.h == h);

	if(claim_reference)
	    O_INCREF(h);

	if(unlikely(ht->marker_sl != NULL))
	    _Ht_MSL_WriteKey(ht, h);

	if(likely(r.replaced == NULL))
	{
	    ++ht->size;
	}
	else
	{
	    assert(O_RefCount(r.replaced) >= 1);
	    
	    _Ht_MSL_DeleteKey(ht, r.replaced);
	    
	    O_DECREF(r.replaced);
	}
    }

    _Ht_debug_HashTableConsistent(ht);

    return r.h;
}

static inline void _Ht_GiveAppendUnique(ht_rptr ht, HashObject *h)
{
    _HT_Item hi = _Ht_Table_MakeItem(h);
    size_t idx = _Ht_Table_Index(ht, hi.hk64);

    _Ht_Table_AppendUnique(&(ht->table[idx]), hi);

    ++ht->size;

    if(unlikely(ht->marker_sl != NULL))
	_Ht_MSL_WriteKey(ht, h);
}

static inline void _Ht_RunKeyAsserts(const HashObject *hk)
{
    assert(hk != NULL);
    assert(O_IsType(HashObject, hk));
    assert(O_RefCount(hk) >= 1);
}

inline HashObject* Ht_ViewByKey(const HashTable *ht, HashKey hk)
{
    /* Returns the given key if it's present in the hash table; if it
     * is not preset, NULL is returned. 
     */

    /* These should be optimized out. */
    _ht_node_rptr htn = NULL, base_node = NULL;
    unsigned int node_idx = 0;
    HashObject *h;

    _Ht_Table_Find(&htn, &node_idx, &base_node, &h, ht, hk);

    if(h != NULL)
    {
	assert(O_RefCount(h) >= 1);
	if(ht->marker_sl != NULL)
	    assert(H_MarkerLockCount(h) >= 1);
    }

    _Ht_debug_HashTableConsistent(ht);

    return h;
}

HashObject* Ht_View(const HashTable *ht, const HashObject *h)
{
    _Ht_RunKeyAsserts(h);
    return Ht_ViewByKey(ht, *H_Hash_RO(h));
}

HashObject* Ht_Get(const HashTable *ht, const HashObject *hk)
{
    /* Returns the given key if it's present in the hash table; if it
     * is not preset, NULL is returned. 
     */
    HashObject *ret = Ht_View(ht, hk);

    if(ret != NULL)
	O_INCREF(ret);

    return ret;
}

static inline void _Ht_Deletion_Bookkeeping(
    HashTable *ht, HashObject *h, const bool decref)
{
    --(ht->size);

    assert(O_RefCount(h) >= 1);

    if(decref)
	O_DECREF(h);
	
    _Ht_debug_HashTableConsistent(ht);
}

static inline HashObject* _Ht_Pop(HashTable *ht, const HashKey hk, bool decref)
{
    /* Finds the given key in the hash table, deletes it from the tree
     * and returns it.  The caller would then own a reference to the
     * item.
     */

    _ht_node_rptr node = NULL, base_node = NULL;
    unsigned int node_idx = 0;
    HashObject *h;

    _Ht_Table_Find(&node, &node_idx, &base_node, &h, ht, hk);

    if(h != NULL)
    {
	_Ht_Table_Delete(node, base_node, node_idx, h);

	if(ht->marker_sl != NULL)
	{
	    assert(H_MarkerLockCount(h) >= 1);
	    assert(H_MarkerLockCount(h) <= O_RefCount(h));
	}
    
	if(ht->marker_sl != NULL)
	    _Ht_MSL_DeleteKey(ht, h);

	/* Must be called last. */
	_Ht_Deletion_Bookkeeping(ht, h, decref);
    }

    return h;
}

/* Deletes the given key from the hash table.  Returns true if the key
 * was present and false otherwise.
 */
HashObject* Ht_Pop(HashTable *ht, const HashObject *h)
{
    _Ht_RunKeyAsserts(h);
    return _Ht_Pop(ht, *H_Hash_RO(h), false);
}

HashObject* Ht_PopByKey(HashTable *ht, HashKey hk)
{
    return _Ht_Pop(ht, hk, false);
}

bool Ht_Clear(HashTable *ht, const HashObject *h)
{
    _Ht_RunKeyAsserts(h);
    return (_Ht_Pop(ht, *H_Hash_RO(h), true) != NULL);
}

bool Ht_ClearByKey(HashTable *ht, HashKey hk)
{
return (_Ht_Pop(ht, hk, true) != NULL);
}

void Ht_Give(HashTable *ht, HashObject *h)
{
    /* Adds the given key to the hash table.  Overwrites the key if it
     * is present already.
     *
     * Need to protect against regiving, i.e. doing a get and then a
     * give.  Thus the assumption is that the reference count does not
     * change using this function.
     */

    _Ht_Insert(ht, h, true, false);

    assert(ht->marker_sl == NULL || H_MarkerLockCount(h) >= 1);
    assert(O_RefCount(h) >= 1);
    
    _Ht_debug_HashTableConsistent(ht);
}

/* Adds the given key to the hash table.  Overwrites the key if it is
 * present already.  Increments the reference count.
 */

void Ht_Set(HashTable *ht, HashObject *h)
{    
    _Ht_Insert(ht, h, true, true);
}

HashObject* Ht_SetDefault(HashTable *ht, HashObject *h)
{
    return _Ht_Insert(ht, h, false, true);
}

size_t Ht_Size(ht_crptr ht)
{ 
    return Ht_SIZE(ht);
}

bool Ht_Contains(ht_crptr ht, const HashObject *hk)
{
    return (Ht_View(ht, hk) == NULL) ? false : true;
}

bool Ht_ContainsByKey(ht_crptr ht, HashKey hk)
{
    return (Ht_ViewByKey(ht, hk) == NULL) ? false : true;
}

bool Ht_ContainsAtByKey(ht_crptr ht, HashKey hk, markertype m)
{
    _Ht_debug_HashTableConsistent(ht);

    HashObject *h = Ht_ViewByKey(ht, hk);

    return (h != NULL) ? H_MarkerPointIsValid(h, m) : false;
}

bool Ht_ContainsAt(ht_crptr ht, const HashObject * hk, markertype m)
{
    _Ht_debug_HashTableConsistent(ht);

    HashObject *h = Ht_View(ht, hk);

    return (h != NULL) ? H_MarkerPointIsValid(h, m) : false;
}

HashObject* Ht_InsertValidRange(HashTable *ht, HashObject *hk, 
				markertype r_start, markertype r_end)
{
    if(ht->marker_sl == NULL)
    {
	HashObject *k = Ht_View(ht, hk);
    
	if(k == NULL)
	{
	    H_ADD_MARKER_VALID_RANGE(hk, r_start, r_end);
	    Ht_Set(ht, hk);
	    _Ht_debug_HashTableConsistent(ht);
	    return hk;
	}
	else
	{
	    H_ADD_MARKER_VALID_RANGE(k, r_start, r_end);
	    _Ht_debug_HashTableConsistent(ht);
	    return k;
	}
    }
    else
    {
	HashObject *k = Ht_Pop(ht, hk);
    
	if(k == NULL)
	{
	    H_ADD_MARKER_VALID_RANGE(hk, r_start, r_end);
	    Ht_Set(ht, hk);
	    _Ht_debug_HashTableConsistent(ht);
	    return hk;
	}
	else
	{
	    H_ADD_MARKER_VALID_RANGE(k, r_start, r_end);
	    Ht_Give(ht, k);
	    _Ht_debug_HashTableConsistent(ht);
	    return k;
	}
    }
}

/* To be used internally to speed up some things. */
static inline HashObject* _Ht_InsertValidNonOverlappingRange(
    HashTable *ht, HashObject *hk, markertype r_start, markertype r_end)
{
    HashObject *k = Ht_View(ht, hk);

    if(k == NULL)
    {
	H_GIVE_MARKER_INFO(hk, Mi_NEW(r_start, r_end));
	Ht_Set(ht, hk);
	return hk;
    }
    else
    {
	if(ht->marker_sl == NULL)
	    H_ADD_MARKER_VALID_RANGE(k, r_start, r_end);
	else
	{
	    H_ReleaseMarkerLock(k);
	    assert(H_MarkerLockCount(k) == 0);
	    _Ht_MSL_WritePair(ht, H_Hash_RO(k), r_start, r_end, false);
	    H_ClaimMarkerLock(k);
	}

	O_DECREF(hk);

	_Ht_debug_HashTableConsistent(ht);
    }
    return k;
}


/********************************************************************************
 *
 *  Various other operations 
 *
 ********************************************************************************/

HashTable* Ht_Copy(ht_crptr ht)
{
    ht_rptr new_ht = NewSizeOptimizedHashTable(Ht_Size(ht));
    HashObject *h;
    _HashTableInternalIterator hti;
    _Hti_INIT(ht, &hti);

    while(_Hti_NEXT(&h, &hti))
	_Ht_GiveAppendUnique(new_ht, h);

    return new_ht;
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

HashObject* Ht_HashOfMarkerRange(HashObject *h_dest, HashTable *ht, 
				 markertype start, markertype end)
{
    if(h_dest == NULL)
	h_dest = ConstructHashObject();

    _Ht_MSL_HashOfMarkerRange(H_Hash_RW(h_dest), ht, start, end);

    return h_dest;
}

bool Ht_EqualAtMarker(ht_rptr ht1, ht_rptr ht2, markertype m)
{
    if(unlikely(ht1 == ht2))
	return true;

    assert(O_RefCount(ht1) >= 1);
    assert(O_RefCount(ht2) >= 1);

    HashKey hk1, hk2;
    _Ht_MSL_HashAtMarkerPoint(&hk1, ht1, m);
    _Ht_MSL_HashAtMarkerPoint(&hk2, ht2, m);

    return Hk_EQUAL(&hk1, &hk2);
}

HashObject* Ht_HashOfEverything(HashObject* h_dest, ht_rptr ht)
{
    if(h_dest == NULL)
	h_dest = NewHashObject();

    H_Clear(h_dest);

    HashTableMarkerIterator htmi;
    Htmi_INIT(ht, &htmi);

    HashValidityItem hvi;

    while(Htmi_NEXT(&hvi, &htmi))
    {
	Hk_InplaceCombinePlusTwoInts(
	    H_Hash_RW(h_dest), &(hvi.hk), hvi.start, hvi.end);
    }

    return h_dest;
}


/****************************************
 *
 *  Debug routines 
 *
 ****************************************/

void Ht_Print(HashTable *ht)
{
    _HashTableInternalIterator hti;
    _Hti_INIT(ht, &hti);
    
    HashObject *h;

    while(_Hti_NEXT(&h, &hti))
    {
	H_debug_print(h);
	printf("\n");
    }
}


void Ht_debug_Print(HashTable *ht)
{
    printf("\n");
    printf("HashTable %lx:\n", (size_t)ht);
    
    _HashTableInternalIterator hti;
    _Hti_INIT(ht, &hti);
    
    HashObject *h;

    while(_Hti_NEXT(&h, &hti))
    {
	printf("   ");
	H_debug_print(h);
	printf("\n");
    }
    
    Ht_MSL_debug_Print(ht);
}

void Ht_MSL_debug_Print(HashTable *ht)
{
    printf("\n");

    bool drop_msl = false;

    if(ht->marker_sl == NULL)
    {
	_Ht_MSL_Init(ht);
	drop_msl = true;
    }

    _HT_MarkerSkipList *msl = ht->marker_sl;

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

    if(drop_msl)
	_Ht_MSL_Drop(ht);
}

void Ht_MSL_debug_PrintNodeStack(_HT_MSL_NodeStack *ns)
{
    printf("NodeStack %lxud:", (size_t)ns);

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


#ifdef RUN_CONSISTENCY_CHECKS

size_t __Ht_debug_CountChainedTableNodes(const _HT_Node * hn)
{
    size_t s = hn->size;
    
    if(hn->next_chain != NULL)
	s += __Ht_debug_CountChainedTableNodes(&(hn->next_chain->node));
    
    return s;
}

void _Ht_debug_HashTableConsistent(const HashTable *_ht)
{
    /* Trust me, I know what I'm doing. */
    HashTable *ht = (HashTable*)_ht;

    /* Go through the table and check sizes. */
    size_t i, s = 0;

    for(i = 0; i < ht->_table_size; ++i)
	s += __Ht_debug_CountChainedTableNodes(&(ht->table[i]));

    if(s != ht->size)
    {
	fprintf(stderr, "Actual table count = %ld, tracked size count = %ld.\n",
	       s, ht->size);

	abort();
    }

    _HashTableInternalIterator hti;
    _Hti_INIT(ht, &hti);

    HashObject *h;
    s = 0;

    while(_Hti_NEXT(&h, &hti))
    {
	++s;
	assert(O_IsType(HashObject, h));
	assert(O_RefCount(h) >= 1);
	assert(ht->marker_sl == NULL || H_MarkerLockCount(h) >= 1);
	
	assert(hti.debug_current_node->size != 0);
	assert(hti.debug_current_node->items[hti.debug_current_index].obj == h);
	assert(hti.debug_current_node->items[hti.debug_current_index].hk64 == H_Hash_RO(h)->hk64[HK64I(0)]);
    }
    
    if(s != ht->size)
    {
	fprintf(stderr, "Iterator covered %ld elements, size = %ld.\n", s, ht->size);
	
	abort();
    }

    // Now step through to make sure things are working 
    bool clear_marker = false;
    
    if(ht->marker_sl == NULL)
    {
	return;
	_Ht_MSL_Init(ht);
	clear_marker = true;
    }

    _HT_MarkerSkipList *msl = ht->marker_sl;

    /* Now just walk through the marker list, adding the hash between
     * each subsequent node. */

    HashObject *temp_h = ALLOCATEHashObject();
    HashObject *running_h = ALLOCATEHashObject();


    _HT_MSL_Node *next_leaf;
    _HT_MSL_Node *cur_leaf = (_HT_MSL_Node *)msl->first_leaf;
    
    /* HashTableMarkerIterator htmi; */
    /* HashValidityItem hv; */
    /* Htmi_INIT(ht, &htmi); */
    /* Htmi_NEXT(&hv, &htmi); */

    markertype cur_m, next_m;

    do{
	cur_m = cur_leaf->marker;
	Hk_REDUCE_UPDATE(H_Hash_RW(running_h), &(cur_leaf->hk));

	/* if(!Hk_Equal(H_Hash_RO(running_h), &(hv.hk))) */
	/* { */
	/*     printf("\nError; htmi != hash at point = %ld\n", cur_m); */

	/*     printf("\n running hash = "); */
	/*     H_debug_print(running_h); */

	/*     printf("\n Htmi hash = "); */
	/*     Hk_debug_PrintHash(&hv.hk); */

	/*     abort(); */
	/* } */

	Ht_HashAtMarkerPoint(temp_h, ht, cur_m);

	if(!H_Equal(temp_h, running_h))
	{
	    printf("\n\n##############\nMarker point = %ld", cur_m);
	    printf("\n calc hash = ");
	    H_debug_print(running_h);

	    printf("\n retrieved hash = ");
	    H_debug_print(temp_h);

	    printf("\n Hash table = ");

	    Ht_debug_Print(ht);
	    abort();
	}

	/* printf("\n running hash = "); */
	/* H_debug_print(running_h); */
	/* printf("[%ld, %ld)", cur_m, next_m); */

	/* printf("\n Htmi hash = "); */
	/* Hk_debug_PrintHash(&hv.hk); */
	/* printf("[%ld, %ld)", hv.start, hv.end); */

	next_leaf = cur_leaf->next;

	if(next_leaf == NULL)
	    next_m = MARKER_PLUS_INFTY;
	else
	    next_m = next_leaf->marker;

	cur_leaf = next_leaf;

	/* if(cur_leaf == NULL) */
	/*     assert(!okay); */
	/* else */
	/*     assert(okay); */

    }while(cur_leaf != NULL);

    O_DECREF(temp_h);
    O_DECREF(running_h);

    if(clear_marker)
	_Ht_MSL_Drop(ht);

}
#endif


/********************************************************************************
 * 
 *  Iteration functions to go through all nodes in the tree.
 *
 *********************************************************************************/


HashTableIterator* Hti_New(ht_rptr ht)
{
    HashTableIterator *hti = (HashTableIterator *)malloc(sizeof(HashTableIterator));
    
    _Hti_INIT(ht, &(hti->hti));

    hti->ht = ht;

    O_INCREF(ht);

    return hti;
}

/* Iterates through the hash key structure; returns NULL after the
 * last element has been returned. */
bool Hti_Next(HashObject **_h_ptr, HashTableIterator *hti)
{
    return Hti_NEXT(_h_ptr, hti);
}

void Hti_Delete(HashTableIterator *hti)
{
    O_DECREF(hti->ht);
    free(hti);
}

/********************************************************************************
 * 
 *  An alternate version of the previous iterator that buffers things;
 *  this is for doing operations on the hash tree that manipulate it.
 *
 *********************************************************************************/

HashTableBufferedIterator* Htib_New(ht_crptr ht)
{
    _Ht_debug_HashTableConsistent(ht);

    HashTableBufferedIterator *htib = 
	(HashTableBufferedIterator*)malloc(sizeof(HashTableBufferedIterator));

    htib->left = Ht_SIZE(ht);
    htib->object_buffer = (HashObject**)malloc(sizeof(HashObject*)*(htib->left));

    assert(htib->object_buffer != NULL);

    _HashTableInternalIterator hti;
    _Hti_INIT(ht, &hti);

    HashObject *h;

    size_t i = 0;

    while(_Hti_NEXT(&h, &hti))
    {
	O_INCREF(h);
	assert(i < htib->size);
	htib->object_buffer[i] = h;
	++i;
    }

    assert(i == ht->size);

    htib->next = htib->object_buffer;
    htib->size = ht->size;

    return htib;
}

/* Iterates through the hash key structure; returns NULL after the
 * last element has been returned. */
bool Htib_Next(HashObject** h_ptr, HashTableBufferedIterator *htib)
{
    return Htib_NEXT(h_ptr, htib);
}

/* Deletes the current iterator.  This needs to be called after every
 * iteration has finished.  */
void Htib_Delete(HashTableBufferedIterator *htib)
{
    HashObject **iter = htib->object_buffer;

    size_t i;
    for(i = 0; i < htib->size; ++i, ++iter)
	O_DECREF(*iter);

    free(htib->object_buffer);
    free(htib);
}

/**********************************************************************
 *
 *  Set functions for the hashtables.
 *
 **********************************************************************/

ht_rptr Ht_Intersection(ht_crptr ht1, ht_crptr ht2)
{
    /* Simply iterate through the two nodes, checking which is
     * equal. */

    ht_rptr ht_dest = NewSizeOptimizedHashTable( min(Ht_SIZE(ht1), Ht_SIZE(ht2)) );
    
    _HashTableInternalIterator hti1, hti2;

    _Hti_INIT(ht1, &hti1);
    _Hti_INIT(ht2, &hti2);

    HashObject *h1, *h2;

    bool okay = _Hti_NEXT(&h1, &hti1) && _Hti_NEXT(&h2, &hti2);

    if(likely(okay))
    {
	while(1)
	{
	    while(!H_EQUAL(h1, h2))
	    {
		bool h1_first = Hk_LT(H_Hash_RO(h1), H_Hash_RO(h2));
		if(unlikely(!_Hti_NEXT(h1_first ? &h1 : &h2, h1_first ? &hti1 : &hti2)))
		    goto HT_INTERSECTION_DONE;
	    }

	    assert(H_EQUAL(h1, h2));

	    MarkerInfo *mi = Mi_Intersection(H_Mi(h1), H_Mi(h2));

	    if(!Mi_ISEMPTY(mi))
	    {
		HashObject * _restrict_ new_h = H_COPY_AS_UNMARKED(NULL, h1);
		H_GIVE_MARKER_INFO(new_h, mi);
		_Ht_GiveAppendUnique(ht_dest, new_h);
	    }
	    else
	    {
		O_DECREF(mi);
	    }

	    if(unlikely(! (_Hti_NEXT(&h1, &hti1) && _Hti_NEXT(&h2, &hti2))))
		break;
	}
    }
HT_INTERSECTION_DONE:;

    _Ht_debug_HashTableConsistent(ht_dest);
    
    return ht_dest;
}

ht_rptr Ht_IntersectionUpdate(ht_rptr ht_accumulator, ht_crptr ht_src)
{
    if(unlikely(ht_accumulator == NULL))
	return Ht_Copy(ht_src);

    ht_rptr ht_temp = Ht_Intersection(ht_accumulator, ht_src);

    Ht_SWAP(ht_temp, ht_accumulator);

    O_DECREF(ht_temp);
    
    return ht_accumulator;
}

ht_rptr Ht_Union(ht_crptr ht1, ht_crptr ht2)
{
    /* Simply iterate through the two nodes, checking which is
     * equal. */

    ht_rptr ht_dest = NewSizeOptimizedHashTable( 
	max(Ht_SIZE(ht1), Ht_SIZE(ht2)) + (min(Ht_SIZE(ht1), Ht_SIZE(ht2)) >> 1));
    
    _HashTableInternalIterator hti1, hti2;

    _Hti_INIT(ht1, &hti1);
    _Hti_INIT(ht2, &hti2);

    HashObject *h1 = NULL, *h2 = NULL;

    if(unlikely(!_Hti_NEXT(&h1, &hti1)))
	goto HT_FINISH_OUT_H2;


    if(unlikely(!_Hti_NEXT(&h2, &hti2)))
	goto HT_FINISH_OUT_H1;

    while(1)
    {
	while(Hk_LT(H_Hash_RO(h1), H_Hash_RO(h2)))
	{
	    _Ht_GiveAppendUnique(ht_dest, H_COPY(NULL, h1));

	    if(unlikely(!_Hti_NEXT(&h1, &hti1)))
		goto HT_FINISH_OUT_H2;
	}

	while(Hk_LT(H_Hash_RO(h2), H_Hash_RO(h1)))
	{
	    _Ht_GiveAppendUnique(ht_dest, H_COPY(NULL, h2));

	    if(unlikely(!_Hti_NEXT(&h2, &hti2)))
		goto HT_FINISH_OUT_H1;
	}
	    
	if(H_EQUAL(h1, h2))
	{
	    MarkerInfo *mi = Mi_Union(H_Mi(h1), H_Mi(h2));
	    HashObject * _restrict_ new_h = H_COPY_AS_UNMARKED(NULL, h1);
	    H_GIVE_MARKER_INFO(new_h, mi);
	    _Ht_GiveAppendUnique(ht_dest, new_h);

	    int h1_okay = likely(_Hti_NEXT(&h1, &hti1)) ? 0 : 1;
	    int h2_okay = likely(_Hti_NEXT(&h2, &hti2)) ? 0 : 2;

	    switch(likely_value(h1_okay + h2_okay, 0))
	    {
	    case 0: continue;
	    case 1: goto HT_FINISH_OUT_H2;
	    case 2: goto HT_FINISH_OUT_H1;
	    case 3: goto HT_UNION_DONE;
	    default: break;
	    }
	}
    }

    /* Finish out h1. */
HT_FINISH_OUT_H1:;
    do{
	_Ht_GiveAppendUnique(ht_dest, H_COPY(NULL, h1));
    }while(_Hti_NEXT(&h1, &hti1));
    goto HT_UNION_DONE;

    /* Finish out h2. */
HT_FINISH_OUT_H2:;
    do{
	_Ht_GiveAppendUnique(ht_dest, H_COPY(NULL, h2));
    }while(_Hti_NEXT(&h2, &hti2));

    goto HT_UNION_DONE;

HT_UNION_DONE:;
    
    _Ht_debug_HashTableConsistent(ht_dest);
    
    return ht_dest;
}

ht_rptr Ht_UnionUpdate(ht_rptr ht_accumulator, ht_crptr ht_src)
{
    if(unlikely(ht_accumulator == NULL))
	return Ht_Copy(ht_src);

    ht_rptr ht_temp = Ht_Union(ht_accumulator, ht_src);

    Ht_SWAP(ht_temp, ht_accumulator);

    O_DECREF(ht_temp);
    
    return ht_accumulator;
}

ht_rptr Ht_DifferenceUpdate(ht_rptr ht1, ht_crptr ht2)
{
    if(unlikely(ht1 == NULL))
	return NewHashTable();

    /* Clear out the msl if needed. */
    if(ht1->marker_sl != NULL)
	_Ht_MSL_Drop(ht1);

    _ht_node_rptr target_node, base_node;
    unsigned int node_idx; 
    HashObject *target_h;

    _HashTableInternalIterator hti;
    HashObject *h1;
    _Hti_INIT(ht2, &hti);

    while(_Hti_NEXT(&h1, &hti) )
    {
	bool found = _Ht_Table_Find(&target_node, &node_idx, &base_node, 
				    &target_h, ht1, *H_Hash_RO(h1));
	
	if(!found)
	    continue;

	MarkerInfo *new_mi = Mi_Difference(H_Mi(target_h), H_Mi(h1));
	
	if(Mi_ISEMPTY(new_mi))
	{
	    _Ht_Table_Delete(target_node, base_node, node_idx, target_h);
	    _Ht_Deletion_Bookkeeping(ht1, target_h, true);
	}
	else
	{
	    H_GIVE_MARKER_INFO(target_h, new_mi);
	}
    }

    return ht1;
}

ht_rptr Ht_Difference(ht_crptr ht1, ht_crptr ht2)
{
    HashTable *ht = Ht_Copy(ht1);
    return Ht_DifferenceUpdate(ht, ht2);
}

void Ht_Swap(ht_rptr ht1, ht_rptr ht2)
{
    Ht_SWAP(ht1, ht2);
}

ht_rptr Ht_KeySet(ht_rptr ht)
{
    ht_rptr new_ht = NewSizeOptimizedHashTable(Ht_Size(ht));
    HashObject *h;
    _HashTableInternalIterator hti;
    _Hti_INIT(ht, &hti);

    while(_Hti_NEXT(&h, &hti))
	_Ht_GiveAppendUnique(new_ht, H_COPY_AS_UNMARKED(NULL, h));

    return new_ht;
}


/********************************************************************************
 *
 *  Iterators for the HashTable Marker stuff.
 *
 ********************************************************************************/

LOCAL_MEMORY_POOL(HashTableMarkerIterator);

HashTableMarkerIterator* Htmi_New(HashTable *ht)
{
    HashTableMarkerIterator *htmi = Mp_NewHashTableMarkerIterator();

    O_INCREF(ht);

    Htmi_INIT(ht, htmi);
    return htmi;
}

HashValidityItem _Htmi_NewForRangeHashing(HashTable *ht, HashTableMarkerIterator* htmi, 
					  markertype m)
{
    htmi->ht = ht;

    /* First need to advance to the location on the stack that is
     * before m */
    if(unlikely(ht->marker_sl == NULL))
	_Ht_MSL_Init(ht);

    _HT_MarkerSkipList *msl = ht->marker_sl;

    Hk_CLEAR(&(htmi->current_item.hk));

    /* Just travel down the tree to the node before this one. */
    _HT_MSL_NodeStack *ns = _Ht_MSL_BS_New((_HT_MSL_Node*)msl->start_node);
    unsigned int cur_level = msl->start_node_level;

    while(__Ht_MSL_AdvanceNodeStack(&ns, &cur_level, m));
    
    assert(ns->node->marker <= m);

    /* Set up the hash key stuff. */
    htmi->next = _Ht_MSL_BS_NextNode(ns);
    htmi->current_item.start = _Ht_MSL_BS_CurNode(ns)->marker;
    htmi->current_item.end   = (htmi->next != NULL) ? htmi->next->marker : MARKER_PLUS_INFTY;

    /* Get the hash value stuff ready. */
    Hk_CLEAR(&(htmi->current_item.hk));

    /* Include the tip leaf, and then all travel nodes past that. */
    Hk_REDUCE_UPDATE(&(htmi->current_item.hk), &(_Ht_MSL_BS_CurNode(ns)->hk));
    _Ht_MSL_BS_Pop(&ns);

    while(ns != NULL)
    {
	if(ns->is_travel_node)
	    Hk_REDUCE_UPDATE(&(htmi->current_item.hk), &(_Ht_MSL_BS_CurNode(ns)->hk));

	_Ht_MSL_BS_Pop(&ns);
    }

    if(likely(htmi->next != NULL) )
    {
	/* Now got to configure this one so we don't include a 0 hash
	 * between end and start. */
	while(unlikely(Hk_ISZERO(&(htmi->next->hk)))) 
	{
	    htmi->next = (_HT_MSL_Node*)(htmi->next->next);
	
	    if(unlikely(htmi->next == NULL))
	    {
		htmi->current_item.end = MARKER_PLUS_INFTY;
		break;
	    }
	
	    htmi->current_item.end = htmi->next->marker;
	}
    }

    /* And we're done! */

#ifdef RUN_CONSISTENCY_CHECKS
    HashTableMarkerIterator htmi2;
    Htmi_INIT(ht, &htmi2);

    HashValidityItem hvi;

    do{
	assert(Htmi_NEXT(&hvi, &htmi2));
    }while(hvi.end <= m);
    
    assert(htmi2.current_item.start <= htmi->current_item.start);
    assert(htmi2.current_item.end == htmi->current_item.end);
    assert(Hk_Equal(&(htmi2.current_item.hk), &(htmi->current_item.hk)));
#endif

    return htmi->current_item;
}


inline void Htmi_Finish(HashTableMarkerIterator* htmi)
{
    O_DECREF(htmi->ht);
    Mp_FreeHashTableMarkerIterator(htmi);
}


/********************************************************************************
 *
 *  Now functions for working on the HashSequence stuff.
 *
 ********************************************************************************/

LOCAL_MEMORY_POOL(_HS_Node);
LOCAL_MEMORY_POOL(HashSequenceIterator);

void _Hs_Constructor(HashSequence* hs)
{
    hs->end_node = &(hs->nodes);
    hs->size = 1;
    hs->nodes.size = 1;

    /* Sets marker minus infty hash to 0; may be updated. */
    hs->nodes.items[0].marker = MARKER_MINUS_INFTY;
}

void _Hs_Destructor(HashSequence* hs)
{
    _HS_Node* dealloc_start = hs->nodes.next;
    _HS_Node* dealloc_x;

    while(dealloc_start != NULL) {
	dealloc_x = dealloc_start;
	dealloc_start = dealloc_start->next;
	Mp_Free_HS_Node(dealloc_x);
    }
}

DEFINE_OBJECT(
    /* Name. */     HashSequence,
    /* BaseType */  Object,
    /* construct */ _Hs_Constructor,
    /* delete */    _Hs_Destructor);

/* Now the iterator stuff to go along with it. */

inline HashSequenceIterator* Hsi_New(HashSequence *hs)
{
    HashSequenceIterator *hsi = Mp_NewHashSequenceIterator();

    Hsi_INIT(hs, hsi);
    
    O_INCREF(hs);

    return hsi;
}

bool Hsi_Next(HashValidityItem *hvi, HashSequenceIterator* hsi)
{
    return Hsi_NEXT(hvi, hsi);
}

void Hsi_Finish(HashSequenceIterator* hsi)
{
    O_DECREF(hsi->hs);
    Mp_FreeHashSequenceIterator(hsi);
}

static inline void _Hs_Append(HashSequence *hs, markertype m, HashKey *hk_ptr)
{
    _HS_Node *end_node = hs->end_node;

    assert(hs->size >= 1);
    assert(end_node->size >= 1);

    if(unlikely(Hk_EQUAL(hk_ptr, &(end_node->items[end_node->size - 1].hk))))
	return;

    if(unlikely(end_node->items[end_node->size - 1].marker == m))
    {
	end_node->items[end_node->size-1].hk = *hk_ptr;
	return;
    }

    if(unlikely(end_node->size == _HS_NODE_SIZE))
    {
	/* Put it on the next one. */
	assert(end_node->next == NULL);
	hs->end_node = end_node = (end_node->next = Mp_New_HS_Node());
	end_node->size = 1;
	end_node->items[0].hk = *hk_ptr;
	end_node->items[0].marker = m;
	++(hs->size);
    }
    else
    {
	assert(end_node->size < _HS_NODE_SIZE);
	end_node->items[end_node->size].hk = *hk_ptr;
	end_node->items[end_node->size].marker = m;
	++(end_node->size);
	++(hs->size);
    }
}

HashSequence* Hs_FromHashTable(HashTable *ht)
{
    assert(ht != NULL);

    HashSequence *hs = ConstructHashSequence();

    HashTableMarkerIterator *htmi = Htmi_New(ht);
    HashValidityItem hvi;
    
    while(Htmi_NEXT(&hvi, htmi))
	_Hs_Append(hs, hvi.start, &(hvi.hk));

    Htmi_Finish(htmi);

    return hs;
}

static inline void _Hs_Swap(HashSequence* _restrict_ hs1, HashSequence* _restrict_ hs2)
{    
    if(unlikely(hs1 == hs2)) return;

    _HS_Node* end_node = hs1->end_node;
    hs1->end_node  = hs2->end_node;
    hs2->end_node  = end_node;

    _HS_Node temp_node;
    /* Swap the node block. */
    memcpy(&temp_node,     &(hs1->nodes), sizeof(_HS_Node));
    memcpy(&(hs1->nodes), &(hs2->nodes),  sizeof(_HS_Node));
    memcpy(&(hs2->nodes), &temp_node,     sizeof(_HS_Node));
}

void Hs_debug_print(HashSequence *hs)
{
    HashSequenceIterator hsi;
    Hsi_INIT(hs, &hsi);
    
    HashValidityItem hvi;

    printf("Hash Sequence = \n");

    while(Hsi_NEXT(&hvi, &hsi)) 
    {
	printf("[%ld, %ld): ", hvi.start, hvi.end);
	Hk_debug_PrintHash(&hvi.hk);
	printf("\n");
    }
}

typedef void (*hash_combine_func)(hk_ptr, chk_ptr, chk_ptr);

static inline HashSequence* _restrict_ _Hs_Update(
    HashSequence* _restrict_ hs, HashTable *ht, hash_combine_func hash_func)
{
    if(unlikely(hs == NULL))
	return Hs_FromHashTable(ht);

    HashSequence * _restrict_ hs_dest;
    
    if(unlikely(hs->size == 0))
    {
	hs_dest = Hs_FromHashTable(ht);
	_Hs_Swap(hs, hs_dest);
	O_DECREF(hs_dest);
	return hs;
    }

    assert(ht != NULL);

    hs_dest = ConstructHashSequence();

    HashTableMarkerIterator htmi;
    Htmi_INIT(ht, &htmi);
    HashValidityItem ht_hvi;
    Htmi_NEXT(&ht_hvi, &htmi);

    HashSequenceIterator hsi;
    Hsi_INIT(hs, &hsi);
    HashValidityItem hs_hvi;
    Hsi_NEXT(&hs_hvi, &hsi);

    assert(ht_hvi.start == MARKER_MINUS_INFTY);
    assert(hs_hvi.start == MARKER_MINUS_INFTY);

    markertype cur_m = MARKER_MINUS_INFTY;

    HashKey insert_hash;
    /* printf(">>>>>>>>>>>>>>>>>>>>>>>>>Init: \ncur_m = %ld, ht = [%ld, %ld), hs = [%ld, %ld)\n", */
    /* 	   cur_m, ht_hvi.start, ht_hvi.end, hs_hvi.start, hs_hvi.end); */
    

    while(true)
    {
	assert(cur_m >= ht_hvi.start);
	assert(cur_m < ht_hvi.end);
	assert(cur_m >= hs_hvi.start);
	assert(cur_m < hs_hvi.end);

	hash_func(&insert_hash, &(hs_hvi.hk), &(ht_hvi.hk));
	_Hs_Append(hs_dest, cur_m, &insert_hash);

	/* printf("Appending at %ld: ", cur_m); */
	/* Hk_debug_PrintHash(&insert_hash); */
	/* printf("From: \nhs = "); */
	/* Hk_debug_PrintHash(&(hs_hvi.hk)); */
	/* printf("ht = "); */
	/* Hk_debug_PrintHash(&(ht_hvi.hk)); */

	/* Now advance the current marker location to the next part. */

	cur_m = min(ht_hvi.end, hs_hvi.end);

	/* printf("1: \ncur_m = %ld, ht = [%ld, %ld), hs = [%ld, %ld)\n", */
	/*    cur_m, ht_hvi.start, ht_hvi.end, hs_hvi.start, hs_hvi.end); */

	if(unlikely(cur_m == MARKER_PLUS_INFTY))
	    break;

	if(ht_hvi.end <= cur_m)
	{
	    Htmi_NEXT(&ht_hvi, &htmi);

	    /* printf("2: \ncur_m = %ld, ht = [%ld, %ld), hs = [%ld, %ld)\n", */
	    /* 	   cur_m, ht_hvi.start, ht_hvi.end, hs_hvi.start, hs_hvi.end); */

	    assert(ht_hvi.start <= cur_m);
	    assert(cur_m < ht_hvi.end);
	}

	if(hs_hvi.end <= cur_m)
	{
	    Hsi_NEXT(&hs_hvi, &hsi);

	    /* printf("3: \ncur_m = %ld, ht = [%ld, %ld), hs = [%ld, %ld)\n", */
	    /* 	   cur_m, ht_hvi.start, ht_hvi.end, hs_hvi.start, hs_hvi.end); */

	    assert(hs_hvi.start <= cur_m);
	    assert(cur_m < hs_hvi.end);
	}
    }

    _Hs_Swap(hs, hs_dest);
    O_DECREF(hs_dest);

    return hs;
}


/* Now the utilizing functions. */

void _Hs_IntersectionFunction(hk_ptr hk_dest, chk_ptr hk1, chk_ptr hk2)
{
    if(Hk_EQUAL(hk1, hk2))
	Hk_COPY(hk_dest, hk1);
    else
	Hk_CLEAR(hk_dest);
}

HashSequence* Hs_HashTableIntersectionUpdate(HashSequence* hs, HashTable *ht)
{
    if(hs == NULL)
	return Hs_FromHashTable(ht);
    else
	return _Hs_Update(hs, ht, _Hs_IntersectionFunction);
}

MarkerInfo* Hs_NonZeroSet(HashSequence* hs)
{
    assert(hs != NULL);

    HashSequenceIterator *hsi = Hsi_New(hs);
    HashValidityItem hvi;
    MarkerInfo *mi = Mi_NEW(0,0);

    while(Hsi_NEXT(&hvi, hsi))
    {
	if(!Hk_ISZERO(&(hvi.hk)))
	    Mi_AppendValidRange(mi, hvi.start, hvi.end);
    }

    return mi;
}

HashTable* Hs_ToHashTable(HashSequence *hs)
{
    assert(hs != NULL);

    if(unlikely(hs->size == 0))
	return NewHashTable();

    HashTable *ht = NewSizeOptimizedHashTable(hs->size);

    HashSequenceIterator hsi;
    Hsi_INIT(hs, &hsi);

    HashValidityItem hvi;
    
    while(Hsi_NEXT(&hvi, &hsi))
    {
	if(!Hk_ISZERO(&(hvi.hk)))
	{
	    HashObject *h = Ht_ViewByKey(ht, hvi.hk);

	    if(h == NULL)
	    {
		HashObject *new_k = ALLOCATEHashObject();
		Hk_COPY(H_Hash_RW(new_k), &hvi.hk);
		H_GIVE_MARKER_INFO(new_k, Mi_NEW(hvi.start, hvi.end));
		Ht_Give(ht, new_k);
	    }
	    else
	    {
		Mi_AppendValidRange(H_Mi(h), hvi.start, hvi.end);
	    }
	}
    }

    return ht;
}

/********************************************************************************
 *
 *   Now the update and reduce functions (as these depend on the
 *   summarizing stuff above.
 *
 ********************************************************************************/
void _Hs_SummarizeFunction(hk_ptr hk_dest, chk_ptr hs_hk, chk_ptr ht_hk)
{
    HashKey hk;
    Hk_REHASH(&hk, ht_hk);
    Hk_REDUCE(hk_dest, hs_hk, &hk);
}

HashSequence *Ht_Summarize_Update(HashSequence *ht_accumulator, HashTable *ht)
{    
    if(unlikely(ht_accumulator == NULL))
    {
	HashSequence *hs = ConstructHashSequence();

	HashTableMarkerIterator htmi;
	Htmi_INIT(ht, &htmi);
	HashValidityItem hvi;

	while(Htmi_NEXT(&hvi, &htmi))
	{
	    /* printf("\n[%ld, %ld): ", hvi.start, hvi.end); */

	    HashKey hk = hvi.hk;
	    /* printf("htmi="); */
	    /* Hk_debug_PrintHash(&hk); */

	    Hk_INPLACE_REHASH(&hk);

	    /* printf("rehash="); */
	    /* Hk_debug_PrintHash(&hk); */

	    _Hs_Append(hs, hvi.start, &hk);
	}
	
	/* printf("\nFirst sumarize hs = \n"); */
	/* Hs_debug_print(hs); */

	return hs;
    }
    else
    {
	return _Hs_Update(ht_accumulator, ht, _Hs_SummarizeFunction);
    }
}

HashTable *Ht_Summarize_Finish(HashSequence *hs)
{
    assert(hs != NULL);

    if(unlikely(hs->size == 0))
	return NewHashTable();

    HashTable *ht = NewSizeOptimizedHashTable(hs->size);

    HashSequenceIterator hsi;
    Hsi_INIT(hs, &hsi);

    HashValidityItem hvi;
    
    /* printf("\nAdding in:"); */

    while(Hsi_NEXT(&hvi, &hsi))
    {
	if(!Hk_ISZERO(&(hvi.hk)))
	{
	    HashKey hk;
	    Hk_REHASH(&hk, &hvi.hk);
	    HashObject *_restrict_ h = Ht_ViewByKey(ht, hk);

	    if(h == NULL)
	    {
		/* printf("\nNew: [%ld, %ld): ", hvi.start, hvi.end); */
		/* Hk_debug_PrintHash(&hk); */

		HashObject * _restrict_ new_k = ALLOCATEHashObject();

		Hk_COPY(H_Hash_RW(new_k), &hk);
		H_GIVE_MARKER_INFO(new_k, Mi_NEW(hvi.start, hvi.end));
		/* printf("\n it's this: "); */
		/* H_debug_print(new_k); */
		Ht_Give(ht, new_k);

		// printf("\nhash table: \n");
		//Ht_debug_Print(ht);
	    }
	    else
	    {
		/* printf("\nUpd: [%ld, %ld): ", hvi.start, hvi.end); */
		/* Hk_debug_PrintHash(&hk); */

		Mi_AppendValidRange(H_Mi(h), hvi.start, hvi.end);

		// printf("\nhash table: \n");
		// Ht_debug_Print(ht);
	    }
	}
    }

    _Ht_debug_HashTableConsistent(ht);

    /* printf("Final hash table: \n"); */

    /* Ht_debug_Print(ht); */
    
    return ht;
}

HashTable *Ht_ReduceTable(HashTable *ht)
{
    HashTable *ht_dest = NewHashTable();
    HashTableMarkerIterator *htmi = Htmi_New(ht);
    HashValidityItem hvi;
    
    while(Htmi_NEXT(&hvi, htmi))
    {
	if(!Hk_ISZERO(&(hvi.hk)))
	{
	    HashObject *h = ALLOCATEHashObject();
	    Hf_COPY_FROM_KEY(h, &(hvi.hk));
	    _Ht_InsertValidNonOverlappingRange(ht_dest, h, hvi.start, hvi.end);
	}
    }

    return ht_dest;
}


/********************************************************************************
 *
 *  More testing things
 *
 ********************************************************************************/

HashSequence* Ht_EqualitySetUpdate(HashSequence *accumulator, HashTable *ht)
{
    return Hs_HashTableIntersectionUpdate(accumulator, ht);
}

MarkerInfo* Ht_EqualitySetFinish(HashSequence *accumulator)
{
    MarkerInfo *mi = Hs_NonZeroSet(accumulator);
    O_DECREF(accumulator);
    return mi;
}
    
MarkerInfo* Ht_EqualitySet(HashTable *ht1, HashTable *ht2)
{
    HashSequence* hs = Ht_EqualitySetUpdate(NULL, ht1);
    Ht_EqualitySetUpdate(hs, ht2);
    return Ht_EqualitySetFinish(hs);
}

