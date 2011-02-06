/* Except where marked, the notice below applies.   
 * 
 * Modifications were done by Hoyt Koepke, hoytak@gmail.com.  The only
 * use case for this code is a strong hash function; as such, we are
 * not constrained to follow the standard.  Several aspects of the
 * original md5 algorithm were modified.  All endianness
 * considerations were stripped out in favor of the fastest
 * implementation, as we do not provide any guarantees that hash
 * functions will be consistent between implementations or computers.
 *
 * Furthermore, some common use cases only require a partial use of
 * the buffer, thus we created an inline implementation of md5_process
 * that can be optimized by the compiler to take advantage of this.
 * As such, we rewrote md5_finish to also take advantage of this.
 *
*/

/* 
  Copyright (C) 1999, 2000, 2002 Aladdin Enterprises.  All rights reserved.

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  L. Peter Deutsch
  ghost@aladdin.com

 */
/* $Id: md5.c,v 1.6 2002/04/13 19:20:28 lpd Exp $ */
/*
  Independent implementation of MD5 (RFC 1321).

  This code implements the MD5 Algorithm defined in RFC 1321, whose
  text is available at
	http://www.ietf.org/rfc/rfc1321.txt
  The code is derived from the text of the RFC, including the test suite
  (section A.5) but excluding the rest of Appendix A.  It does not include
  any code or documentation that is identified in the RFC as being
  copyrighted.

  The original and principal author of md5.c is L. Peter Deutsch
  <ghost@aladdin.com>.  Other authors are noted in the change history
  that follows (in reverse chronological order):

  2002-04-13 lpd Clarified derivation from RFC 1321; now handles byte order
	either statically or dynamically; added missing #include <string.h>
	in library.
  2002-03-11 lpd Corrected argument list for main(), and added int return
	type, in test program and T value program.
  2002-02-21 lpd Added missing #include <stdio.h> in test program.
  2000-07-03 lpd Patched to eliminate warnings about "constant is
	unsigned in ANSI C, signed in traditional"; made test program
	self-checking.
  1999-11-04 lpd Edited comments slightly for automatic TOC extraction.
  1999-10-18 lpd Fixed typo in header comment (ansi2knr rather than md5).
  1999-05-03 lpd Original version.

  Modifications: 2009-07-14, by Hoyt Koepke.  Marked several functions
  as inline for speed in other functions, removed endianness
  considerations for speed, and other tweeks for speed.  
 */

#include "hashobject.h"
#include "bitops.h"
#include <string.h>

/********************************************************************************
 *
 *  Basic Hash key operations that don't involve actual hashing considerations.
 *
 ********************************************************************************/

bool Hk_Equal(chk_ptr hk1, chk_ptr hk2)
{
    assert(hk1 != NULL);
    assert(hk2 != NULL);
    return Hk_EQUAL(hk1, hk2);
}

bool Hk_IsZero(chk_ptr hk)
{
    assert(hk != NULL);
    return Hk_ISZERO(hk);
}

void Hk_Clear(hk_ptr hk)
{
    Hk_CLEAR(hk);
}

void Hk_Copy(hk_ptr hk_dest, chk_ptr hk_src)
{
    Hk_COPY(hk_dest, hk_src);
}

inline void check_hashkey_range(hk_ptr hk)
{
    /* Ensure (pedantically) that we're less than 2^128 - 151 */
    if(unlikely(!(~hk->hk64[0])))
    {
	if(unlikely(hk->hk64[1] >= 0xFFFFFFFFFFFFFF69ull) )
	{
	    hk->hk64[0] -= 1;
	}
    }
}    


/*********************************************************************************
 *
 *  Now we're ready to define all the functions needed to fill the
 *  hash objects.
 *
 *********************************************************************************/

/********* The following material subject to the notice at the top. *********/

typedef unsigned char md5_byte_t; /* 8-bit byte */
typedef unsigned int md5_word_t;  /* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state_s {
    md5_word_t abcd[4];		/* digest buffer */
} md5_state_t;

/********* End cut and past *********/

