/********************************************************************************
 *
 *  Functions for implementing validity ranges for hash objects.
 *
 ********************************************************************************/

#include "markerinfo.h"
#include "errorhandling.h"
#include "debugging.h"
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

MemoryPool marker_info_pool = STATIC_MEMORY_POOL_VALUES(sizeof(MarkerInfo));

#define max(a,b) ( (a) < (b) ? (b) : (a) )
#define min(a,b) ( (a) < (b) ? (a) : (b) )

/* Create a new marker info class. */
MarkerInfo* Mi_New(markertype start, markertype end)
{
    WARNING(start > end, "Start of range is > end of range; zeroing range.");

    MarkerInfo *mi = ConstructMarkerInfo();

    if(likely(start < end) )
    {
	mi->r.start = start;
	mi->r.end   = end;
    }
    else
    {
	mi->r.start = 0;
	mi->r.end   = 0;
    }

    return mi;
}

/* Delete the marker info class. */
void Mi_Destroy(MarkerInfo* mi)
{
    if(mi->allocated_array_ranges != 0)
	free(mi->range_list);
}

DEFINE_OBJECT(
    MarkerInfo,
    Object,
    NULL,
    Mi_Destroy,					
    NULL);


/* Clear all the information out of the marker info class. */
void Mi_Clear(MarkerInfo* mi)
{
    mi->r.start = 0;
    mi->r.end = 0;
    mi->num_array_ranges = 0;
    mi->allocated_array_ranges = 0;

    if(mi->allocated_array_ranges != 0)
	free(mi->range_list);
}

static inline void _Mi_ConvertToArrayStyle(MarkerInfo *mi)
{
    assert(mi->num_array_ranges == 0);
    assert(mi->allocated_array_ranges == 0);
    assert(mi->range_list == NULL);

    /* Be conservative about allocating new ranges, as 99.9% of the
     * cases will involve 1 or 2 validity ranges. */

    mi->allocated_array_ranges = 2;
    mi->range_list = (MarkerRange*)malloc(sizeof(MarkerRange)*mi->allocated_array_ranges);
    
    mi->num_array_ranges = 1;
    mi->range_list[0].start = mi->r.start;
    mi->range_list[0].end = mi->r.end;
}

static inline size_t _Mi_Bisect(MarkerInfo *mi, markertype s)
{
    size_t low = 0, high = mi->num_array_ranges;

    size_t b;

#ifdef DEBUG_MODE
    int i;
    for(i = 0; i < mi->num_array_ranges; ++i)
	assert(mi->range_list[i].start <= mi->range_list[i].end);

    for(i = 0; i < mi->num_array_ranges-1; ++i)
	assert(mi->range_list[i].end < mi->range_list[i+1].start);
#endif
    
    assert(low < high);

    /* Optimize a bit for the case of a long sequential adding */
    if(unlikely(mi->range_list[high-1].end < s))
	return high - 1;

    while(1)
    {
	assert(low < high);

	if(unlikely(high == low + 1))
	    return low;

	b = low + (high - low) / 2;

	if(mi->range_list[b].start > s)
	    high = b;
	else
	    low = b;
    }
}

static inline void _Mi_Insert(MarkerInfo *mi, size_t idx, markertype start, markertype end)
{
    assert(idx <= mi->num_array_ranges);

    /* Allocate the space. */
    ++mi->num_array_ranges;
    
    if(mi->num_array_ranges > mi->allocated_array_ranges)
    {
	mi->allocated_array_ranges = 2*mi->num_array_ranges;
	assert(mi->range_list != NULL);
	mi->range_list = (MarkerRange*)realloc(mi->range_list, (sizeof(MarkerRange)*mi->allocated_array_ranges));
    }

    /* Copy over any ranges above the current index.*/

    size_t i;

    for(i = mi->num_array_ranges - 1; i > idx; --i)
    {
	mi->range_list[i].start = mi->range_list[i-1].start;
	mi->range_list[i].end = mi->range_list[i-1].end;
    }

    /* Insert the new values. */
    mi->range_list[idx].start = start;
    mi->range_list[idx].end = end;
}

