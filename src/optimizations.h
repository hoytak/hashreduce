#ifndef OPTIMIZATIONS_H
#define OPTIMIZATIONS_H

/* Branching hints -- unlikely and likely */

#ifdef __GNUC__
/* Test for GCC > 2.95 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && (__GNUC_MINOR__ > 95)) 

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely_value(x, c)   __builtin_expect((x), c)

#else /* __GNUC__ > 2 ... */

#define likely(x)   (x)
#define unlikely(x) (x)
#define likely_value(x, c) (x)

#endif /* __GNUC__ > 2 ... */

#else /* __GNUC__ */

#define likely(x)   (x)
#define unlikely(x) (x)
#define likely_value(x, c) (x)

#endif /* __GNUC__ */

/* Prefetching operations */ 

/* Test for GCC >= 3.2 */
#if __GNUC__ > 3 || (__GNUC__ == 3 && (__GNUC_MINOR__ >= 2))

#define prefetch_ro(addr)  __builtin_prefetch((addr), 0)
#define prefetch_rw(addr)  __builtin_prefetch((addr), 1)
#define prefetch_ro_keep(addr)  __builtin_prefetch((addr), 0, 1)
#define prefetch_rw_keep(addr)  __builtin_prefetch((addr), 1, 1)

#else

#define prefetch_ro(addr)  
#define prefetch_rw(addr)  
#define prefetch_ro_keep(addr)  
#define prefetch_rw_keep(addr)  

#endif

#define SIZE_T_INFTY (~( (size_t) 0 ) )
#define SIZE_T_IS_INFTY(x) ( !(~((size_t)(x))) )

#endif /* OPTIMIZATIONS_H */