#define T_MASK ((md5_word_t)~0)
#define T1 /* 0xd76aa478 */ (T_MASK ^ 0x28955b87)
#define T2 /* 0xe8c7b756 */ (T_MASK ^ 0x173848a9)
#define T3    0x242070db
#define T4 /* 0xc1bdceee */ (T_MASK ^ 0x3e423111)
#define T5 /* 0xf57c0faf */ (T_MASK ^ 0x0a83f050)
#define T6    0x4787c62a
#define T7 /* 0xa8304613 */ (T_MASK ^ 0x57cfb9ec)
#define T8 /* 0xfd469501 */ (T_MASK ^ 0x02b96afe)
#define T9    0x698098d8
#define T10 /* 0x8b44f7af */ (T_MASK ^ 0x74bb0850)
#define T11 /* 0xffff5bb1 */ (T_MASK ^ 0x0000a44e)
#define T12 /* 0x895cd7be */ (T_MASK ^ 0x76a32841)
#define T13    0x6b901122
#define T14 /* 0xfd987193 */ (T_MASK ^ 0x02678e6c)
#define T15 /* 0xa679438e */ (T_MASK ^ 0x5986bc71)
#define T16    0x49b40821
#define T17 /* 0xf61e2562 */ (T_MASK ^ 0x09e1da9d)
#define T18 /* 0xc040b340 */ (T_MASK ^ 0x3fbf4cbf)
#define T19    0x265e5a51
#define T20 /* 0xe9b6c7aa */ (T_MASK ^ 0x16493855)
#define T21 /* 0xd62f105d */ (T_MASK ^ 0x29d0efa2)
#define T22    0x02441453
#define T23 /* 0xd8a1e681 */ (T_MASK ^ 0x275e197e)
#define T24 /* 0xe7d3fbc8 */ (T_MASK ^ 0x182c0437)
#define T25    0x21e1cde6
#define T26 /* 0xc33707d6 */ (T_MASK ^ 0x3cc8f829)
#define T27 /* 0xf4d50d87 */ (T_MASK ^ 0x0b2af278)
#define T28    0x455a14ed
#define T29 /* 0xa9e3e905 */ (T_MASK ^ 0x561c16fa)
#define T30 /* 0xfcefa3f8 */ (T_MASK ^ 0x03105c07)
#define T31    0x676f02d9
#define T32 /* 0x8d2a4c8a */ (T_MASK ^ 0x72d5b375)
#define T33 /* 0xfffa3942 */ (T_MASK ^ 0x0005c6bd)
#define T34 /* 0x8771f681 */ (T_MASK ^ 0x788e097e)
#define T35    0x6d9d6122
#define T36 /* 0xfde5380c */ (T_MASK ^ 0x021ac7f3)
#define T37 /* 0xa4beea44 */ (T_MASK ^ 0x5b4115bb)
#define T38    0x4bdecfa9
#define T39 /* 0xf6bb4b60 */ (T_MASK ^ 0x0944b49f)
#define T40 /* 0xbebfbc70 */ (T_MASK ^ 0x4140438f)
#define T41    0x289b7ec6
#define T42 /* 0xeaa127fa */ (T_MASK ^ 0x155ed805)
#define T43 /* 0xd4ef3085 */ (T_MASK ^ 0x2b10cf7a)
#define T44    0x04881d05
#define T45 /* 0xd9d4d039 */ (T_MASK ^ 0x262b2fc6)
#define T46 /* 0xe6db99e5 */ (T_MASK ^ 0x1924661a)
#define T47    0x1fa27cf8
#define T48 /* 0xc4ac5665 */ (T_MASK ^ 0x3b53a99a)
#define T49 /* 0xf4292244 */ (T_MASK ^ 0x0bd6ddbb)
#define T50    0x432aff97
#define T51 /* 0xab9423a7 */ (T_MASK ^ 0x546bdc58)
#define T52 /* 0xfc93a039 */ (T_MASK ^ 0x036c5fc6)
#define T53    0x655b59c3
#define T54 /* 0x8f0ccc92 */ (T_MASK ^ 0x70f3336d)
#define T55 /* 0xffeff47d */ (T_MASK ^ 0x00100b82)
#define T56 /* 0x85845dd1 */ (T_MASK ^ 0x7a7ba22e)
#define T57    0x6fa87e4f
#define T58 /* 0xfe2ce6e0 */ (T_MASK ^ 0x01d3191f)
#define T59 /* 0xa3014314 */ (T_MASK ^ 0x5cfebceb)
#define T60    0x4e0811a1
#define T61 /* 0xf7537e82 */ (T_MASK ^ 0x08ac817d)
#define T62 /* 0xbd3af235 */ (T_MASK ^ 0x42c50dca)
#define T63    0x2ad7d2bb
#define T64 /* 0xeb86d391 */ (T_MASK ^ 0x14792c6e)

