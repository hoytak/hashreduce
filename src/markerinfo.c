/********************************************************************************
 *
 *  Functions for implementing validity ranges for hash objects.
 *
 ********************************************************************************/

#include "markerinfo.h"
#include "errorhandling.h"
#include "debugging.h"
#include "utilities.h"
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

static bool marker_range_warnings_enabled = true;

void Mi_DisableWarnings()
{
    marker_range_warnings_enabled = false;
}

/* Create a new marker info class. */
mi_ptr Mi_New(markertype start, markertype end)
{
    WARNING(start > end && marker_range_warnings_enabled, 
	    "Start of range is > end of range; zeroing range.  Use Mi_DisableWarnings() to disable.");

    return Mi_NEW(start, end);
}

/* Create a new marker info class. */
mi_ptr Mi_NewInvalid()
{
    return Mi_NEW_INVALID();
}

/* Delete the marker info class. */
void Mi_Destroy(mi_ptr mi)
{
    if(mi->allocated_array_ranges != 0)
	free(mi->range_list);
}

DEFINE_OBJECT(
    MarkerInfo,
    Object,
    NULL,
    Mi_Destroy);


/* Clear all the information out of the marker info class. */
void Mi_Clear(mi_ptr mi)
{
    assert(!Mi_IsDebugLocked(mi));

    mi->r.start = 0;
    mi->r.end = 0;
    mi->num_array_ranges = 0;
    mi->allocated_array_ranges = 0;

    if(mi->allocated_array_ranges != 0)
	free(mi->range_list);
}

bool Mi_ValidEverywhere(const MarkerInfo* mi)
{
    return Mi_VALID_EVERYWHERE(mi);
}

bool Mi_ValidAnywhere(const MarkerInfo* mi)
{
    return Mi_VALID_ANYWHERE(mi);
}

static inline void _Mi_ConvertToArrayStyle(mi_ptr mi)
{
    assert(mi->num_array_ranges == 0);
    
    if(likely(mi->allocated_array_ranges == 0))
    {

	assert(mi->range_list == NULL);

	/* Be conservative about allocating new ranges, as 99.9% of the
	 * cases will involve 1 or 2 validity ranges. */

	mi->allocated_array_ranges = 4;
	mi->range_list = (mr_ptr)malloc(sizeof(MarkerRange)*mi->allocated_array_ranges);
    }

    mi->num_array_ranges = 1;
    mi->range_list[0].start = mi->r.start;
    mi->range_list[0].end = mi->r.end;
}


static inline void Mi_CheckRestructure(mi_ptr mi)
{
    if(unlikely(mi->num_array_ranges == 1))
    {
	mi->r = mi->range_list[0];
	mi->num_array_ranges = 0;
    }
}

static inline size_t _Mi_Bisect(cmi_ptr mi, markertype s)
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

