/***********************************************************************
 *
 *  Defines an efficient memory pool method for efficiently creating
 *  and destroying large numbers of various objects.
 *
 ***********************************************************************/

#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include "bitops.h"
#include <stdlib.h>


/* For convenience, define a way for structures using the memory pool
 * to automatically embed the proper information in structures being
 * used.  This macro must always be first in a structure's
 * definitions.
 */

#define MEMORY_POOL_ITEMS			\
    size_t _mpool_origin_index

typedef struct {
    void *memblock;
    bitfield use_mask;
    size_t search_start;
} _MP_Page;

typedef struct {
    size_t item_size;

    size_t num_pages;
    _MP_Page *pages;

    size_t page_search_start;		
    size_t candidate_index_for_collection;
} MemoryPool;

typedef struct {
    MEMORY_POOL_ITEMS;
}MemoryPoolItem;

#define STATIC_MEMORY_POOL_VALUES(item_size) {(item_size),0,NULL,0,0}

void  MP_Init(MemoryPool *mp, size_t item_size);
void  MP_Clear(MemoryPool *mp);

void* MP_Malloc(MemoryPool *mp, size_t *mpool_origin_index_dest);
void  MP_Free(MemoryPool *mp, void *obj, size_t mpool_origin_index);

/* TODO: Add in block allocation ability. */



/* These convenience macros simply wrap the above functions when the
 * item within it is a MemoryPoolItem struct.*/
static inline MemoryPoolItem* MP_ItemMalloc(MemoryPool *mp)
{
    size_t mpool_origin_index;
    MemoryPoolItem *mpi = MP_Malloc(mp, &mpool_origin_index);
    mpi->_mpool_origin_index = mpool_origin_index;
    return mpi;
}

#define MP_ItemFree(mp, item)					\
    do{								\
	assert((item) != NULL);					\
	MP_Free((mp), (item), (item)->_mpool_origin_index);	\
    }while(0)



#endif