#define Cp1(typename, r)						\
    ( sizeof(typename) <= 32 ? (r) : getBitGroup((unsigned long)(r),32,32)) 

#define Cp2(typename, r)						\
    ( sizeof(typename) <= 32 ? (0) : getBitGroup((unsigned long)(r),0,32)) 

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
	
/* Define the set operations for the md5 stuff.  This code was
 * rearranged by hoyt koepke from the original code.  
 *
 * I chose to implement the md5 calculation as a macro, even though
 * this may cause some code bloat, for two reasons:
 *
 * 1. The usual design pattern is to do a long series of the same
 * operations (e.g. creating hashes of 1000s of strings).  Thus the
 * code for that process will be in the cache.
 *
 * 2. Many of the processes set 12-15 of the 16 elements needed in
 * calculating the hash to 0.  Thus many steps in the calculation can
 * be optimized out or evaluated by the optimizer at compile time.
 */

#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define SETF(a, b, c, d, x, s, Ti)		\
    t = a + F(b,c,d) + (x) + Ti;		\
    a = ROTATE_LEFT(t, s) + b

#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define SETG(a, b, c, d, x, s, Ti)		\
    t = a + G(b,c,d) + (x) + Ti;		\
    a = ROTATE_LEFT(t, s) + b

#define H(x, y, z) ((x) ^ (y) ^ (z))
#define SETH(a, b, c, d, x, s, Ti)		\
    t = a + H(b,c,d) + (x) + Ti;		\
    a = ROTATE_LEFT(t, s) + b

#define I(x, y, z) ((y) ^ ((x) | ~(z)))
#define SETI(a, b, c, d, x, s, Ti)		\
    t = a + I(b,c,d) + (x) + Ti;		\
    a = ROTATE_LEFT(t, s) + b

