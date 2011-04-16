#include "memorypool.h"
#include "errorhandling.h"
#include "optimizations.h"
#include "debugging.h"
#include <stdlib.h>
#include <stddef.h>

#define AsBytePtr(ptr) ( (char*) ( (void*) (ptr) ) )
#define ExactOffsetFrom(ptr, offset) ((void*)(AsBytePtr(ptr) + offset) )
#define ExactPtrDiff(ptr1, ptr2) (AsBytePtr(ptr2) - AsBytePtr(ptr1))

#define __Max(x,y) ( (x) < (y) ? (y) : (x) )
#define __Min(x,y) ( (x) < (y) ? (x) : (y) )

#define __TargetLogPageSize 5  /* 32, determined by bitsize. */

#define __PageSizeAtLevel(size, level)			\
    __Min(1 << (level), 1 << __TargetLogPageSize)

/* Init function. */
void MP_Init(MemoryPool *mp, size_t item_size)
{
    mp->item_size = item_size;

    mp->num_pages = 4;

    mp->pages = (_MP_Page*)malloc(mp->num_pages*sizeof(_MP_Page));
    MEMCHECK(mp->pages);
    memset(mp->pages, 0, mp->num_pages*sizeof(_MP_Page));

    mp->page_search_start = 0;
    mp->candidate_index_for_collection = 0;
}

void MP_Clear(MemoryPool *mp)
{
    if(mp->pages != NULL)
    {
	size_t i;
	
	for(i = 0; i < mp->num_pages; ++i)
	{
	    _MP_Page *page = &mp->pages[i];
	    
	    if(page->memblock != NULL)
		free(page->memblock);
	}

	free(mp->pages);
    }

    mp->pages = NULL;
    mp->num_pages = 0;
    mp->page_search_start = 0;
    mp->candidate_index_for_collection = 0;
}

void* MP_Malloc(MemoryPool *mp, size_t *mpool_origin_index_dest)
{
    /* Possible for statically initialized memory pools. */
    if(unlikely(mp->num_pages == 0))
	MP_Init(mp, mp->item_size);

    size_t page_idx = mp->page_search_start;
    size_t block_idx = 0;

    while(1)
    {
	if(unlikely(page_idx == mp->num_pages))
	{
	    size_t old_size = mp->num_pages;
	    
	    mp->num_pages *= 2;
	    
	    mp->pages = (_MP_Page*)realloc(mp->pages, mp->num_pages*sizeof(_MP_Page));
	    MEMCHECK(mp->pages);
	    memset(&mp->pages[old_size], 0, (mp->num_pages-old_size)*sizeof(_MP_Page));

	    break;
	}
	
	if(!(firstnBitsOn(mp->pages[page_idx].use_mask,
			  __PageSizeAtLevel(mp->item_size, page_idx))))
	    break;
	
	++page_idx;
    }

    _MP_Page *page = &(mp->pages[page_idx]);
    size_t block_count = __PageSizeAtLevel(mp->item_size, page_idx);

    /* Now set the block index. */

    /* see if we need to allocate a new page. */
    if(unlikely(page->memblock == NULL))
    {
	assert(page->use_mask == 0);
     
	page->memblock = (void*)malloc(block_count*mp->item_size);
	MEMCHECK(page->memblock);
	page->use_mask = 0;
	block_idx = 0;
    }
    else
    {
	size_t i;

	assert(!firstnBitsOn(page->use_mask, block_count));
	assert(firstnBitsOn(page->use_mask, page->search_start));

	/* find the one that is free. */
	for(i = page->search_start; i < block_count; ++i)
	{
	    if(bitOff((page->use_mask), i))
	    {
		block_idx = i;
		break;
	    }
	}
	
	assert(i != block_count);
    }

    assert(block_idx < block_count);
    
    /* Record which one we've set. */
    setBitOn(page->use_mask, block_idx);
    
    /* Set the next search hints. */
    if(block_idx + 1 == block_count)
    {
	mp->page_search_start = page_idx + 1;
	page->search_start = block_count + 1;
    }
    else
    {
	mp->page_search_start = page_idx;
	page->search_start = block_idx + 1;
    }

    assert(&mp->pages[page_idx] == page);
    assert(page->memblock != NULL);

    /* Return the proper information. */
    *mpool_origin_index_dest = page_idx;

    void* ret_key = (void*)ExactOffsetFrom(page->memblock, mp->item_size * block_idx);
    memset(ret_key, 0, mp->item_size);

    return ret_key;
}


void MP_Free(MemoryPool *mp, void *obj, size_t page_loc)
{
    assert(page_loc < mp->num_pages);

    _MP_Page *page = &mp->pages[page_loc];

    assert(page->memblock != NULL);
    
    ptrdiff_t bytediff = AsBytePtr(obj) - AsBytePtr(page->memblock);
    
    assert(bytediff % mp->item_size == 0);

    ptrdiff_t block_index = bytediff / mp->item_size;

    assert(block_index < __PageSizeAtLevel(mp->item_size, page_loc));

    if(debug_false(bitOn(page->use_mask, block_index)))
    {
	DBGMSG("Memory Double Free!");
	abort();
    }
    
    setBitOff(page->use_mask, block_index);
    
    if(page->search_start > block_index)
	page->search_start = block_index;

    if(mp->page_search_start > page_loc)
	mp->page_search_start = page_loc;

    if(unlikely(page->use_mask == 0))
    {
	/* We free a page if there are two pages available to be
	 * freed; The one with the highest number is freed as this
	 * reduces the times it is reallocated as a new item is always
	 * taken from the first available spot.*/

	if(likely(mp->pages[mp->candidate_index_for_collection].use_mask == 0
		  && mp->pages[mp->candidate_index_for_collection].memblock != NULL))
	{
	    size_t free_idx = __Max(mp->candidate_index_for_collection, page_loc);
	    size_t new_cand_idx = __Min(mp->candidate_index_for_collection, page_loc);
	    
	    free(mp->pages[free_idx].memblock);
	    mp->pages[free_idx].memblock = NULL;
	    mp->candidate_index_for_collection = new_cand_idx;
	}
	else
	{
	    mp->candidate_index_for_collection = page_loc;
	}
    }
}
