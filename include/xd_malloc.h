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

/**
 * @brief The minimum allocation size that the allocator will
 * provide.
 *
 * All requested allocation sizes are rounded up to a multiple of
 * this value.
 */
#define XD_MIN_ALLOCATION_SIZE (8)

/**
 * @brief The default size of an arena - a large contiguous block
 * of memory requested from the operating system.
 *
 * All requested arenas are rounded up to a multiple of this value.
 */
#define XD_ARENA_SIZE (4096)

/**
 * @brief The size of a memory block header (only metadata).
 */
#define XD_HEADER_SIZE \
  (sizeof(xd_mem_block_header) - 2 * sizeof(xd_mem_block_header *))

/**
 * @brief The minimum size a free block must be to be managed in the free list.
 */
#define XD_MIN_FREE_BLOCK_SIZE (2 * sizeof(xd_mem_block_header *))

/**
 * @brief Represents a single byte of memory.
 */
typedef char xd_byte;

/**
 * @brief Represents the state of a memory block.
 */
typedef enum xd_mem_block_state {
  XD_MEM_BLOCK_UNALLOCATED = 0b000,  // Unallocated memory block
  XD_MEM_BLOCK_ALLOCATED = 0b001,    // Allocated memory block
  XD_MEM_BLOCK_FENCEPOST = 0b010     // Separator between two OS chunks
} xd_mem_block_state;

/**
 * @brief Represents a memory block header, contains the metadata of the
 * memory block.
 */
typedef struct xd_mem_block_header {
  size_t size;       // The size of the block (only data excluding header).
                     // Since `MIN_ALLOCATION_SIZE` is `8` we will use the
                     // three least significant bits to store the  state of
                     // the block.
  size_t prev_size;  // The size of the previous block's data (for coalescing)

  // The start of the user's data
  // when the block is free (in the free list) `prev` and `next`
  // are used, otherwise (allocated) `data` will be used.
  union {
    struct {
      struct xd_mem_block_header *next;  // The next block in the free list
      struct xd_mem_block_header *prev;  // The previous block in the free list
    };
    xd_byte data[0];  // Pointer to the data section in the memory block
  };
} xd_mem_block_header;

/**
 * @brief Allocates a block of memory of the passed size.
 *
 * @param size The size of the memory block to be allocated (in bytes).
 *
 * @return A pointer to the allocated memory on success, or `NULL` on
 * failure.
 *
 * @note The returned allocated memory is uninitialized.
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

#endif  // XD_MALLOC_H