inline void md5_buffer_process(
    md5_state_t *pms_ptr, 
    md5_word_t x0, md5_word_t x1, md5_word_t x2, md5_word_t x3,
    md5_word_t x4, md5_word_t x5, md5_word_t x6, md5_word_t x7, 
    md5_word_t x8, md5_word_t x9, md5_word_t x10, md5_word_t x11,
    md5_word_t x12, md5_word_t x13, md5_word_t x14, md5_word_t x15)	
{
    md5_word_t
	a = (pms_ptr)->abcd[0], b = (pms_ptr)->abcd[1],
	c = (pms_ptr)->abcd[2], d = (pms_ptr)->abcd[3];

    md5_word_t t;

    /* Round 1. */
    /* Let [abcd k s i] denote the operation			
       a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */

    /* Do the following 16 operations. */
    SETF(a, b, c, d,  (x0),  7,  T1);
    SETF(d, a, b, c,  (x1), 12,  T2);
    SETF(c, d, a, b,  (x2), 17,  T3);
    SETF(b, c, d, a,  (x3), 22,  T4);
    SETF(a, b, c, d,  (x4),  7,  T5);
    SETF(d, a, b, c,  (x5), 12,  T6);
    SETF(c, d, a, b,  (x6), 17,  T7);
    SETF(b, c, d, a,  (x7), 22,  T8);
    SETF(a, b, c, d,  (x8),  7,  T9);
    SETF(d, a, b, c,  (x9), 12, T10);
    SETF(c, d, a, b, (x10), 17, T11);
    SETF(b, c, d, a, (x11), 22, T12);
    SETF(a, b, c, d, (x12),  7, T13);
    SETF(d, a, b, c, (x13), 12, T14);
    SETF(c, d, a, b, (x14), 17, T15);
    SETF(b, c, d, a, (x15), 22, T16);

    /* Round 2. */
    /* Let [abcd k s i] denote the operation
       a = b + ((a + G(b,c,d) + Y[k] + T[i]) <<< s). */

    /* Do the following 16 operations. */
    SETG(a, b, c, d,  (x1),  5, T17);
    SETG(d, a, b, c,  (x6),  9, T18);
    SETG(c, d, a, b, (x11), 14, T19);
    SETG(b, c, d, a,  (x0), 20, T20);
    SETG(a, b, c, d,  (x5),  5, T21);
    SETG(d, a, b, c, (x10),  9, T22);
    SETG(c, d, a, b, (x15), 14, T23);
    SETG(b, c, d, a,  (x4), 20, T24);
    SETG(a, b, c, d,  (x9),  5, T25);
    SETG(d, a, b, c, (x14),  9, T26);
    SETG(c, d, a, b,  (x3), 14, T27);
    SETG(b, c, d, a,  (x8), 20, T28);
    SETG(a, b, c, d, (x13),  5, T29);
    SETG(d, a, b, c,  (x2),  9, T30);
    SETG(c, d, a, b,  (x7), 14, T31);
    SETG(b, c, d, a, (x12), 20, T32);

    /* Round 3. */
    /* Let [abcd k s t] denote the operation
       a = b + ((a + H(b,c,d) + Y[k] + T[i]) <<< s). */

    /* Do the following 16 operations. */
    SETH(a, b, c, d,  (x5),  4, T33);
    SETH(d, a, b, c,  (x8), 11, T34);
    SETH(c, d, a, b, (x11), 16, T35);
    SETH(b, c, d, a, (x14), 23, T36);
    SETH(a, b, c, d,  (x1),  4, T37);
    SETH(d, a, b, c,  (x4), 11, T38);
    SETH(c, d, a, b,  (x7), 16, T39);
    SETH(b, c, d, a, (x10), 23, T40);
    SETH(a, b, c, d, (x13),  4, T41);
    SETH(d, a, b, c,  (x0), 11, T42);
    SETH(c, d, a, b,  (x3), 16, T43);
    SETH(b, c, d, a,  (x6), 23, T44);
    SETH(a, b, c, d,  (x9),  4, T45);
    SETH(d, a, b, c, (x12), 11, T46);
    SETH(c, d, a, b, (x15), 16, T47);
    SETH(b, c, d, a,  (x2), 23, T48);

    /* Round 4. */
    /* Let [abcd k s t] denote the operation
       a = b + ((a + I(b,c,d) + Y[k] + T[i]) <<< s). */

    /* Do the following 16 operations. */
    SETI(a, b, c, d,  (x0),  6, T49);
    SETI(d, a, b, c,  (x7), 10, T50);
    SETI(c, d, a, b, (x14), 15, T51);
    SETI(b, c, d, a,  (x5), 21, T52);
    SETI(a, b, c, d, (x12),  6, T53);
    SETI(d, a, b, c,  (x3), 10, T54);
    SETI(c, d, a, b, (x10), 15, T55);
    SETI(b, c, d, a,  (x1), 21, T56);
    SETI(a, b, c, d,  (x8),  6, T57);
    SETI(d, a, b, c, (x15), 10, T58);
    SETI(c, d, a, b,  (x6), 15, T59);
    SETI(b, c, d, a, (x13), 21, T60);
    SETI(a, b, c, d,  (x4),  6, T61);
    SETI(d, a, b, c, (x11), 10, T62);
    SETI(c, d, a, b,  (x2), 15, T63);
    SETI(b, c, d, a,  (x9), 21, T64);

    /* Then perform the following additions. (That is increment
       each of the four registers by the value it had before this
       block was started.) */
    (pms_ptr)->abcd[0] += a;
    (pms_ptr)->abcd[1] += b;
    (pms_ptr)->abcd[2] += c;
    (pms_ptr)->abcd[3] += d;
}

