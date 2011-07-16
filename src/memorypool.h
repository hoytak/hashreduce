#ifndef _MEMORYPOOL_H_
#define _MEMORYPOOL_H_

#include "bitops.h"
#include "optimizations.h"
#include "debugging.h"
#include "utilities.h"
#include <stdlib.h>
#include <string.h>

#define MEMORY_POOL_ITEMS			\
    size_t _mpool_origin_index

typedef struct {
    MEMORY_POOL_ITEMS;
} MemoryPoolItem;

typedef struct {
    void *memblock;
    bitfield available_mask;
} _MP_Page;

typedef struct {
    size_t item_size;
    size_t allocated_pages;
    size_t first_page_with_free_spot;
    _MP_Page *pages;
    size_t candidate_page_for_deallocation;
} MemoryPool;

#define STATIC_MEMORY_POOL_VALUES(item_size) {(item_size),0,0,NULL,0}

static void _MP_MorePages(MemoryPool *mp)
{
    size_t old_size = mp->allocated_pages;
    mp->allocated_pages *= 2;
    mp->pages = (_MP_Page*)realloc(mp->pages, mp->allocated_pages * sizeof(_MP_Page) );
    memset(mp->pages + old_size, 0, (mp->allocated_pages - old_size )* sizeof(_MP_Page) );
}

static void _MP_InitPage(MemoryPool *mp, _MP_Page *mpp)
{
    assert(mpp->memblock == NULL);
    mpp->memblock = malloc(mp->item_size * bitsizeof(bitfield));
    memset(mpp->memblock, 0, mp->item_size * bitsizeof(bitfield));
    mpp->available_mask = ~((bitfield)0);
}

static void _MP_Init(MemoryPool *mp)
{
    mp->allocated_pages = 16;
    mp->first_page_with_free_spot = 0;
    mp->pages = (_MP_Page*)malloc(mp->allocated_pages * sizeof(_MP_Page) );
    memset(mp->pages, 0, mp->allocated_pages * sizeof(_MP_Page) );
    _MP_InitPage(mp, &(mp->pages[0]));
}

static void _MP_ClearPage(_MP_Page *mpp)
{
    assert(mpp->memblock != NULL);
    assert(mpp->available_mask == ~((bitfield)0));
    free(mpp->memblock);
    mpp->available_mask = 0;
    mpp->memblock = NULL;
}

static inline void MP_Clear(MemoryPool *mp)
{
    size_t i;
    for(i = 0; i < mp->allocated_pages; ++i)
	if(mp->pages[i].memblock != NULL)
	    free(mp->pages[i].memblock);
    
    free(mp->pages);
}

static void _MP_AdvancedPagePointer(MemoryPool *mp)
{
    assert(mp->first_page_with_free_spot < mp->allocated_pages);

    while(true)
    {
	assert(mp->pages[mp->first_page_with_free_spot].memblock != NULL);
	assert(mp->pages[mp->first_page_with_free_spot].available_mask == 0);
	
	++mp->first_page_with_free_spot;

	/* Need more pages? */
	if(mp->first_page_with_free_spot == mp->allocated_pages)
	    _MP_MorePages(mp);

	assert(mp->first_page_with_free_spot < mp->allocated_pages);

	/* Need more allocation? */
	if(mp->pages[mp->first_page_with_free_spot].memblock == NULL) 
	{
	    _MP_Page *mpp = &(mp->pages[mp->first_page_with_free_spot]);
	    _MP_InitPage(mp, mpp);
	    return;
	}

	/* Simply the next one that has stuff available? */
	if(mp->pages[mp->first_page_with_free_spot].available_mask != 0)
	    return;
    }
}

static MemoryPoolItem* MP_ItemMalloc(MemoryPool *mp)
{
    if(unlikely(mp->pages == NULL))
	_MP_Init(mp);

    if(unlikely(! (mp->pages[mp->first_page_with_free_spot].available_mask) ))
	_MP_AdvancedPagePointer(mp);

    _MP_Page *mpp = &(mp->pages[mp->first_page_with_free_spot]);

    assert(mpp->available_mask != 0);
    assert(mpp->memblock != NULL);

    unsigned int idx = getFirstBitOn(mpp->available_mask);

    // fprintf(stderr, "bf = %llx, idx = %d\n", mpp->available_mask, idx);

    assert(idx < bitsizeof(bitfield));

    flipBitToOff(mpp->available_mask, idx);
    assert(bitOff(mpp->available_mask, idx));

    MemoryPoolItem* r = (MemoryPoolItem*)( &((char*)mpp->memblock)[mp->item_size * idx]);
    r->_mpool_origin_index = mp->first_page_with_free_spot;
    return r;
}

static void  MP_ItemFree(MemoryPool *mp, void *v_obj)
{
    MemoryPoolItem *obj = (MemoryPoolItem*)(v_obj);

    assert(obj->_mpool_origin_index < mp->allocated_pages);
    
    size_t page_index = obj->_mpool_origin_index;

    _MP_Page * mpp = &(mp->pages[page_index]);

    // Could be faster if we knew the type
    size_t pos = ((char*)obj - (char*)(mpp->memblock)) / mp->item_size;
    assert(pos < bitsizeof(bitfield));
    assert(bitOff(mpp->available_mask, pos));

    setBitOn(mpp->available_mask, pos);

    mp->first_page_with_free_spot = min(mp->first_page_with_free_spot, page_index);

    // Strict policy; cleared on return
    memset(obj, 0, mp->item_size);

    if(unlikely( ~(mpp->available_mask) == 0) )
    {
	size_t col_idx = mp->candidate_page_for_deallocation;

	if(col_idx != page_index && (~(mp->pages[col_idx].available_mask)) == 0)
	    _MP_ClearPage(&(mp->pages[col_idx]));

	mp->candidate_page_for_deallocation = page_index;
    }
}

#define DEFINE_GLOBAL_MEMORY_POOL(Type)			\
    MemoryPool _memorypool_##Type =			\
	STATIC_MEMORY_POOL_VALUES(sizeof(Type));	\
    
#define DECLARE_GLOBAL_MEMORY_POOL(Type)				\
									\
    extern MemoryPool _memorypool_##Type;				\
									\
    static inline Type* Mp_New##Type()					\
    {									\
	return (Type*)MP_ItemMalloc(&_memorypool_##Type);		\
    }									\
									\
    static inline void Mp_Free##Type(Type* item)			\
    {									\
	MP_ItemFree(&_memorypool_##Type, (MemoryPoolItem*)item);	\
    }									\

#define LOCAL_MEMORY_POOL(Type)						\
    static MemoryPool _memorypool_##Type =				\
	STATIC_MEMORY_POOL_VALUES(sizeof(Type));			\
									\
    static inline Type* Mp_New##Type()					\
    {									\
	return (Type*)MP_ItemMalloc(&_memorypool_##Type);		\
    }									\
									\
    static inline void Mp_Free##Type(Type* item)			\
    {									\
	MP_ItemFree(&_memorypool_##Type, (MemoryPoolItem*)item);	\
    }									\

#endif /* _MEMORYPOOL_H_ */