static inline void _Mi_Clear(MarkerInfo *mi, size_t first_idx, size_t last_idx)
{
    if(first_idx > last_idx) return;

    size_t i;

    for(i = 0; last_idx + 1 + i < mi->num_array_ranges; ++i)
    {
	mi->range_list[first_idx+i].start = mi->range_list[last_idx+1+i].start;
	mi->range_list[first_idx+i].end = mi->range_list[last_idx+1+i].end;
    }

    mi->num_array_ranges -= (last_idx - first_idx) + 1;
}


/* Add a range in which the marker is valid. */
void Mi_AddValidRange(MarkerInfo* mi, markertype start, markertype end)
{

    if(unlikely(start > end))
    {
	WARNING(start > end, "Start of range is > end of range; no range added.");
	return;
    }

    /* See if we are in array-of-ranges mode or just the regular
     * markertype mode. */

    if(mi->num_array_ranges == 0)
    {
	assert(mi->r.start <= mi->r.end);
	
	/* If the original range is null, just set it to this and
	 * go.  Unlikely, though. */

	if(unlikely(mi->r.start >= mi->r.end) )
	{
	    mi->r.start = start;
	    mi->r.end   = end;
	    return;
	}
		
        /* If it extends the original r.start, r.end range, we're good to
	 * go. */
  
	if( !(start > mi->r.end || end < mi->r.start) )
	{
	    mi->r.start = min(start, mi->r.start);
	    mi->r.end = max(end, mi->r.end);
	    return;
	}
	else
	{
	    /* If not, now convert over to array-of-ranges and continue. */ 
	    _Mi_ConvertToArrayStyle(mi);
	    
	    assert(mi->num_array_ranges == 1);

	    if(end < mi->range_list[0].start)
	    {
		_Mi_Insert(mi, 0, start, end);
	    }
	    else
	    {
		ASSERTF(start > mi->range_list[0].start, Mi_debug_printMi(mi));
		_Mi_Insert(mi, 1, start, end);
	    }

	    return;
	}
    }    
    else
    {
	/* Now have to insert the new range; possibly extending or
	 * connecting ranges. */

	size_t idx = _Mi_Bisect(mi, start);
	
	/* Three factors control what we do: the insert idx, the final
	 * end idx of the range we collapse (prob usually the same),
	 * and whether we are extending the existing node there. */

	/* See if idx or idx + 1 is the insert idx. */
	size_t ins_idx, end_idx;
	bool extends_idx;

	if(mi->range_list[idx].end >= start)
	{
	    ins_idx = idx;

	    /* False if everything belongs beyond the first element. */
	    extends_idx = (mi->range_list[idx].start <= end) ? true : false;

	    if(unlikely(mi->range_list[idx].end >= end && mi->range_list[idx].start <= start)) 
	    {
		/* No changes needed; encapsulated in existing range. */
		return; 
	    }
	}
	else
	{
	    ins_idx = idx + 1;
	    extends_idx = (ins_idx < mi->num_array_ranges && mi->range_list[ins_idx].start <= end) ? true : false;
	}

	/* Set end_idx by seeing how many additional ranges we can
	 * "eat" with this one. */

	for(end_idx = ins_idx; 
	    end_idx + 1 < mi->num_array_ranges && mi->range_list[end_idx + 1].start <= end; 
	    ++end_idx);
	
	assert(end_idx == ins_idx || mi->num_array_ranges > end_idx);

	/* See if it actually does extend an existing one, just not at
	 * ins_idx.*/

	if(!extends_idx && end_idx != ins_idx)
	{
	    ++ins_idx;
	    extends_idx = true;
	}

	if(extends_idx)
	{
	    mi->range_list[ins_idx].start = min(start, mi->range_list[ins_idx].start);
	    mi->range_list[ins_idx].end = max(end, mi->range_list[end_idx].end);

	    /* Now delete any combined ranges if needed, and we're
	     * done. */

	    _Mi_Clear(mi, ins_idx + 1, end_idx);

	    return;
	}
	else
	{
	    _Mi_Insert(mi, ins_idx, start, end);
	    
	    return;
	}
    }
}