inline void weakmd5_hashkey_update(
    hk_ptr hk, 
    md5_word_t x0, md5_word_t x1, md5_word_t x2, md5_word_t x3,
    md5_word_t x4, md5_word_t x5, md5_word_t x6, md5_word_t x7, 
    md5_word_t x8, md5_word_t x9, md5_word_t x10, md5_word_t x11,
    md5_word_t x12, md5_word_t x13, md5_word_t x14, md5_word_t x15)	
{
    assert(hk != NULL);

    md5_word_t
	a = hk->hk32[0], b = hk->hk32[1],
	c = hk->hk32[2], d = hk->hk32[3];

    md5_word_t t;

    /* This hash function is weaker and faster, made so by leaving off
     * rounds 3 and 4; it is intended for updating and combining other
     * hash keys in a way that freezes all the invariance
     * relationships in previous parts. */

    /* Round 1. */
    /* Let [abcd k s i] denote the operation			
       a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */

    /* Do the following 16 operations. */
    SETF(a, b, c, d,  (x0),  7,  T1);
    SETF(d, a, b, c,  (x1), 12,  T2);
    SETF(c, d, a, b,  (x2), 17,  T3);
    SETF(b, c, d, a,  (x3), 22,  T4);
    SETF(a, b, c, d,  (x4),  7,  T5);
    SETF(d, a, b, c,  (x5), 12,  T6);
    SETF(c, d, a, b,  (x6), 17,  T7);
    SETF(b, c, d, a,  (x7), 22,  T8);
    SETF(a, b, c, d,  (x8),  7,  T9);
    SETF(d, a, b, c,  (x9), 12, T10);
    SETF(c, d, a, b, (x10), 17, T11);
    SETF(b, c, d, a, (x11), 22, T12);
    SETF(a, b, c, d, (x12),  7, T13);
    SETF(d, a, b, c, (x13), 12, T14);
    SETF(c, d, a, b, (x14), 17, T15);
    SETF(b, c, d, a, (x15), 22, T16);

    /* Round 2. */
    /* Let [abcd k s i] denote the operation
       a = b + ((a + G(b,c,d) + Y[k] + T[i]) <<< s). */

    /* Do the following 16 operations. */
    SETG(a, b, c, d,  (x1),  5, T17);
    SETG(d, a, b, c,  (x6),  9, T18);
    SETG(c, d, a, b, (x11), 14, T19);
    SETG(b, c, d, a,  (x0), 20, T20);
    SETG(a, b, c, d,  (x5),  5, T21);
    SETG(d, a, b, c, (x10),  9, T22);
    SETG(c, d, a, b, (x15), 14, T23);
    SETG(b, c, d, a,  (x4), 20, T24);
    SETG(a, b, c, d,  (x9),  5, T25);
    SETG(d, a, b, c, (x14),  9, T26);
    SETG(c, d, a, b,  (x3), 14, T27);
    SETG(b, c, d, a,  (x8), 20, T28);
    SETG(a, b, c, d, (x13),  5, T29);
    SETG(d, a, b, c,  (x2),  9, T30);
    SETG(c, d, a, b,  (x7), 14, T31);
    SETG(b, c, d, a, (x12), 20, T32);

    /* Then perform the following additions. (That is increment each
       of the four hash key registers by the value it had before this
       block was started.) */
    hk->hk32[0] += a;
    hk->hk32[1] += b;
    hk->hk32[2] += c;
    hk->hk32[3] += d;
    
    check_hashkey_range(hk);
}


#define md5_ap_ArEl(n,c,len) ((len) > (n) ? (*((c) + (n))) : 0)

inline void md5_array_process(md5_state_t *pms_ptr, const md5_word_t* c, const int len)
{
    const md5_word_t _md5_ap_c0 = md5_ap_ArEl(0, (c), (len));
    const md5_word_t _md5_ap_c1 = md5_ap_ArEl(1, (c), (len));
    const md5_word_t _md5_ap_c2 = md5_ap_ArEl(2, (c), (len));
    const md5_word_t _md5_ap_c3 = md5_ap_ArEl(3, (c), (len));
    const md5_word_t _md5_ap_c4 = md5_ap_ArEl(4, (c), (len));
    const md5_word_t _md5_ap_c5 = md5_ap_ArEl(5, (c), (len));
    const md5_word_t _md5_ap_c6 = md5_ap_ArEl(6, (c), (len));
    const md5_word_t _md5_ap_c7 = md5_ap_ArEl(7, (c), (len));
    const md5_word_t _md5_ap_c8 = md5_ap_ArEl(8, (c), (len));
    const md5_word_t _md5_ap_c9 = md5_ap_ArEl(9, (c), (len));
    const md5_word_t _md5_ap_c10 = md5_ap_ArEl(10, (c), (len));
    const md5_word_t _md5_ap_c11 = md5_ap_ArEl(11, (c), (len));
    const md5_word_t _md5_ap_c12 = md5_ap_ArEl(12, (c), (len));
    const md5_word_t _md5_ap_c13 = md5_ap_ArEl(13, (c), (len));
    const md5_word_t _md5_ap_c14 = md5_ap_ArEl(14, (c), (len));
    const md5_word_t _md5_ap_c15 = md5_ap_ArEl(15, (c), (len));

    md5_buffer_process(
	(pms_ptr),
	_md5_ap_c0,_md5_ap_c1,_md5_ap_c2,_md5_ap_c3,
	_md5_ap_c4,_md5_ap_c5,_md5_ap_c6,_md5_ap_c7,
	_md5_ap_c8,_md5_ap_c9,_md5_ap_c10,_md5_ap_c11,
	_md5_ap_c12,_md5_ap_c13,_md5_ap_c14,_md5_ap_c15);		
}

