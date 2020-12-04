/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if bswap_16 is available. */
/* #undef HAVE_BSWAP_16 */

/* Define to 1 if bswap_32 is available. */
/* #undef HAVE_BSWAP_32 */

/* Define to 1 if bswap_64 is available. */
/* #undef HAVE_BSWAP_64 */

/* Define to 1 if you have the <byteswap.h> header file. */
/* #undef HAVE_BYTESWAP_H */

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <sys/byteorder.h> header file. */
/* #undef HAVE_SYS_BYTEORDER_H */

/* Define to 1 if you have the <sys/endian.h> header file. */
/* #undef HAVE_SYS_ENDIAN_H */

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* Define to 1 if the GNU C extension __builtin_assume_aligned is supported.
   */
#define HAVE___BUILTIN_ASSUME_ALIGNED 1

/* Define to 1 if the GNU C extensions __builtin_bswap16/32/64 are supported.
   */
#define HAVE___BUILTIN_BSWAPXX 1

/* Define to 1 if the system supports fast unaligned access to 16-bit and
   32-bit integers. */
#define TUKLIB_FAST_UNALIGNED_ACCESS 1

/* Define to 1 to use unsafe type punning, e.g. char *x = ...; *(int *)x =
   123; which violates strict aliasing rules and thus is undefined behavior
   and might result in broken code. */
/* #undef TUKLIB_USE_UNSAFE_TYPE_PUNNING */


/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
#endif