static void _Mi_Insert(mi_ptr mi, size_t idx, markertype start, markertype end)
{
    assert(idx <= mi->num_array_ranges);

    /* Allocate the space. */
    ++mi->num_array_ranges;
    
    if(mi->num_array_ranges > mi->allocated_array_ranges)
    {
	mi->allocated_array_ranges = 2*mi->num_array_ranges;
	assert(mi->range_list != NULL);
	mi->range_list = (mr_ptr)realloc(mi->range_list, (sizeof(MarkerRange)*mi->allocated_array_ranges));
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

    Mi_CheckRestructure(mi);
}

static void _Mi_InsertAtEnd(mi_ptr mi, markertype start, markertype end)
{
    /* Allocate the space. */
    size_t idx = mi->num_array_ranges;

    assert(idx >= 1);

    if(unlikely(start <= mi->range_list[idx-1].end))
    {    
	--idx;
	if(unlikely(mi->range_list[idx].start > start))
	{
	    while(idx >= 1 && mi->range_list[idx-1].end >= start)
		--idx;
	 
	    /* Now we may have to go back a bit, swallowing the end. */
	    mi->range_list[idx].start = min(mi->range_list[idx].start, start);
	    mi->num_array_ranges = idx+1;
	}

	mi->range_list[idx].end = end;
	Mi_CheckRestructure(mi);
    }
    else
    {
	++mi->num_array_ranges;
    
	if(mi->num_array_ranges > mi->allocated_array_ranges)
	{
	    mi->allocated_array_ranges = 2*mi->num_array_ranges;
	    assert(mi->range_list != NULL);
	    mi->range_list = (mr_ptr)realloc(mi->range_list, (sizeof(MarkerRange)*mi->allocated_array_ranges));
	}

	/* Insert the new values. */
	mi->range_list[idx].start = start;
	mi->range_list[idx].end = end;
    }

    Mi_CheckRestructure(mi);
}

void Mi_AppendValidRange(mi_ptr mi, markertype r_lower, markertype r_higher)
{
    assert(!Mi_IsDebugLocked(mi));

    if(unlikely(r_lower >= r_higher))
	return;

    if(unlikely(mi->num_array_ranges == 0))
    {
	if(mi->r.start == mi->r.end)
	{
	    mi->r.start = r_lower;
	    mi->r.end = r_higher;
	    return;
	}
	else if(unlikely(r_lower <= mi->r.end))
	{
	    mi->r.start = min(r_lower, mi->r.start);
	    mi->r.end = r_higher;
	    return;
	}
	else
	{
	    _Mi_ConvertToArrayStyle(mi);
	}
    }

    _Mi_InsertAtEnd(mi, r_lower, r_higher);
}


static inline void _Mi_Clear(mi_ptr mi, size_t first_idx, size_t last_idx)
{
    if(first_idx > last_idx) return;

    size_t i;

    for(i = 0; last_idx + 1 + i < mi->num_array_ranges; ++i)
    {
	mi->range_list[first_idx+i].start = mi->range_list[last_idx+1+i].start;
	mi->range_list[first_idx+i].end = mi->range_list[last_idx+1+i].end;
    }

    mi->num_array_ranges -= (last_idx - first_idx) + 1;

    Mi_CheckRestructure(mi);
}


/* Add a range in which the marker is valid. */
void Mi_AddValidRange(mi_ptr mi, markertype start, markertype end)
{
    assert(!Mi_IsDebugLocked(mi));

    if(unlikely(start > end && marker_range_warnings_enabled))
    {
	fprintf(stderr, "WARNING: Start of range (%ld) is > end of range (%ld); no range added.", start, end);
	return;
    }

    if(unlikely(mi == NULL))
	return;

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
		_Mi_InsertAtEnd(mi, start, end);
	    }

	    return;
	}
    }    
    else if(mi->range_list[mi->num_array_ranges - 1].end <= end) 
    {
	_Mi_InsertAtEnd(mi, start, end);
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

void Mi_Swap(mi_ptr mi1, mi_ptr mi2)
{
    assert(!Mi_IsDebugLocked(mi1));
    assert(!Mi_IsDebugLocked(mi2));

    assert(mi1 != NULL);
    assert(mi2 != NULL);

    MarkerRange r				= mi1->r;
    unsigned long num_array_ranges		= mi1->num_array_ranges;
    unsigned long allocated_array_ranges	= mi1->allocated_array_ranges;
    mr_ptr range_list          			= mi1->range_list;

    mi1->r					= mi2->r;
    mi1->num_array_ranges			= mi2->num_array_ranges;
    mi1->allocated_array_ranges			= mi2->allocated_array_ranges;
    mi1->range_list				= mi2->range_list;

    mi2->r					= r;
    mi2->num_array_ranges			= num_array_ranges;
    mi2->allocated_array_ranges			= allocated_array_ranges;
    mi2->range_list				= range_list;
}

void Mi_RemoveValidRange(mi_ptr mi, markertype start, markertype end)
{
    /* Could be optimized a bunch more. */
    assert(!Mi_IsDebugLocked(mi));

    if(unlikely(start >= end))
	return;

    mi_ptr m = Mi_NEW(MARKER_MINUS_INFTY, start);
    Mi_AppendValidRange(m, end, MARKER_PLUS_INFTY);

    mi_ptr m2 = Mi_Intersection(mi, m);
    
    Mi_Swap(mi, m2);

    O_DECREF(m);
    O_DECREF(m2);
}

/* Query whether the range is valid at a certain point. */
bool Mi_IsValid(cmi_ptr mi, markertype m)
{
    if(unlikely(mi == NULL))
	return (m != MARKER_PLUS_INFTY);

    if(mi->num_array_ranges == 0)
    {
	return (mi->r.start <= m && m < mi->r.end);
    }
    else
    {
	size_t idx = _Mi_Bisect(mi, m);

	return (mi->range_list[idx].start <= m && m < mi->range_list[idx].end);
    }
}

bool Mi_IsEmpty(cmi_ptr mi)
{
    return Mi_ISEMPTY(mi);
}

/*****************************************
 *
 *  Set operations
 *
 ****************************************/

/* Returns the complement of the two ranges. */
mi_ptr Mi_Complement(cmi_ptr mi)
{
    if(unlikely(mi == NULL))
	return Mi_NEW(0,0);

    mi_ptr ret_mi = Mi_NEW(0,0);

    MarkerIterator *mii = Mii_New(mi);
    MarkerRange mr;

    markertype last_r_end = MARKER_MINUS_INFTY;

    while(Mii_NEXT(&mr, mii))
    {
	Mi_AppendValidRange(ret_mi, last_r_end, mr.start);
	last_r_end = mr.end;
    }

    Mi_AppendValidRange(ret_mi, last_r_end, MARKER_PLUS_INFTY);

    return ret_mi;
}

bool Mi_Equal(const MarkerInfo *mi1, const MarkerInfo* mi2)
{
    if(mi1 == NULL)
	return Mi_ValidEverywhere(mi2);
    else if(mi2 == NULL)
	return Mi_ValidEverywhere(mi1);

    if(mi1->num_array_ranges == 0 || mi2->num_array_ranges == 0)
    {
	if(mi2->num_array_ranges < mi1->num_array_ranges)
	{
	    const MarkerInfo *mi_t = mi1; mi1 = mi2; mi2 = mi_t;
	}

	switch(mi2->num_array_ranges)
	{
	case 0:
	    return (mi1->r.start == mi2->r.start) && (mi1->r.end == mi2->r.end);
	case 1:
	    return ((mi1->r.start == mi2->range_list[0].start) 
		    && (mi1->r.end == mi2->range_list[0].end));
	default:
	    return false;
	}
    }
    else
    {
	if(mi1->num_array_ranges != mi2->num_array_ranges)
	    return false;

	size_t i;
	for(i = 0; i < mi1->num_array_ranges; ++i)
	{
	    if(! ((mi1->range_list[i].start == mi2->range_list[i].start) 
		  && (mi1->range_list[i].end == mi2->range_list[i].end) ) )
		return false;
	}

	return true;
    }
}

mi_ptr Mi_Copy(cmi_ptr mi)
{
    if(unlikely(mi == NULL))
	return Mi_NEW(MARKER_MINUS_INFTY, MARKER_PLUS_INFTY);

    mi_ptr mic = Mi_NEW(0,0);

    mic->r = mi->r;
    
    if(mi->num_array_ranges != 0)
    {
	mic->num_array_ranges = mi->num_array_ranges;
	mic->range_list = (mr_ptr)malloc(sizeof(MarkerRange)*mi->num_array_ranges);
	memcpy(mic->range_list, mi->range_list, sizeof(MarkerRange)*mi->num_array_ranges);
	mic->allocated_array_ranges = mi->num_array_ranges;
    }

    return mic;
}

/* Intersection of two ranges. */
mi_ptr Mi_Union(cmi_ptr mi1, cmi_ptr mi2)
{
    if(unlikely(mi1 == NULL || mi2 == NULL) )
	return NULL;

    MarkerRange mr1;
    MarkerIterator *mii1 = Mii_New(mi1);
    bool mr1_okay = Mii_NEXT(&mr1, mii1);

    if(unlikely(!mr1_okay))
    {
	Mii_Delete(mii1);
	return Mi_Copy(mi2);
    }
    
    MarkerRange mr2;
    MarkerIterator *mii2 = Mii_New(mi2);
    bool mr2_okay = Mii_NEXT(&mr2, mii2);

    if(unlikely(!mr2_okay))
    {
	Mii_Delete(mii2);
	return Mi_Copy(mi1);
    }

    mi_ptr mi = Mi_NEW(0,0);

    while(true)
    {
	markertype start, end;

	if(mr1.start <= mr2.start)
	{
	    start = mr1.start;
	    end = (mr2.start <= mr1.end) ? max(mr1.end, mr2.end) : mr1.end;
	}
	else
	{
	    start = mr2.start;
	    end = (mr1.start <= mr2.end) ? max(mr1.end, mr2.end) : mr2.end;
	}
	
	Mi_AppendValidRange(mi, start, end);

	while(mr1.end <= end)
	{
	    if(unlikely(!Mii_NEXT(&mr1, mii1)))
	    {
		do{
		    if(mr2.end > end)
			Mi_AppendValidRange(mi, mr2.start, mr2.end);
		} while(Mii_NEXT(&mr2, mii2));

		goto MR_UNION_DONE;
	    }
	}

	while(mr2.end <= end)
	{
	    if(unlikely(!Mii_NEXT(&mr2, mii2)))
	    {
		do{
		    if(mr1.end > end)
			Mi_AppendValidRange(mi, mr1.start, mr1.end);
		} while(Mii_NEXT(&mr1, mii1));

		goto MR_UNION_DONE;
	    }
	}
    }

MR_UNION_DONE:;

    Mii_Delete(mii1);
    Mii_Delete(mii2);
 
    return mi;
}

mi_ptr Mi_UnionUpdate(mi_ptr mi1, cmi_ptr mi2)
{
    assert(!Mi_IsDebugLocked(mi1));

    if(unlikely(mi1 == NULL))
	return Mi_Copy(mi2);

    mi_ptr mi = Mi_Union(mi1, mi2);
    Mi_Swap(mi, mi1);
    O_DECREF(mi);
    return mi1;
}

mi_ptr Mi_IntersectionUpdate(mi_ptr mi1, cmi_ptr mi2)
{
    assert(!Mi_IsDebugLocked(mi1));

    if(unlikely(mi1 == NULL))
	return Mi_Copy(mi2);

    mi_ptr mi = Mi_Intersection(mi1, mi2);
    Mi_Swap(mi, mi1);
    O_DECREF(mi);
    return mi1;
}

/* Intersection of two ranges. */
mi_ptr Mi_Intersection(cmi_ptr mi1, cmi_ptr mi2)
{
    if(unlikely(mi1 == NULL))
	return Mi_Copy(mi2);
    else if(unlikely(mi2 == NULL))
	return Mi_Copy(mi1);

    mi_ptr mi = Mi_NEW(0,0);

    MarkerIterator *mii1 = Mii_New(mi1);
    MarkerIterator *mii2 = Mii_New(mi2);

    MarkerRange mr1 = {0,0}, mr2 = {0,0};
    
    bool mr1_okay = Mii_NEXT(&mr1, mii1);
    bool mr2_okay = Mii_NEXT(&mr2, mii2);

    if(likely(mr1_okay && mr2_okay))
    {
	while(true)
	{
	    if(mr1.start <= mr2.end && mr2.start <= mr1.end)
		Mi_AppendValidRange(mi, max(mr1.start, mr2.start), min(mr1.end, mr2.end));

	    if(mr1.end < mr2.end)
	    {
		mr1_okay = Mii_NEXT(&mr1, mii1);
		if(unlikely(!mr1_okay))
		    break;
	    }
	    else
	    {
		mr2_okay = Mii_NEXT(&mr2, mii2);
		if(unlikely(!mr2_okay))
		    break;
	    }
	}
    }

    Mii_Delete(mii1);
    Mii_Delete(mii2);
 
    return mi;
}

/* All the elements in mi1 that aren't in mi2 */
mi_ptr Mi_Difference(cmi_ptr mi1, cmi_ptr mi2)
{
    if(unlikely(mi1 == NULL))
	return Mi_Complement(mi2);
    else if(unlikely(mi2 == NULL))
	return Mi_NEW(0,0);

    mi_ptr mi2c = Mi_Complement(mi2);
    mi_ptr ret_mi = Mi_Intersection(mi1, mi2c);

    O_DECREF(mi2c);
    
    return ret_mi;
}

/* All the elements in exactly one set but not both. */
mi_ptr Mi_SymmetricDifference(cmi_ptr mi1, cmi_ptr mi2)
{
    mi_ptr mi1m2 = Mi_Difference(mi1, mi2);
    mi_ptr mi2m1 = Mi_Difference(mi2, mi1);

    mi_ptr ret_mi = Mi_Union(mi1m2, mi2m1);

    O_DECREF(mi1m2);
    O_DECREF(mi2m1);
    
    return ret_mi;
}

LOCAL_MEMORY_POOL(MarkerIterator);
LOCAL_MEMORY_POOL(MarkerRevIterator);

static const MarkerRange _mr_all_valid = {MARKER_MINUS_INFTY, MARKER_PLUS_INFTY};

MarkerIterator *Mii_New(cmi_ptr mi)
{
    MarkerIterator *mii = Mp_NewMarkerIterator();
    
    /* See if it's actually empty, and if so, set it up to terminate
     * after . */
    if(likely(mi != NULL))
    {
	if(mi->num_array_ranges == 0)
	{
	    if(mi->r.start == mi->r.end)
	    {
		assert(mi->r.start == 0);
		mii->counts_left = 0;
	    }
	    else
	    {
		mii->counts_left = 1;
		mii->next_mr = &(mi->r);
	    }
	}
	else
	{
	    assert(mi->range_list != NULL);
	    mii->counts_left = mi->num_array_ranges;
	    mii->next_mr = mi->range_list;
	}
    }
    else
    {
	mii->next_mr = &_mr_all_valid;
	mii->counts_left = 1;
    }

    return mii;
}

/* Returns the next marker range in the sequence. */
bool Mii_Next(MarkerRange *mr, MarkerIterator *mii)
{
    return Mii_NEXT(mr, mii);
}

/* Cleans up the marker iterator instance. */
void Mii_Delete(MarkerIterator *mii)
{
    Mp_FreeMarkerIterator(mii);
}

/************************************************************/
/* Allocates a new marker info iterator that travels the list backwards. */
MarkerRevIterator *Miri_New(cmi_ptr mi)
{
    MarkerRevIterator *miri = Mp_NewMarkerRevIterator();
    
    /* See if it's actually empty, and if so, set it up to terminate
     * after . */
    if(likely(mi != NULL))
    {
	if(mi->num_array_ranges == 0)
	{
	    if(mi->r.start == mi->r.end)
	    {
		assert(mi->r.start == 0);
		miri->counts_left = 0;
	    }
	    else
	    {
		miri->counts_left = 1;
		miri->next_mr = &(mi->r);
	    }
	}
	else
	{
	    assert(mi->range_list != NULL);
	    miri->counts_left = mi->num_array_ranges;
	    miri->next_mr = mi->range_list + (mi->num_array_ranges - 1); 
	}
    }
    else
    {
	miri->next_mr = &_mr_all_valid;
	miri->counts_left = 1;
    }

    return miri;
}

/* Returns the next marker range in the sequence. */
bool Miri_Next(MarkerRange* dest, MarkerRevIterator *miri)
{
    return Miri_NEXT(dest, miri);
}


/* Cleans up the marker iterator instance. */
void Miri_Delete(MarkerRevIterator *miri)
{
    Mp_FreeMarkerRevIterator(miri);
}



MarkerRange* Mii_ctypes_Next(MarkerIterator* mii)
{
    static MarkerRange _mii_mr;
    bool okay = Mii_Next(&_mii_mr, mii);
    if(okay)
	printf("Mr = (%ld, %ld)\n", _mii_mr.start, _mii_mr.end);
    return okay ? &_mii_mr : NULL;
}

MarkerRange* Miri_ctypes_Next(MarkerRevIterator* miri)
{
    static MarkerRange _miri_mr;
    bool okay = Miri_Next(&_miri_mr, miri);
    return okay ? &_miri_mr : NULL;
}

markertype Mr_Start(cmr_ptr mr)
{
    return mr->start;
}

markertype Mr_End(cmr_ptr mr)
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

void Mi_Print(cmi_ptr mi)
{
    if(mi == NULL)
    {
	printf(" [-inf, inf)");
    }
    else if(mi->num_array_ranges == 0)
    {
	if(mi->r.start != mi->r.end)
	    printf(" [%ld,%ld)", mi->r.start, mi->r.end);
	else
	    printf(" [)");
    }
    else
    {
	int i;

	printf(" ");

	for(i = 0; i < mi->num_array_ranges; ++i)
	{
	    printf(" [%ld,%ld), ", mi->range_list[i].start, mi->range_list[i].end);
	}
    }
}

void Mi_debug_printMi(cmi_ptr mi)
{
     printf("MI %lxud: ", (size_t)mi);
     Mi_Print(mi);
     fflush(stdout);
}
  