/* This function was written by Hoyt Koepke */
inline void md5_process(md5_state_t *pms, const md5_byte_t *data /*[64]*/)
{
#define B2W(n) ( *((const uint32_t*)(data)) + (n))
	
	md5_buffer_process(
	    pms,
	    B2W(0), B2W(1), B2W(2), B2W(3), 
	    B2W(4), B2W(5), B2W(6), B2W(7), 
	    B2W(8), B2W(9), B2W(10), B2W(11), 
	    B2W(12), B2W(13), B2W(14), B2W(15));
#undef B2W
}

/* The purpose of using __LINE__ as a salt is simply to ensure that
 * none of the functions below return, in any way, the same hash
 * (e.g. hash of a string vs of an integer).  If the compiler's
 * optimizer is any good, there should be no performance penalty to
 * using md5_buffer_process with a salt to refine the initial state
 * (here relying on the __LINE__ macro to ensure uniqueness between
 * the different functions below), as it should all become a set of
 * constant expressions that can be evaluated at compile time -- the
 * standard doesn't say it has to be, though.  */

#define md5_init(pms_ptr)						\
    do{									\
	(pms_ptr)->abcd[0] = 0x67452301;				\
	(pms_ptr)->abcd[1] = /*0xefcdab89*/ (T_MASK ^ 0x10325476);	\
	(pms_ptr)->abcd[2] = /*0x98badcfe*/ (T_MASK ^ 0x67452301);	\
	(pms_ptr)->abcd[3] = 0x10325476;				\
									\
	md5_buffer_process(						\
	    (pms_ptr),							\
	    __LINE__, __LINE__, __LINE__, __LINE__,			\
	    __LINE__, __LINE__, __LINE__, __LINE__,			\
	    __LINE__, __LINE__, __LINE__, __LINE__,			\
	    __LINE__, __LINE__, __LINE__, __LINE__);			\
    }while(0)

void inline md5_run(md5_state_t *pms, const md5_byte_t *data, size_t nbytes)
{
    /* This part is modified; we need only a one-shot version of the
     * algorithm, so we do not have to be as careful with storing
     * intermediate data. */

    md5_byte_t buf[64];
    size_t bytes_left;

    for(bytes_left = nbytes; bytes_left >= 64; bytes_left -= 64)
	md5_process(pms, data);
    
    if(bytes_left == 0) return;

    /* Get the remaining bit */

    memcpy(buf, data, bytes_left);
    memset(buf+nbytes, 0, 64 - bytes_left);
    
    md5_process(pms, buf);
    
    union {
	size_t v64; 
	uint32_t v32[2];
    } nbytes_sep;

    nbytes_sep.v64 = nbytes;

    /* process again to put the length in there. */
    md5_buffer_process(
	pms, 
	0x8000000, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, nbytes_sep.v32[0], nbytes_sep.v32[1]);
}

/* The following function was modified from its original form to fill
 * a hash key instead of an 8 byte digest. */

void inline md5_finish(HashKey* hk, md5_state_t *pms)
{    
    /* Adding in the count is removed; it isn't needed for our
     * purposes. */

    assert(hk != NULL);

    hk->hk32[0] = (uint32_t)pms->abcd[0];
    hk->hk32[1] = (uint32_t)pms->abcd[1];
    hk->hk32[2] = (uint32_t)pms->abcd[2];
    hk->hk32[3] = (uint32_t)pms->abcd[3];

    check_hashkey_range(hk);
}

/*******************************************************************************
 *
 *  Routines for creating hash values from other objects.
 *
 ********************************************************************************/

/* Create field from string. */
void Hkf_FromString(hk_ptr dest_key, const char *string)
{
    Hkf_FromCharBuffer(dest_key, string, strlen(string));
}

/* Create field from char buffer (just like a string, except length is
 * a set paraemter. */
void Hkf_FromCharBuffer(hk_ptr dest_key, const char *string, size_t length)
{
    md5_state_t pms;
    
    md5_init(&pms);
    md5_run(&pms, (md5_byte_t *)string, length);
    md5_finish(dest_key, &pms);
}

void Hkf_FromIntBuffer(hk_ptr dest_key, const unsigned int *it, size_t length)
{
    md5_state_t pms;
    
    md5_init(&pms);
    md5_run(&pms, (md5_byte_t *)it, length*(sizeof(unsigned int) / sizeof(md5_byte_t)));
    md5_finish(dest_key, &pms);
}

