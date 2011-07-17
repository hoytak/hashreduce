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

#include "types.h"
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

#define MARKER_RANGE_DEFAULT {0,0}

typedef MarkerRange* mr_ptr;
typedef const MarkerRange* cmr_ptr;

typedef struct {
    OBJECT_ITEMS;
    MarkerRange r;
    size_t num_array_ranges, allocated_array_ranges;
    MarkerRange* range_list;
#ifndef NDEBUG
    size_t lock_count;
#endif
} MarkerInfo;

DECLARE_OBJECT(MarkerInfo);

typedef  MarkerInfo * _restrict_ mi_ptr;
typedef const MarkerInfo * _restrict_ cmi_ptr;

/*****************************************
 *
 *  Operations on the marker. 
 *
 ****************************************/

/* Disables warnings associated with the set. */
void Mi_DisableWarnings();

/* Create a new marker info class. */
mi_ptr Mi_New(markertype start, markertype end);

/* Creates a new marker info class that is valid nowhere. */
static inline mi_ptr Mi_NEW_INVALID();
mi_ptr Mi_NewInvalid();

/* Clear all the information out of the marker info class. */
void Mi_Clear(mi_ptr mi);

/* Add a range in which the marker is valid. */
void Mi_AddValidRange(mi_ptr mi, markertype r_lower, markertype r_higher);

/* Adds a valid range on the end.  Must be non-overlapping.  Faster
 * than above, but less safe. */
void Mi_AppendValidRange(mi_ptr mi, markertype r_lower, markertype r_higher);

/* Remove a range in which the marker is valid. */
void Mi_RemoveValidRange(mi_ptr mi, markertype r_lower, markertype r_higher);

/* Query whether the range is valid at a certain point. */
bool Mi_IsValid(cmi_ptr mi, markertype m);

/* Query whether the range is valid everywhere. */
bool Mi_ValidEverywhere(cmi_ptr mi);

/* Query whether the range is valid anywhere. */
bool Mi_ValidAnywhere(cmi_ptr mi);

/* If it has any valid range. */
bool Mi_IsEmpty(cmi_ptr mi);

/* In case we need a copy. */
mi_ptr Mi_Copy(cmi_ptr mi);

/* Tests whether two marker ranges are equal on all counts. */
bool Mi_Equal(cmi_ptr mi1, cmi_ptr);

/* Swaps the validity info in the two markerinfo objects. */
void Mi_Swap(mi_ptr mi1, mi_ptr mi2);


/*****************************************
 *
 *  Set operations
 *
 ****************************************/

/* Returns the complement of the two ranges. */
mi_ptr Mi_Complement(cmi_ptr mi);

/* Union of two ranges. */
mi_ptr Mi_Union(cmi_ptr mi1, cmi_ptr mi2);
mi_ptr Mi_UnionUpdate(mi_ptr mi1, cmi_ptr mi2);

/* Intersection of two ranges. */
mi_ptr Mi_Intersection(cmi_ptr mi1, cmi_ptr mi2);
mi_ptr Mi_IntersectionUpdate(mi_ptr mi1, cmi_ptr mi2);

/* All the elements in mi1 that aren't in mi2 */
mi_ptr Mi_Difference(cmi_ptr mi1, cmi_ptr mi2);

/* All the elements in exactly one set but not both. */
mi_ptr Mi_SymmetricDifference(cmi_ptr mi1, cmi_ptr mi2);

/*****************************************
 *
 *  Debug routines
 *
 ****************************************/

void Mi_Print(cmi_ptr mi);
void Mi_debug_printMi(cmi_ptr mi);

/****************************************
 *
 * Macro versions of some of the above.
 *
 ****************************************/

static inline bool Mi_ISEMPTY(cmi_ptr mi)
{
    if(unlikely(mi == NULL)) return true;
    
    return (mi->num_array_ranges == 0 && mi->r.start == mi->r.end);
}

static inline bool Mi_MINUS_INFTY_IS_VALID(cmi_ptr mi)
{
    return ((mi == NULL) ||
	    (mi->num_array_ranges == 0 
	     ? mi->r.start == MARKER_MINUS_INFTY 
	     : mi->range_list[0].start == MARKER_MINUS_INFTY));
}

static inline markertype Mi_Min(cmi_ptr mi)
{
    return (mi != NULL) 
	? (mi->num_array_ranges == 0 ? mi->r.start : mi->range_list[0].start)
	: MARKER_MINUS_INFTY;
}

static inline markertype Mi_Max(cmi_ptr mi)
{
    return (mi != NULL) 
	? (mi->num_array_ranges == 0 ? mi->r.end : mi->range_list[mi->num_array_ranges-1].end)
	: MARKER_PLUS_INFTY;
}

static inline void Mi_ClaimDebugLock(mi_ptr mi) 
{
#ifndef NDEBUG
    ++(mi->lock_count);
#endif
}

static inline void Mi_ReleaseDebugLock(mi_ptr mi)
{
#ifndef NDEBUG
    assert(mi->lock_count >= 1);
    --(mi->lock_count);
#endif
}

static inline size_t Mi_DebugLockCount(cmi_ptr mi)
{
#ifndef NDEBUG
    return mi->lock_count;
#else
    return 0;
#endif
}

static inline bool Mi_IsDebugLocked(cmi_ptr mi)
{
#ifndef NDEBUG
    return mi->lock_count != 0;
#else
    return false;
#endif
}

/************************************************************/
/* The primary purpose of these routines is to be used as testing and
 * debug routines that interface with ctypes easily.  */

markertype Mr_Start(cmr_ptr);
markertype Mr_End(cmr_ptr);
markertype Mr_Plus_Infinity();
markertype Mr_Minus_Infinity();
static inline markertype _Mr_Plus_Infinity(){return MARKER_PLUS_INFTY;}
static inline markertype _Mr_Minus_Infinity(){return MARKER_MINUS_INFTY;}

/********************************************************************************
 *
 *   Iterators for the marker ranges
 *
 ********************************************************************************/
 
/* Range iterations */
typedef struct {
    MEMORY_POOL_ITEMS;
    const MarkerRange *next_mr;
    size_t counts_left;
} MarkerIterator;

typedef struct {
    MEMORY_POOL_ITEMS;
    const MarkerRange *next_mr;
    size_t counts_left;
} MarkerRevIterator; 


/****** Forward iterators (good for most purposes) ****/

/* Allocates a new marker info iterator. */
MarkerIterator *Mii_New(cmi_ptr mi);

/* Returns the next marker range in the sequence. */
bool Mii_Next(MarkerRange*, MarkerIterator *mii);
static inline bool Mii_NEXT(MarkerRange*, MarkerIterator *mii);

/* Cleans up the marker iterator instance. */
void Mii_Delete(MarkerIterator *mii);


/****** Backward iterators (better for some algorithms in hash tables). *****/

/* Allocates a new marker info iterator that travels the list backwards. */
MarkerRevIterator *Miri_New(cmi_ptr mi);

/* Returns the next marker range in the sequence. */
bool Miri_Next(MarkerRange*, MarkerRevIterator *mii);
static inline bool Miri_NEXT(MarkerRange*, MarkerRevIterator *mii);

/* Cleans up the marker iterator instance. */
void Miri_Delete(MarkerRevIterator *miri);

/* Some functions for the testing routines. */
MarkerRange* Mii_ctypes_Next(MarkerIterator* mii);
MarkerRange* Miri_ctypes_Next(MarkerRevIterator* miri);


/* Debug stuff. */
void Mi_debug_Print(MarkerInfo *mi);

#include "markerinfo_inline.h"

#endif
