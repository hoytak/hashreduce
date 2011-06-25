/* This file provides some basic debugging features, such as an assert
 * macro which can be turned on or off at compile time. */

#ifndef DEBUGGING_H
#define DEBUGGING_H

#include <assert.h>
#include <stdbool.h>

#ifdef NDEBUG

/* Do things that disable debugging stuff; some things we just rely on
 * the optimizer and use deterministic if statements; other places, we
 * actually use defines to change behavior. */
#define IN_DEBUG_MODE 0

#warning "Compiling in release mode; type checking and other checks disabled."

#else

#ifdef ENABLE_CONSISTENCY_CHECKS
#define RUN_CONSISTENCY_CHECKS
#endif


#define IN_DEBUG_MODE 1
#define DEBUG_MODE

#endif

#ifdef DEBUG_MODE /* debug mode on. */

/* Define macros needed under debug mode. */
#include "optimizations.h"
#include <stdio.h>

/* Basically a soft version of assert in assert.h that doesn't call abort() */
#define ASSERT(a)							\
    do{									\
	if(unlikely(!(a)) )						\
	{								\
	    fprintf(stderr, "\nAssert Failed, %s  :  %s  :  %d  : \n",	\
		    __FILE__, __func__, __LINE__);			\
	    fprintf(stderr, "     %s \n\n", #a);			\
	    fflush(stderr);						\
	}								\
    }while(0)

#define ASSERTF(a, f)							\
    do{									\
	if(unlikely(!(a)) )						\
	{								\
	    ASSERT(a);							\
	    f;								\
	}								\
    }while(0)
	 
#define ASSERTM(a, m)							\
    do{									\
	if(unlikely(!(a)) )						\
	{								\
	    fprintf(stderr, "\nAssert Failed, %s  :  %s  :  %d  : \n",	\
		    __FILE__, __func__, __LINE__);			\
	    fprintf(stderr, "     %s \n\n", #a);			\
	    fprintf(stderr, "     %s \n\n", m);				\
	    fflush(stderr);						\
	}								\
    }while(0)

#define DBGMSG(m)						     \
    do{								     \
	fprintf(stderr, "\n%s:%d:\t%s.\n\n", __FILE__, __LINE__, m); \
	fflush(stderr);						     \
    }while(0) 

#define debug_false(t) (!(t))
#define debug_true(t) (!!(t))

#define DBHERE  DBGMSG("HERE")

#else /* debug mode off. */

//#warning "Debug Not On"

#define ASSERT(a) do{}while(0)

#define DBGMSG(m) do{}while(0)

#define ASSERTM(a, m) do{}while(0)

#define ASSERTF(a, f) do{}while(0)

#define debug_false(t) (false)
#define debug_true(t) (true)

#endif /* end debug mode off. */

/* A few other things, like memory checking. */

#define CHECK_MALLOC(x)							\
    do{									\
	if(unlikely((x) == NULL))					\
	{								\
	    fprintf(stderr, "Out of memory! (%s : %s : %d)",		\
		    __FILE__, __func__, __LINE__);			\
	    abort();							\
	}								\
    }while(0)

#endif
