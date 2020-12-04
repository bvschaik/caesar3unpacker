/**
 * \file        lzma/vli.h
 * \brief       Variable-length integer handling
 *
 * In the .xz format, most integers are encoded in a variable-length
 * representation, which is sometimes called little endian base-128 encoding.
 * This saves space when smaller values are more likely than bigger values.
 *
 * The encoding scheme encodes seven bits to every byte, using minimum
 * number of bytes required to represent the given value. Encodings that use
 * non-minimum number of bytes are invalid, thus every integer has exactly
 * one encoded representation. The maximum number of bits in a VLI is 63,
 * thus the vli argument must be less than or equal to UINT64_MAX / 2. You
 * should use LZMA_VLI_MAX for clarity.
 */

/*
 * Author: Lasse Collin
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 *
 * See ../lzma.h for information about liblzma as a whole.
 */

#ifndef LZMA_H_INTERNAL
#	error Never include this file directly. Use <lzma.h> instead.
#endif


/**
 * \brief       Maximum supported value of a variable-length integer
 */
#define LZMA_VLI_MAX (UINT64_MAX / 2)

/**
 * \brief       VLI value to denote that the value is unknown
 */
#define LZMA_VLI_UNKNOWN UINT64_MAX

/**
 * \brief       Maximum supported encoded length of variable length integers
 */
#define LZMA_VLI_BYTES_MAX 9

/**
 * \brief       VLI constant suffix
 */
#define LZMA_VLI_C(n) UINT64_C(n)


/**
 * \brief       Variable-length integer type
 *
 * Valid VLI values are in the range [0, LZMA_VLI_MAX]. Unknown value is
 * indicated with LZMA_VLI_UNKNOWN, which is the maximum value of the
 * underlying integer type.
 *
 * lzma_vli will be uint64_t for the foreseeable future. If a bigger size
 * is needed in the future, it is guaranteed that 2 * LZMA_VLI_MAX will
 * not overflow lzma_vli. This simplifies integer overflow detection.
 */
typedef uint64_t lzma_vli;
