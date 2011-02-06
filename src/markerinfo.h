/********************************************************************************
 *
 *  Marker information -- Every hash key has possible marker
 *  information associated with it.  This specifies ranges of marker
 *  values in which that item is valid or applies.  
 *  
 *
 ********************************************************************************/

#ifndef MARKERINFO_H
#define MARKERINFO_H

#include "object.h"
#include "memorypool.h"
#include <stdbool.h>

/* Set markertype */
typedef signed long int markertype;

/* Set the limits of markertype; if floating point type is use,
 * include float.h instead. */
#include <limits.h>

#define MARKER_MINUS_INFTY (LONG_MIN)
#define MARKER_PLUS_INFTY  (LONG_MAX) 

typedef struct {
    markertype start, end;
}MarkerRange;

typedef struct {
    OBJECT_ITEMS;
    MarkerRange r;
    unsigned long num_array_ranges, allocated_array_ranges;
    MarkerRange* range_list;
} MarkerInfo;

DECLARE_OBJECT(MarkerInfo);

/*****************************************
 *
 *  Operations on the marker. 
 *
 ****************************************/

/* Create a new marker info class. */
MarkerInfo* Mi_New(markertype start, markertype end);

/* Clear all the information out of the marker info class. */
void Mi_Clear(MarkerInfo* mi);

/* Add a range in which the marker is valid. */
void Mi_AddValidRange(MarkerInfo* mi, markertype r_lower, markertype r_higher);

/* Remove a range in which the marker is valid. */
void Mi_RemoveValidRange(MarkerInfo* mi, markertype r_lower, markertype r_higher);

/* Query whether the range is valid at a certain point. */
bool Mi_IsValid(MarkerInfo* mi, markertype m);

/* If it has any valid range. */
bool Mi_IsEmpty(MarkerInfo* mi);

/*****************************************
 *
 *  Set operations
 *
 ****************************************/

/* Returns the complement of the two ranges. */
MarkerInfo* Mi_Complement(MarkerInfo* mi);

/* Union of two ranges. */
MarkerInfo* Mi_Union(MarkerInfo* mi1, MarkerInfo* mi2);

/* Intersection of two ranges. */
MarkerInfo* Mi_Intersection(MarkerInfo* mi1, MarkerInfo* mi2);

/* All the elements in mi1 that aren't in mi2 */
MarkerInfo* Mi_Difference(MarkerInfo* mi1, MarkerInfo* mi2);

/* All the elements in exactly one set but not both. */
MarkerInfo* Mi_SymmetricDifference(MarkerInfo* mi1, MarkerInfo* mi2);

/*****************************************
 *
 *  Debug routines
 *
 ****************************************/

void Mi_debug_printMi(MarkerInfo* mi);

/****************************************
 *
 * Macro versions of some of the above.
 *
 ****************************************/

static inline bool Mi_ISEMPTY(MarkerInfo* mi)
{
    if(unlikely(mi == NULL)) return true;
    
    return (mi->num_array_ranges == 0 && mi->r.start == mi->r.end);
}

static inline markertype Mi_Min(MarkerInfo* mi)
{
    return (mi != NULL) 
	? (mi->num_array_ranges == 0 ? mi->r.start : mi->range_list[0].start)
	: MARKER_MINUS_INFTY;
}

static inline markertype Mi_Max(MarkerInfo* mi)
{
    return (mi != NULL) 
	? (mi->num_array_ranges == 0 ? mi->r.end : mi->range_list[mi->num_array_ranges-1].end)
	: MARKER_PLUS_INFTY;
}
 
/* Range iterations */
typedef struct {
    MEMORY_POOL_ITEMS;
    MarkerInfo *mi;
    size_t next_range_index;
} MarkerIterator; 

typedef struct {
    MEMORY_POOL_ITEMS;
    MarkerInfo *mi;
    size_t previous_range_index;
} MarkerRevIterator; 


/****** Forward iterators (good for most purposes) ****/

/* Allocates a new marker info iterator. */
MarkerIterator *NewMarkerIterator(MarkerInfo *mi);

/* Returns the next marker range in the sequence. */
MarkerRange* Mii_Next(MarkerIterator *mii);

/* Cleans up the marker iterator instance. */
void Mii_Delete(MarkerIterator *mii);


/****** Backward iterators (better for some algorithms in hash tables). *****/

/* Allocates a new marker info iterator that travels the list backwards. */
MarkerRevIterator *NewReversedMarkerIterator(MarkerInfo *mi);

/* Returns the next marker range in the sequence. */
MarkerRange* Miri_Next(MarkerRevIterator *mii);

/* Cleans up the marker iterator instance. */
void Miri_Delete(MarkerRevIterator *miri);

/*** Macro version of NEXT methods above ***/

static inline MarkerRange* Mii_NEXT(MarkerIterator *mii)
{
    MarkerInfo *mi = mii->mi;

    if(unlikely(mi == NULL))
	return NULL;
   
    if(unlikely(mi->num_array_ranges == 0))
    {
	if(mii->next_range_index == 1)
	    return NULL;
	else
	{
	    mii->next_range_index = 1;
	    return &(mi->r);	    
	}
    }
    else
    {	    
	if(unlikely(mii->next_range_index >= mi->num_array_ranges))
	    return NULL;
		
	MarkerRange *ret = &(mi->range_list[mii->next_range_index]);
	++mii->next_range_index;
	return ret;
    }
}

static inline MarkerRange* Miri_NEXT(MarkerRevIterator *miri)
{
    MarkerRange *r;

    while(1)
    {
	if(unlikely(miri->previous_range_index == 0))
	    return NULL;

	MarkerInfo *mi = miri->mi;
 
	if(unlikely(mi->num_array_ranges == 0))
	{
	    assert(miri->previous_range_index == 1);
	    miri->previous_range_index = 0;

	    r = &(mi->r);

	}
	else
	{
	    --miri->previous_range_index;
	    assert(miri->previous_range_index >= 0);
	    assert(miri->previous_range_index < mi->num_array_ranges);

	    r = &(mi->range_list[miri->previous_range_index]);

	}
	
	if(likely(r->start != r->end))
	    return r;
    }
}

/************************************************************/
/* The primary purpose of these routines is to be used as testing and
 * debug routines that interface with ctypes easily.  */

markertype Mr_Start(MarkerRange *);
markertype Mr_End(MarkerRange *);
markertype Mr_Plus_Infinity();
markertype Mr_Minus_Infinity();

#endif