/* Remove a range in which the marker is valid. */
void Mi_RemoveValidRange(MarkerInfo* mi, markertype r_lower, markertype r_higher)
{
    /* This is pretty much the ultimate in slowness. */
}

/* Query whether the range is valid at a certain point. */
bool Mi_IsValid(MarkerInfo* mi, markertype m)
{
    if(mi->num_array_ranges == 0)
    {
	return (mi->r.start <= m && m < mi->r.end) ? 1 : 0;
    }
    else
    {
	size_t idx = _Mi_Bisect(mi, m);

	return (mi->range_list[idx].start <= m && m < mi->range_list[idx].end);
    }
}

bool Mi_IsEmpty(MarkerInfo* mi)
{
    return Mi_ISEMPTY(mi);
}

/*****************************************
 *
 *  Set operations
 *
 ****************************************/

/* Returns the complement of the two ranges. */
MarkerInfo* Mi_Complement(MarkerInfo* mi)
{
    
    MarkerInfo* ret_mi = Mi_New(0,0);

    MarkerIterator *mii = NewMarkerIterator(mi);

    MarkerRange *mr;

    //Mi_debug_printMi(mi);

    markertype last_r_end = MARKER_MINUS_INFTY;

    while( (mr = Mii_NEXT(mii)) != NULL)
    {
	Mi_AddValidRange(ret_mi, last_r_end, mr->start);
	
	last_r_end = mr->end;
    }

    Mi_AddValidRange(ret_mi, last_r_end, MARKER_PLUS_INFTY);

    //Mi_debug_printMi(ret_mi);

    return ret_mi;
}

/* Union of two ranges. */
MarkerInfo* Mi_Union(MarkerInfo* mi1, MarkerInfo* mi2)
{
    /* Stupid, simple, unoptimized at this point.  Just got to get
     * something done. */

    MarkerInfo* mi = Mi_New(0,0);
    MarkerRange* mr;

    MarkerIterator *mii1 = NewMarkerIterator(mi1);
    MarkerIterator *mii2 = NewMarkerIterator(mi2);

    while( (mr = Mii_NEXT(mii1)) != NULL)
	Mi_AddValidRange(mi, mr->start, mr->end);

    while( (mr = Mii_NEXT(mii2)) != NULL)
	Mi_AddValidRange(mi, mr->start, mr->end);

    Mii_Delete(mii1);
    Mii_Delete(mii2);
    
    return mi;
}

/* Intersection of two ranges. */
MarkerInfo* Mi_Intersection(MarkerInfo* mi1, MarkerInfo* mi2)
{
    MarkerInfo* mi1_c = Mi_Complement(mi1);
    MarkerInfo* mi2_c = Mi_Complement(mi2);
    
    MarkerInfo* mi1cu2c = Mi_Union(mi1_c, mi2_c);

    MarkerInfo* ret_mi = Mi_Complement(mi1cu2c);

    O_DECREF(mi1_c);
    O_DECREF(mi2_c);
    O_DECREF(mi1cu2c);

    return ret_mi;
}

/* All the elements in mi1 that aren't in mi2 */
MarkerInfo* Mi_Difference(MarkerInfo* mi1, MarkerInfo* mi2)
{
    MarkerInfo* mi2c = Mi_Complement(mi2);
    MarkerInfo* ret_mi = Mi_Intersection(mi1, mi2c);

    O_DECREF(mi2c);
    
    return ret_mi;
}

