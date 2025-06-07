/*
 * ==============================================================================
 * File: xd_malloc.h
 * Author: Duraid Maihoub
 * Date: 1 June 2025
 * Description: Part of the xd-malloc project.
 * Repository: https://github.com/xduraid/xd-malloc
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-malloc is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_MALLOC_H
#define XD_MALLOC_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Allocates a block of memory of the passed size.
 *
 * @param size The size of the memory block to be allocated (in bytes).
 *
 * @return A pointer to the allocated memory on success, or `NULL` on
 * failure.
 *
 * @note If allocation fails due to lack of memory, `errno` is set to `ENOMEM`
 * and `NULL` is returned.
 * @note If the passed `size` is 0, `NULL` is returned.
 */
void *xd_malloc(size_t size);

/**
 * @brief Frees a previously allocated memory block.
 *
 * @param ptr The pointer to the memory block to be freed.
 *
 * @note If the passed pointer is `NULL` this function will do nothing.
 */
void xd_free(void *ptr);

/**
 * @brief Allocates a block of memory for an array of elements, and initializes
 * all the bytes in the allocated block to zero.
 *
 * @param n The number of elements in the array.
 * @param size The size of each element in the array (in bytes).
 *
 * @return A pointer to the allocated memory on success, or `NULL` on failure.
 *
 * @note If allocation fails due to lack of memory, `errno` is set to `ENOMEM`
 * and `NULL` is returned.
 * @note If the passed `n` or `size` is 0, `NULL` is returned.
 * @note If an overflow occurs (`n * size > SIZE_MAX`), `NULL` is returned.
 */
void *xd_calloc(size_t n, size_t size);

/**
 * @brief Changes the size of the memory block pointed to by passed pointer to
 * the passed new size. The contents of the memory will be unchanged in the
 * range from the start of the region up to the minimum of the old and new
 * sizes. If the new size is larger than the old size, the added memory will
 * not be initialized.
 *
 * @param ptr The pointer to the memory block to be resized.
 * @param size The new size for the memory block (in bytes).
 *
 * @return A pointer to the allocated memory on success, or `NULL` on failure.
 *
 * @note If the passed pointer is `NULL`, this function behaves like
 * `xd_malloc(size)`.
 * @note If the passed size is `0`, this function behaves like `xd_free(ptr)`
 * and returns `NULL`.
 */
void *xd_realloc(void *ptr, size_t size);

/**
 * @brief Dumps all memory block headers in a specified range of the heap to the
 * passed output stream.
 *
 * @param out Pointer to the output file stream.
 * @param start Start address of the range to dump. If `NULL`, uses heap start.
 * @param end End address of the range to dump. If `NULL`, uses current heap
 * end.
 */
void xd_heap_headers_dump(FILE *out, void *start, void *end);

/**
 * @brief Dumps all block headers currently in the free list to the passed
 * output stream.
 *
 * @param out Pointer to the output file stream.
 */
void xd_free_list_headers_dump(FILE *out);

#endif  // XD_MALLOC_H
