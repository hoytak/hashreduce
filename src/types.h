#ifndef _TYPES_H_
#define _TYPES_H_

/* These macros are set by CMake in the configuration process. */

#ifdef USE__uint128_t
typedef __uint128_t uint128_type;
#elif USEuint128_t
typedef uint128_t uint128_type;
#elif USE__uint128
typedef __uint128 uint128_type;
#elif USEuint128
typedef uint128 uint128_type;
#elif USEunsigned__int128_t
typedef unsigned __int128_t uint128_type;
#elif USEunsignedint128_t
typedef unsigned int128_t uint128_type;
#elif USEunsignedint128
typedef unsigned __int128 uint128_type;
#elif USEunsignedint128
typedef unsigned int128 uint128_type;
#else
#define NO_UINT128
#endif

#ifdef RESTRICT_USE_restrict
#define _restrict_ restrict
#elif RESTRICT_USE___restrict
#define _restrict_ __restrict
#elif RESTRICT_USE__restrict
#define _restrict_ _restrict
#elif RESTRICT_USE___restrict__
#define _restrict_ __restrict__
#else
#define _restrict_
#endif

#endif /* _TYPES_H_ */