/* All the elements in exactly one set but not both. */
MarkerInfo* Mi_SymmetricDifference(MarkerInfo* mi1, MarkerInfo* mi2)
{
    MarkerInfo* mi1m2 = Mi_Difference(mi1, mi2);
    MarkerInfo* mi2m1 = Mi_Difference(mi2, mi1);

    MarkerInfo* ret_mi = Mi_Union(mi1m2, mi2m1);

    O_DECREF(mi1m2);
    O_DECREF(mi2m1);
    
    return ret_mi;
}

void Mi_debug_printMi(MarkerInfo *mi)
{
    if(mi == NULL)
    {
	printf("Mi: Empty\n");
    }
    else if(mi->num_array_ranges == 0)
    {
	if(mi->r.start != mi->r.end)
	    printf("Mi.P: [%ld,%ld)\n", mi->r.start, mi->r.end);
	else
	    printf("Mi: Empty\n");
    }
    else
    {
	int i;

	printf("Mi.M: ");

	for(i = 0; i < mi->num_array_ranges; ++i)
	{
	    printf("[%ld,%ld) ", mi->range_list[i].start, mi->range_list[i].end);
	}
	printf("\n");
    }

    fflush(stdout);
}

MemoryPool _markerinfo_iterator_pool = STATIC_MEMORY_POOL_VALUES(sizeof(MarkerIterator));
MemoryPool _markerinfo_rev_iterator_pool = STATIC_MEMORY_POOL_VALUES(sizeof(MarkerRevIterator));

MarkerIterator *NewMarkerIterator(MarkerInfo *mi)
{
    MarkerIterator *mii = (MarkerIterator*)MP_ItemMalloc(&_markerinfo_iterator_pool);
    mii->mi = mi;
    
    /* See if it's actually empty, and if so, set it up to terminate
     * right away. */
    if(likely(mi != NULL))
    {
	if(unlikely(mi->num_array_ranges == 0 && mi->r.start == mi->r.end))
	    mii->next_range_index = 1;
    }

    return mii;
}

/* Returns the next marker range in the sequence. */
MarkerRange* Mii_Next(MarkerIterator *mii)
{
    return Mii_NEXT(mii);
}

/* Cleans up the marker iterator instance. */
void Mii_Delete(MarkerIterator *mii)
{
    MP_ItemFree(&_markerinfo_iterator_pool, mii);
}

/************************************************************/
/* Allocates a new marker info iterator that travels the list backwards. */
MarkerRevIterator *NewReversedMarkerIterator(MarkerInfo *mi)
{
    MarkerRevIterator *miri = (MarkerRevIterator*)MP_ItemMalloc(&_markerinfo_rev_iterator_pool);

    if(unlikely(mi == NULL))
    {
	/* return empty iteration. */
	miri->previous_range_index = 0;
	return miri;
    }

    miri->mi = mi;

    if(mi->num_array_ranges == 0)
    {
	if(unlikely(mi->r.start == mi->r.end))
	    miri->previous_range_index = 0;
	else
	    miri->previous_range_index = 1;
    }
    else
	miri->previous_range_index = mi->num_array_ranges;

    return miri;
}

/* Returns the next marker range in the sequence. */
MarkerRange* Miri_Next(MarkerRevIterator *miri)
{
    return Miri_NEXT(miri);
}


/* Cleans up the marker iterator instance. */
void Miri_Delete(MarkerRevIterator *miri)
{
    MP_ItemFree(&_markerinfo_rev_iterator_pool, miri);
}

markertype Mr_Start(MarkerRange *mr)
{
    return mr->start;
}

markertype Mr_End(MarkerRange *mr)
{
    return mr->end;
}

markertype Mr_Plus_Infinity()
{
    return MARKER_PLUS_INFTY;
}

markertype Mr_Minus_Infinity()
{
    return MARKER_MINUS_INFTY;
}
