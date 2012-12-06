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

#define STATIC_MEMORY_POOL_VALUES {0,0,NULL,NULL}

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
	Type *reserve_memblock;						\
    } MemoryPool_##Type;						

#define _MP_MEM_POOL_STEP 8

static inline bool _MP_IndexMasterPage(size_t idx)	
{
    return (idx <= _MP_MEM_POOL_STEP) ? true : ((idx & (_MP_MEM_POOL_STEP-1)) == 0);
}

static inline size_t _MP_IndexMasterPage_Size(size_t idx)
{
    assert(_MP_IndexMasterPage(idx));
    return (idx < _MP_MEM_POOL_STEP) ? 1 : _MP_MEM_POOL_STEP;
}

#define _DECLARE_MEMORY_POOL_BASE_FUNCTIONS(Type)			\
									\
    static void _MP_InitPages_##Type(size_t idx)			\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
	_MP_Page_##Type *mpp = &(mp->pages[idx]);			\
	size_t num = _MP_IndexMasterPage_Size(idx);			\
	Type* ptr;							\
	if(mp->reserve_memblock != NULL && idx > _MP_MEM_POOL_STEP)	\
	{								\
	    ptr = mp->reserve_memblock;					\
	    mp->reserve_memblock = NULL;				\
	}								\
	else								\
	{								\
	    ptr = (Type*)calloc(bitsizeof(bitfield)*num, sizeof(Type)); \
	    CHECK_MALLOC(ptr);						\
	}								\
									\
	int i;								\
	for(i = 0; i < num; ++i)					\
	{								\
	    assert( (mpp+i)->memblock == NULL);				\
	    (mpp + i)->memblock = ptr + i*bitsizeof(bitfield);		\
	    (mpp + i)->available_mask = ~((bitfield)0);			\
	}								\
    }									\
									\
    static void _MP_MorePages_##Type()					\
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
    static void _MP_Init_##Type()					\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
									\
	mp->allocated_pages = 4*_MP_MEM_POOL_STEP;			\
	mp->first_page_with_free_spot = 0;				\
	mp->pages = (_MP_Page_##Type*)calloc(mp->allocated_pages, sizeof(_MP_Page_##Type) ); \
	_MP_InitPages_##Type(0);					\
    }									\
									\
    static inline Type* _MP_ClearPages_##Type(size_t idx)		\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
									\
	_MP_Page_##Type *mpp = &(mp->pages[idx]);			\
	size_t num = _MP_IndexMasterPage_Size(idx);			\
	Type* memblock = mpp->memblock;					\
	assert(mpp->memblock != NULL);					\
	assert(mpp->available_mask == ~((bitfield)0));			\
	size_t i, pos;							\
	for(i = 0; i < num; ++i)					\
	{								\
	    if(true)							\
	    {								\
		for(pos = 0; pos < bitsizeof(bitfield); ++pos)		\
		    assert(bitOn((mpp+i)->available_mask, pos));	\
	    }								\
									\
	    (mpp + i)->available_mask = 0;				\
	    (mpp + i)->memblock = NULL;					\
	}								\
	return memblock;						\
    }									\
									\
    static void _MP_AdvancePagePointer_##Type()				\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
									\
	assert(mp->first_page_with_free_spot < mp->allocated_pages);	\
	assert(isPowerOf2(mp->allocated_pages));			\
									\
	do{								\
	    assert(mp->pages[mp->first_page_with_free_spot].available_mask == 0); \
									\
	    if(mp->pages[mp->first_page_with_free_spot].memblock != NULL) \
		++mp->first_page_with_free_spot;			\
	    else							\
		assert(_MP_IndexMasterPage(mp->first_page_with_free_spot)); \
									\
	    if(_MP_IndexMasterPage(mp->first_page_with_free_spot))	\
	    {								\
		/* Need more pages? */					\
		if(unlikely(mp->first_page_with_free_spot == mp->allocated_pages)) \
		{							\
		    _MP_MorePages_##Type();				\
		    _MP_InitPages_##Type(mp->first_page_with_free_spot); \
		    return;						\
		}							\
									\
		assert(mp->first_page_with_free_spot < mp->allocated_pages); \
									\
		/* Need more allocation? */				\
		if(likely(mp->pages[mp->first_page_with_free_spot].memblock == NULL)) \
		{							\
		    _MP_InitPages_##Type(mp->first_page_with_free_spot); \
		    return;						\
		}							\
	    }								\
									\
	    assert(mp->pages[mp->first_page_with_free_spot].memblock != NULL); \
									\
	}while(unlikely(likely(mp->pages[mp->first_page_with_free_spot].available_mask == 0))); \
    }									\
    									\
    static Type*  Mp_New##Type()					\
    {									\
	MemoryPool_##Type *mp = &_memorypool_##Type;			\
									\
	if(unlikely(mp->pages == NULL))					\
	    _MP_Init_##Type(mp);					\
									\
	if(unlikely(! (mp->pages[mp->first_page_with_free_spot].available_mask) )) \
	    _MP_AdvancePagePointer_##Type(mp);				\
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
	if(unlikely(_MP_IndexMasterPage(page_index) && (~(mpp->available_mask)) == 0) ) \
	{								\
	    bool okay = true;						\
	    int i;							\
	    int num = _MP_IndexMasterPage_Size(page_index);		\
									\
	    for(i = 1; i < num; ++i)					\
	    {								\
		if( (~((mpp + i)->available_mask)) != 0)		\
		{okay = false; break; }					\
	    }								\
	    if(okay)							\
	    {								\
		Type* memblock = _MP_ClearPages_##Type(page_index);	\
		if(page_index < _MP_MEM_POOL_STEP || mp->reserve_memblock != NULL) \
		{							\
		    free(memblock);					\
		}							\
		else							\
		    mp->reserve_memblock = memblock;			\
	    }								\
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