inline void _Hf_FromUInt(hk_ptr dest_key, unsigned long x, unsigned long salt)
{
    md5_state_t pms;
    md5_init(&pms);

    if(sizeof(unsigned long) == 4)
    {
	/* Just need to distinguish everything from, e.g., a zero-length string. */
	md5_buffer_process(
	    &pms,
	    0,   x,0,0,   
	    0,salt,0,0,
	    0,   0,0,0,
	    0,   0,0,0);
    }
    else
    {
	md5_buffer_process(
	    &pms, 
	    b64high(x),b64low(x),0,0,   
	    b64high(salt),b64low(salt),0,0,   
	    0,0,0,0,
	    0,0,0,0);
    }

    md5_finish(dest_key, &pms);
}

void Hkf_FromInt(hk_ptr dest_key, signed long x)
{
    if(x < 0)
    {
	_Hf_FromUInt(dest_key, (unsigned long)(-x), __LINE__);
    }
    else
    {
	Hkf_FromUnsignedInt(dest_key, x);
    }
}

void Hkf_FromUnsignedInt(hk_ptr dest_key, unsigned long x)
{
    _Hf_FromUInt(dest_key, x, __LINE__);
}

void Hkf_FromHashKey(hk_ptr dest_key, chk_ptr hk)
{
    /* Can use the weaker, faster version since we already have strong hashes. */

    Hk_COPY(dest_key, hk);

    weakmd5_hashkey_update(
	dest_key, 
	hk->hk32[0], hk->hk32[1], hk->hk32[2], hk->hk32[3],
	0,0,0,0,
	0,0,0,0,
	0,0,0,__LINE__);
}

void Hkf_FromHashKeyAndInt(hk_ptr dest_key, chk_ptr hk, signed long x)
{
    Hk_COPY(dest_key, hk);

    weakmd5_hashkey_update(
	dest_key, 
	hk->hk32[0], hk->hk32[1], hk->hk32[2], hk->hk32[3],
	x,x,x,x,
	x,x,x,x,
	x,x,x,__LINE__);
}

/************************************************************
 * 
 *  Atomic operations between hash keys.  The ones involving a
 *  "rehashing" are here, as they are more substantial functions.
 *
 ************************************************************/

void Hk_Combine(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2)
{
    assert(dest_key != NULL);
    assert(hk1 != NULL);
    assert(hk2 != NULL);
    
    Hk_COPY(dest_key, hk1);
    Hk_InplaceCombine(dest_key, hk2);
}


/* Update the value of the first depending on the value of the second. */
void Hk_InplaceCombine(hk_ptr dest_key, chk_ptr hk)
{
    weakmd5_hashkey_update(dest_key, 
			   hk->hk32[0],hk->hk32[1],hk->hk32[2],hk->hk32[3],
			   0,0,0,0, 
			   0,0,0,0, 
			   0,0,0,__LINE__);
}

/* Do a Rehashing of the key hk; note this is not the Rehash function
 * below, which special cases the null hash. */
void Hk_InplaceHash(hk_ptr hk)
{
    weakmd5_hashkey_update(hk, 
			   hk->hk32[0],hk->hk32[1],hk->hk32[2],hk->hk32[3],
			   0,0,0,0, 
			   0,0,0,0, 
			   0,0,0,__LINE__);
}

/* Fast functions for use with various hashing operations elsewhere.
 * These are not equivalent, in any way, to Hk_Update. */
void Hk_UpdateWithFourHashes(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2,chk_ptr hk3,chk_ptr hk4)
{
    
    weakmd5_hashkey_update(dest_key, 
			   hk1->hk32[0],hk1->hk32[1],hk1->hk32[2],hk1->hk32[3],
			   hk2->hk32[0],hk2->hk32[1],hk2->hk32[2],hk2->hk32[3],
			   hk3->hk32[0],hk3->hk32[1],hk3->hk32[2],hk3->hk32[3],
			   hk4->hk32[0],hk4->hk32[1],hk4->hk32[2],hk4->hk32[3]);
}

void Hk_UpdateWithInt(hk_ptr dest_key, uint64_t v)
{
    const unsigned int h = b64high(v);
    const unsigned int l = b64low(v);
    
    weakmd5_hashkey_update(dest_key, 
			   l,h,l,h,
			   l,h,l,h,
			   0,0,0,0,
			   0,0,0,__LINE__);
}

void Hk_UpdateWithIntArray(hk_ptr dest_key, const unsigned int *it, size_t len)
{
    HashKey a_hk;
    Hkf_FromIntBuffer(&a_hk, it, len);
    Hk_InplaceCombine(dest_key, &a_hk);
}

