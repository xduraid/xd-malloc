/*
 * ==============================================================================
 * File: xd_malloc_test_utils.h
 * Author: Duraid Maihoub
 * Date: 7 June 2025
 * Description: Part of the xd-malloc project.
 * Repository: https://github.com/xduraid/xd-malloc
 * ==============================================================================
 * Copyright (c) 2025 Duraid Maihoub
 *
 * xd-malloc is distributed under the MIT License. See the LICENSE file
 * for more information.
 * ==============================================================================
 */

#ifndef XD_MALLOC_TEST_UTILS
#define XD_MALLOC_TEST_UTILS

// ========================
// Constants
// ========================

/**
 * @brief Alignment requirement for all memory blocks.
 *
 * All allocated memory block sizes must be a multiple of this value.
 */
#define XD_ALIGNMENT (8)

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
#define XD_BLOCK_HEADER_SIZE \
  (sizeof(xd_mem_block_header) - 2 * sizeof(xd_mem_block_header *))

/**
 * @brief The minimum data section size a memory block must have to be managed
 * in the free list.
 */
#define XD_MIN_ALLOC_SIZE (2 * sizeof(xd_mem_block_header *))

/**
 * @brief Used to calculate the size/state of a memory block.
 */
#define XD_STATE_MASK (0b111)

// ========================
// Types
// ========================

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
  // if it is a fencepost this part is not used at all and no memory is
  // allocated for it.
  union {
    struct {
      struct xd_mem_block_header *next;  // The next block in the free list
      struct xd_mem_block_header *prev;  // The previous block in the free list
    };
    xd_byte data[0];  // Pointer to the data section in the memory block
  };
} xd_mem_block_header;

// ========================
// Helpers
// ========================

/**
 * @brief Returns the header of a memory block from its data section address.
 *
 * @param ptr Pointer to the data section of a block.
 *
 * @return Pointer to the memory block's header.
 */
static inline xd_mem_block_header *xd_block_get_header_from_data(void *ptr) {
  return (xd_mem_block_header *)((xd_byte *)ptr - XD_BLOCK_HEADER_SIZE);
}  // xd_block_get_header_from_data()

/**
 * @brief Gets the state of a memory block from its header.
 *
 * @param header Pointer to the memory block header.
 *
 * @return The state of the memory block.
 */
static inline xd_mem_block_state xd_block_get_state(
    const xd_mem_block_header *header) {
  return (xd_mem_block_state)(header->size & XD_STATE_MASK);
}  // xd_block_get_state()

/**
 * @brief Gets the size of a memory block (excluding header, only user data).
 *
 * @param header Pointer to the memory block header.
 *
 * @return The size of the memory block.
 */
static inline size_t xd_block_get_size(const xd_mem_block_header *header) {
  return (size_t)(header->size & ~XD_STATE_MASK);
}  // xd_block_get_size()

/**
 * @brief Returns the header of the next block in memory.
 *
 * @param header Pointer to the current block's header.
 *
 * @return Pointer to the next block's header.
 */
static inline xd_mem_block_header *xd_block_get_next(
    const xd_mem_block_header *header) {
  return (xd_mem_block_header *)((xd_byte *)header + XD_BLOCK_HEADER_SIZE +
                                 xd_block_get_size(header));
}  // xd_block_get_next()

/**
 * @brief Returns the header of the previous block in memory.
 *
 * @param header Pointer to the current block's header.
 *
 * @return Pointer to the previous block's header.
 */
static inline xd_mem_block_header *xd_block_get_prev(
    const xd_mem_block_header *header) {
  return (xd_mem_block_header *)((xd_byte *)header - header->prev_size -
                                 XD_BLOCK_HEADER_SIZE);
}  // xd_block_get_prev()

#endif  // XD_MALLOC_TEST_UTILS
