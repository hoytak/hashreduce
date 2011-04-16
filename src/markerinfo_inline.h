#ifndef _MARKERINFO_INLINE_H_
#define _MARKERINFO_INLINE_H_


static inline mi_ptr Mi_NEW(markertype start, markertype end)
{
    mi_ptr mi = ConstructMarkerInfo();

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


static inline mi_ptr Mi_NEW_INVALID()
{
    return ConstructMarkerInfo();
}

static inline bool Mi_VALID_EVERYWHERE(const MarkerInfo* mi)
{
    assert((mi == NULL) || mi->num_array_ranges != 1);

    return ((mi == NULL) 
	    || (mi->num_array_ranges == 0 
		&& mi->r.start == MARKER_MINUS_INFTY 
		&& mi->r.end == MARKER_PLUS_INFTY));
}

static inline bool Mi_VALID_ANYWHERE(const MarkerInfo* mi)
{
    assert((mi == NULL) || mi->num_array_ranges != 1);

    return ((mi == NULL) || (mi->num_array_ranges != 0) || (mi->r.start != mi->r.end));
}


/*** Macro version of NEXT methods above ***/

static inline bool Mii_NEXT(MarkerRange* dest, MarkerIterator *mii)
{
    if(unlikely(mii->counts_left == 0))
       return false;

    *dest = *(mii->next_mr);
    ++(mii->next_mr);
    --(mii->counts_left);
    
    return true;
}

static inline bool Miri_NEXT(MarkerRange* dest, MarkerRevIterator *miri)
{
    if(unlikely(miri->counts_left == 0))
       return false;

    *dest = *(miri->next_mr);
    --(miri->next_mr);
    --(miri->counts_left);
    
    return true;
}

#endif /* _MARKERINFO_INLINE_H_ */