void Hk_Reduce(hk_ptr dest_key, chk_ptr hk1, chk_ptr hk2)
{
    assert(dest_key != NULL);
    assert(hk1 != NULL);
    assert(hk2 != NULL);

    Hk_REDUCE(dest_key, hk1, hk2);
}

void Hk_ReduceUpdate(hk_ptr dest_key, chk_ptr hk)
{
    assert(dest_key != NULL);
    assert(hk != NULL);

    Hk_REDUCE_UPDATE(dest_key, hk);
}

void Hk_Rehash(hk_ptr dest_key, chk_ptr hk)
{
    assert(dest_key != NULL);
    assert(hk != NULL);

    Hk_REHASH(dest_key, hk);
}

void Hk_InplaceRehash(hk_ptr hk)
{
    assert(hk != NULL);

    Hk_INPLACE_REHASH(hk);
}

/************************************************************
 * 
 *  Debug functions; these shouldn't be needed in production code.
 *
 ************************************************************/

void Hkf_FillExact(hk_ptr dest_key, const char *hash_string)
{
    char hash[32];

#if(PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    int i;
    for(i = 0; i < 32; ++i) 
	hash[32 - 1 - i] = hash_string[i];
#else
    int i;
    for(i = 0; i < 32; ++i) 
	hash[i] = hash_string[i];
#endif

    assert(dest_key != NULL);

    static char map[256];

#ifdef DEBUG_MODE
    memset(map, 255, 256*sizeof(char));
#endif

    map['0'] = 0;
    map['1'] = 1;
    map['2'] = 2;
    map['3'] = 3;
    map['4'] = 4;
    map['5'] = 5;
    map['6'] = 6;
    map['7'] = 7;
    map['8'] = 8;
    map['9'] = 9;
    map['a'] = 10;
    map['b'] = 11;
    map['c'] = 12;
    map['d'] = 13;
    map['e'] = 14;
    map['f'] = 15;
    map['A'] = 10;
    map['B'] = 11;
    map['C'] = 12;
    map['D'] = 13;
    map['E'] = 14;
    map['F'] = 15;

    int hi;

    for(hi = 0; hi < H_NUM_8BIT_COMPONENTS; ++hi)
    {
	int idx1 = 2*hi;
	int idx2 = 2*hi + 1;
	
	ASSERTM(map[(size_t)hash[idx1]] != 255, "Non-hexadecimal character encountered.");
	ASSERTM(map[(size_t)hash[idx2]] != 255, "Non-hexadecimal character encountered.");

	dest_key->hk8[hi] = (map[(size_t)hash[idx1]]) + ((map[(size_t)hash[idx2]]) << 4);
    }
}

void Hkf_FillFromComponents(hk_ptr dest_key, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    assert(dest_key != NULL);

    dest_key->hk32[HK32I(0)] = a;
    dest_key->hk32[HK32I(1)] = b;
    dest_key->hk32[HK32I(2)] = c;
    dest_key->hk32[HK32I(3)] = d;
}


unsigned long Hk_ExtractHashComponent(chk_ptr hk, unsigned int pos)
{
    assert(hk != NULL);

    if(pos < 4)
	return hk->hk32[HK32I(pos)];
    else
	return 0;
}

void Hk_ExtractHash(char* dest_string, chk_ptr hk)
{
    char dest[32];

    if(unlikely(hk == NULL))
    {
	dest_string[0] = 'N';
	dest_string[1] = 'U';
	dest_string[2] = 'L';
	dest_string[3] = 'L';
	memset(dest_string + 4, ' ', 28);
	return;
    }

    static const char map[16] = 
	{ '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    
    int hi;
    for(hi = 0; hi < H_NUM_8BIT_COMPONENTS; ++hi)
    {
	int idx1 = 2*hi;
	int idx2 = 2*hi + 1;

	dest[idx1] = map[getBitGroup(hk->hk8[hi], 0, 4)];
	dest[idx2] = map[getBitGroup(hk->hk8[hi], 4, 4)];
    }

#if(PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    int i;
    for(i = 0; i < 32; ++i) 
	dest_string[32 - 1 - i] = dest[i];
#else
    int i;
    for(i = 0; i < 32; ++i) 
	dest_string[i] = dest[i];
#endif


}

void Hk_debug_PrintHash(chk_ptr hk)
{
    char s[33];

    Hk_ExtractHash(s, hk);
    s[32] = '\0';
    printf("%s\n", s);
}

unsigned long Hk_GetPrimeOffset()
{
    return H_HASHKEY_PRIME_OFFSET;
}
