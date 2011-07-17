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

#define STATIC_MEMORY_POOL_VALUES {0,0,NULL,0}

#define _DECLARE_MEMORY_POOL_BASE_DATASTRUCTS(Type)			\
									\
    typedef struct {							\
	Type *memblock;							\
	bitfield available_mask;					\
    } _MP_Page_##Type;							\
									\
    typedef struct {							\
	size_t allocated_pages;						\
	size_t first_page_with_free_spot;				\
	_MP_Page_##Type *pages;						\
	size_t candidate_page_for_deallocation;				\
    } MemoryPool_##Type;						

#define _DECLARE_MEMORY_POOL_BASE_FUNCTIONS(Type)			\
    									\
    static inline void _MP_MorePages_##Type()				\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
									\
	size_t old_size = mp->allocated_pages;				\
	mp->allocated_pages *= 2;					\
	mp->pages = (_MP_Page_##Type*)realloc(				\
	    mp->pages, mp->allocated_pages * sizeof(_MP_Page_##Type) );	\
	memset(mp->pages + old_size, 0,					\
	       (mp->allocated_pages - old_size )* sizeof(_MP_Page_##Type) ); \
    }									\
									\
    static inline void _MP_InitPage_##Type(_MP_Page_##Type *mpp)	\
    {									\
	assert(mpp->memblock == NULL);					\
	mpp->memblock = malloc(sizeof(Type) * bitsizeof(bitfield));	\
	memset(mpp->memblock, 0, sizeof(Type) * bitsizeof(bitfield));	\
	mpp->available_mask = ~((bitfield)0);				\
    }									\
									\
    static void _MP_Init_##Type()					\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
									\
	mp->allocated_pages = 32;					\
	mp->first_page_with_free_spot = 0;				\
	mp->pages = (_MP_Page_##Type*)malloc(mp->allocated_pages * sizeof(_MP_Page_##Type) ); \
	memset(mp->pages, 0, mp->allocated_pages * sizeof(_MP_Page_##Type) ); \
	_MP_InitPage_##Type(&(mp->pages[0]));				\
    }									\
									\
    static inline void _MP_ClearPage_##Type(_MP_Page_##Type *mpp)	\
    {									\
	assert(mpp->memblock != NULL);					\
	assert(mpp->available_mask == ~((bitfield)0));			\
	free(mpp->memblock);						\
	mpp->available_mask = 0;					\
	mpp->memblock = NULL;						\
    }									\
									\
    static void _MP_AdvancedPagePointer_##Type()			\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
									\
	assert(mp->first_page_with_free_spot < mp->allocated_pages);	\
									\
	while(true)							\
	{								\
	    assert(mp->pages[mp->first_page_with_free_spot].memblock != NULL); \
	    assert(mp->pages[mp->first_page_with_free_spot].available_mask == 0); \
									\
	    ++mp->first_page_with_free_spot;				\
									\
	    /* Need more pages? */					\
	    if(unlikely(mp->first_page_with_free_spot == mp->allocated_pages)) \
		_MP_MorePages_##Type();					\
									\
	    assert(mp->first_page_with_free_spot < mp->allocated_pages); \
									\
	    /* Need more allocation? */					\
	    if(mp->pages[mp->first_page_with_free_spot].memblock == NULL) \
	    {								\
		_MP_Page_##Type *mpp = &(mp->pages[mp->first_page_with_free_spot]); \
		_MP_InitPage_##Type(mpp);				\
		return;							\
	    }								\
									\
	    /* Simply the next one that has stuff available? */		\
	    if(mp->pages[mp->first_page_with_free_spot].available_mask != 0) \
		return;							\
	}								\
    }									\
    									\
    static Type* Mp_New##Type()						\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
									\
	if(unlikely(mp->pages == NULL))					\
	    _MP_Init_##Type(mp);					\
									\
	if(unlikely(! (mp->pages[mp->first_page_with_free_spot].available_mask) )) \
	    _MP_AdvancedPagePointer_##Type(mp);				\
									\
	_MP_Page_##Type *mpp = &(mp->pages[mp->first_page_with_free_spot]); \
									\
	assert(mpp->available_mask != 0);				\
	assert(mpp->memblock != NULL);					\
									\
	unsigned int idx = getFirstBitOn(mpp->available_mask);		\
									\
	assert(idx < bitsizeof(bitfield));				\
									\
	flipBitToOff(mpp->available_mask, idx);				\
	assert(bitOff(mpp->available_mask, idx));			\
									\
	Type* r = &(mpp->memblock[idx]);				\
	r->_mpool_origin_index = mp->first_page_with_free_spot;		\
	return r;							\
    }									\
									\
    static inline void Mp_Free##Type(void *v_obj)			\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
									\
	Type *obj = (Type*)(v_obj);					\
									\
	assert(obj->_mpool_origin_index < mp->allocated_pages);		\
									\
	size_t page_index = obj->_mpool_origin_index;			\
									\
	_MP_Page_##Type * mpp = &(mp->pages[page_index]);		\
									\
	size_t pos = (obj - mpp->memblock);				\
	assert(pos < bitsizeof(bitfield));				\
	assert(bitOff(mpp->available_mask, pos));			\
									\
	setBitOn(mpp->available_mask, pos);				\
									\
	mp->first_page_with_free_spot = min(mp->first_page_with_free_spot, page_index); \
									\
	memset(obj, 0, sizeof(Type));					\
									\
	if(unlikely( ~(mpp->available_mask) == 0) )			\
	{								\
	    size_t col_idx = mp->candidate_page_for_deallocation;	\
									\
	    if(col_idx != page_index && (~(mp->pages[col_idx].available_mask)) == 0) \
		_MP_ClearPage_##Type(&(mp->pages[col_idx]));		\
									\
	    mp->candidate_page_for_deallocation = page_index;		\
	}								\
    }

#define DECLARE_GLOBAL_MEMORY_POOL(Type)				\
    _DECLARE_MEMORY_POOL_BASE_DATASTRUCTS(Type)				\
    extern MemoryPool_##Type _memorypool_##Type;			\
    _DECLARE_MEMORY_POOL_BASE_FUNCTIONS(Type)				\
    void Mp_Free##Type##_NonStatic(Type *);				
    
#define DEFINE_GLOBAL_MEMORY_POOL(Type)					\
    MemoryPool_##Type _memorypool_##Type = STATIC_MEMORY_POOL_VALUES;	\
									\
    void Mp_Free##Type##_NonStatic(Type * obj)				\
    {									\
	Mp_Free##Type(obj);						\
    }

#define LOCAL_MEMORY_POOL(Type)						\
    _DECLARE_MEMORY_POOL_BASE_DATASTRUCTS(Type)				\
    static MemoryPool_##Type _memorypool_##Type = STATIC_MEMORY_POOL_VALUES; \
    _DECLARE_MEMORY_POOL_BASE_FUNCTIONS(Type)				\

#endif /* _MEMORYPOOL_H_ */
