/*
 * ==============================================================================
 * File: xd_malloc.c
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

#include "xd_malloc.h"

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
// Global Variables
// ========================

/**
 * @brief Pointer to the location of the heap prior to any sbrk calls.
 */
static void *xd_heap_start_address = NULL;

/**
 * @brief Pointer to the head of the free list.
 */
static xd_mem_block_header *xd_free_list_head = NULL;

// ========================
// Static Variables
// ========================

/**
 * @brief Pointer to the right fencepost of the most recently created heap
 * chunk.
 *
 * Used when coalescing heap chunks.
 */
static xd_mem_block_header *xd_recent_chunk_right_fencepost = NULL;

/**
 * @brief Mutex to ensure thread safety.
 */
static pthread_mutex_t xd_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

// ========================
// Function Declarations
// ========================

// constructor/destructor for the library
static void xd_malloc_init() __attribute__((constructor));
static void xd_malloc_destroy() __attribute__((destructor));

// helpers

static inline xd_mem_block_header *xd_block_get_header_from_data(void *ptr);
static inline void xd_block_set_size(xd_mem_block_header *header, size_t size);
static inline void xd_block_set_state(xd_mem_block_header *header,
                                      xd_mem_block_state state);
static inline void xd_block_set_size_and_state(xd_mem_block_header *header,
                                               size_t size,
                                               xd_mem_block_state state);
static inline xd_mem_block_state xd_block_get_state(
    const xd_mem_block_header *header);
static inline size_t xd_block_get_size(const xd_mem_block_header *header);

static inline xd_mem_block_header *xd_block_get_next(
    const xd_mem_block_header *header);
static inline xd_mem_block_header *xd_block_get_prev(
    const xd_mem_block_header *header);

static void xd_block_split(xd_mem_block_header *header, size_t size);
static void xd_block_coalesce_with_prev_and_next(xd_mem_block_header *header);
static void xd_block_coalesce_with_prev(xd_mem_block_header *header);
static void xd_block_coalesce_with_next(xd_mem_block_header *header);

static void xd_free_list_insert(xd_mem_block_header *header);
static void xd_free_list_remove(xd_mem_block_header *header);

static xd_mem_block_header *xd_free_list_find(size_t size);

static void *xd_heap_chunk_create(size_t size);
static bool xd_heap_chunk_try_coalesce(xd_mem_block_header *chunk_header);

static inline uintptr_t xd_block_header_relative_address(
    xd_mem_block_header *header);
static inline void xd_block_header_dump(FILE *out, xd_mem_block_header *header);

// ========================
// Function Implementations
// ========================

/**
 * @brief Constructor to be executed before `main()` to initialize the
 * `xd_malloc` library.
 */
static void xd_malloc_init() {
  // initialize the free list
  xd_free_list_head = NULL;

  // initialize the mutex
  if (pthread_mutex_init(&xd_malloc_mutex, NULL) != 0) {
    perror("fatal - mutex init failed");
    exit(EXIT_FAILURE);
  }

  // disable stdout buffer so it won't call malloc
  setvbuf(stdout, NULL, _IONBF, 0);

  // store the start adress of the heap
  xd_heap_start_address = sbrk(0);
  if ((intptr_t)xd_heap_start_address % XD_ALIGNMENT != 0) {
    fprintf(stderr, "xd_malloc: heap start is not 8-byte aligned\n");
    exit(EXIT_FAILURE);
  }
}  // xd_malloc_init()

/**
 * @brief Destructor to be executed on exit to cleanup.
 */
static void xd_malloc_destroy() {
  pthread_mutex_destroy(&xd_malloc_mutex);
}  // xd_malloc_destroy()

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
 * @brief Sets the size of a memory block header.
 *
 * @param header Pointer to the memory block header.
 *
 * @param size Size of the block's data (in bytes).
 */
static inline void xd_block_set_size(xd_mem_block_header *header, size_t size) {
  header->size = size | (header->size & XD_STATE_MASK);
}  // xd_block_set_size()

/**
 * @brief Sets the state of a memory block header.
 *
 * @param header Pointer to the memory block header.
 *
 * @param state Allocation state of the block.
 */
static inline void xd_block_set_state(xd_mem_block_header *header,
                                      xd_mem_block_state state) {
  header->size = (header->size & ~XD_STATE_MASK) | state;
}  // xd_block_set_state()

/**
 * @brief Sets the size and state of a memory block from its header.
 *
 * @param header Pointer to the memory block header.
 *
 * @param size Size of the block's data (in bytes).
 * @param state Allocation state of the block.
 */
static inline void xd_block_set_size_and_state(xd_mem_block_header *header,
                                               size_t size,
                                               xd_mem_block_state state) {
  header->size = (size & ~XD_STATE_MASK) | (state & XD_STATE_MASK);
}  // xd_block_set_size_and_state()

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

/**
 * @brief Splits the block pointed to by the passed header into two blocks,
 * making the first block with the passed required size, and the second block
 * with the rest of the block size.
 *
 * @param header Pointer to the block's header.
 * @param size The required size of the first block after split.
 *
 * @note this function is a helpr for `xd_malloc()` and must be used only on
 * unallocated memory blocks.
 */
static void xd_block_split(xd_mem_block_header *header, size_t size) {
  // get the size of the block before split
  size_t block_size = xd_block_get_size(header);

  // shrink the size of the block and mark it as allocated
  xd_block_set_size_and_state(header, size, XD_MEM_BLOCK_UNALLOCATED);

  // create a new free block with the rest of the size
  xd_mem_block_header *new_block = xd_block_get_next(header);
  size_t new_block_size = block_size - size - XD_BLOCK_HEADER_SIZE;
  xd_block_set_size_and_state(new_block, new_block_size,
                              XD_MEM_BLOCK_UNALLOCATED);
  new_block->prev_size = size;
  xd_free_list_insert(new_block);

  // update the previous size of the block on the right of the new block
  xd_mem_block_header *new_block_next = xd_block_get_next(new_block);
  new_block_next->prev_size = new_block_size;
}  // xd_block_split()

/**
 * @brief Coalesce the memory block pointed to by the passed header with both
 * the block before it and the block after it in memory.
 *
 * @param header Pointer to the block's header to be coalesced.
 *
 * @note This function is a helper for `xd_free()` and must be called only on a
 * block when both the blocks before it and after it are unallocated.
 */
static void xd_block_coalesce_with_prev_and_next(xd_mem_block_header *header) {
  xd_mem_block_header *prev = xd_block_get_prev(header);
  xd_mem_block_header *next = xd_block_get_next(header);
  size_t size = xd_block_get_size(header) + xd_block_get_size(prev) +
                xd_block_get_size(next) + (2 * XD_BLOCK_HEADER_SIZE);
  xd_free_list_remove(next);
  header = prev;
  xd_block_set_size_and_state(header, size, XD_MEM_BLOCK_UNALLOCATED);
  next = xd_block_get_next(header);
  next->prev_size = size;
}  // xd_block_coalesce_with_prev_and_next()

/**
 * @brief Coalesce the memory block pointed to by the passed header with the
 * block before it.
 *
 * @param header Pointer to the block's header to be coalesced.
 *
 * @note This function is a helper for `xd_free()` and must be called only on a
 * block when the block before it is unallocated.
 */
static void xd_block_coalesce_with_prev(xd_mem_block_header *header) {
  xd_mem_block_header *prev = xd_block_get_prev(header);
  size_t size = xd_block_get_size(header) + xd_block_get_size(prev) +
                XD_BLOCK_HEADER_SIZE;
  header = prev;
  xd_block_set_size_and_state(header, size, XD_MEM_BLOCK_UNALLOCATED);
  xd_mem_block_header *next = xd_block_get_next(header);
  next->prev_size = size;
}  // xd_block_coalesce_with_prev()

/**
 * @brief Coalesce the memory block pointed to by the passed header with the
 * block after it.
 *
 * @param header Pointer to the block's header to be coalesced.
 *
 * @note This function is a helper for `xd_free()` and must be called only on a
 * block when the block after it is unallocated.
 */
static void xd_block_coalesce_with_next(xd_mem_block_header *header) {
  xd_mem_block_header *next = xd_block_get_next(header);
  size_t size = xd_block_get_size(header) + xd_block_get_size(next) +
                XD_BLOCK_HEADER_SIZE;
  xd_block_set_size_and_state(header, size, XD_MEM_BLOCK_UNALLOCATED);
  header->prev = next->prev;
  header->next = next->next;
  if (header->prev != NULL) {
    header->prev->next = header;
  }
  if (header->next != NULL) {
    header->next->prev = header;
  }
  if (next == xd_free_list_head) {
    xd_free_list_head = header;
  }
  next = xd_block_get_next(header);
  next->prev_size = size;
}  // xd_block_coalesce_with_next()

/**
 * @brief Inserts the passed memory block header at the beginning of the free
 * list.
 *
 * @param header A pointer to the memory block header to be inserted.
 */
static void xd_free_list_insert(xd_mem_block_header *header) {
  header->prev = NULL;
  header->next = xd_free_list_head;

  if (xd_free_list_head != NULL) {
    xd_free_list_head->prev = header;
  }

  xd_free_list_head = header;
}  // xd_free_list_insert()

/**
 * @brief Removes the passed memory block header from the free list.
 *
 * @param header A pointer to the memory block header to be removed.
 */
static void xd_free_list_remove(xd_mem_block_header *header) {
  if (header->prev != NULL) {
    header->prev->next = header->next;
  }
  if (header->next != NULL) {
    header->next->prev = header->prev;
  }

  if (header == xd_free_list_head) {
    xd_free_list_head = xd_free_list_head->next;
  }
}  // xd_free_list_remove()

/**
 * @brief Searches the free list for a block that can satisfy the requested size
 * and returns its header using First-Fit by default or Best-Fit if
 * `XD_USE_BEST_FIT` is defined.
 *
 * @param size The requested size in bytes.
 *
 * @return A pointer to the header of a suitable free block, or `NULL` if no
 * such block exists.
 */
static xd_mem_block_header *xd_free_list_find(size_t size) {
#ifdef XD_USE_BEST_FIT
  xd_mem_block_header *header = xd_free_list_head;
  xd_mem_block_header *best_header = NULL;
  while (header != NULL) {
    if (xd_block_get_size(header) >= size) {
      if (best_header == NULL ||
          xd_block_get_size(header) < xd_block_get_size(best_header)) {
        best_header = header;
      }
    }
    header = header->next;
  }
  return best_header;
#else
  xd_mem_block_header *header = xd_free_list_head;
  while (header != NULL && xd_block_get_size(header) < size) {
    header = header->next;
  }
  return header;
#endif
}  // xd_free_list_find()

/**
 * @brief Requests a heap chunk from the OS and initializes it with fenceposts
 * and a free block.
 *
 * The new chunk includes space for 2 fenceposts and a block header.
 *
 * @param size The required size of the usable data block in bytes.
 *
 * @return A pointer to the free block header on success, or `NULL` on
 * failure.
 */
static void *xd_heap_chunk_create(size_t size) {
  // ensure enough space for header and two fenceposts (left + right)
  size += 3 * XD_BLOCK_HEADER_SIZE;

  // roundup to multiple of XD_ARENA_SIZE
  if (size % XD_ARENA_SIZE != 0) {
    size += XD_ARENA_SIZE - (size % XD_ARENA_SIZE);
  }

  // increase heap size (request the chunk)
  void *chunk = sbrk((intptr_t)size);
  if (chunk == (void *)-1 || (intptr_t)chunk % XD_ALIGNMENT != 0) {
    return NULL;
  }

  // clean block size (data section)
  size -= 3 * XD_BLOCK_HEADER_SIZE;

  // create the left fencepost
  xd_mem_block_header *left_fencepost = (xd_mem_block_header *)chunk;
  xd_block_set_size_and_state(left_fencepost, 0, XD_MEM_BLOCK_FENCEPOST);
  left_fencepost->prev_size = 0;

  // create the free block
  xd_mem_block_header *chunk_header = xd_block_get_next(left_fencepost);
  xd_block_set_size_and_state(chunk_header, size, XD_MEM_BLOCK_UNALLOCATED);
  chunk_header->prev_size = 0;

  // create the right fencepost
  xd_mem_block_header *right_fencepost = xd_block_get_next(chunk_header);
  xd_block_set_size_and_state(right_fencepost, 0, XD_MEM_BLOCK_FENCEPOST);
  right_fencepost->prev_size = size;

  return chunk_header;
}  // xd_heap_chunk_create()

/**
 * @brief Attempts to coalesce a new heap chunk with the chunk created before
 * it.
 *
 * @param chunk_header A pointer to the heap chunk (initialized as free block).
 *
 * @return `true` on success, `false` otherwise.
 */
static bool xd_heap_chunk_try_coalesce(xd_mem_block_header *chunk_header) {
  // this is the first allocated chunk, can't coalesce
  if (xd_recent_chunk_right_fencepost == NULL) {
    return false;
  }

  xd_mem_block_header *left_fencepost = xd_block_get_prev(chunk_header);
  xd_mem_block_header *prev_chunk_right_fencepost =
      xd_block_get_prev(left_fencepost);

  // the recent chunk is not adjacent to the new chunk, can't coalesce
  if (prev_chunk_right_fencepost != xd_recent_chunk_right_fencepost) {
    return false;
  }

  size_t chunk_size = xd_block_get_size(chunk_header);

  // get the last block in the previous chunk
  xd_mem_block_header *prev_chunk_last_block =
      xd_block_get_prev(prev_chunk_right_fencepost);

  if (xd_block_get_state(prev_chunk_last_block) == XD_MEM_BLOCK_UNALLOCATED) {
    // last block is unallocated, coalesce with the block
    // remove the fenceposts
    chunk_header = prev_chunk_last_block;
    chunk_size +=
        xd_block_get_size(prev_chunk_last_block) + (3 * XD_BLOCK_HEADER_SIZE);

    // remove the block from list to be re-inserted at the beginning
    xd_free_list_remove(chunk_header);
  }
  else {
    // last block is allocated, just remove the fenceposts
    chunk_header = prev_chunk_right_fencepost;
    chunk_size += 2 * XD_BLOCK_HEADER_SIZE;

    chunk_header->prev_size = xd_block_get_size(prev_chunk_last_block);
  }

  // initialize the header after coalescing
  xd_block_set_size_and_state(chunk_header, chunk_size,
                              XD_MEM_BLOCK_UNALLOCATED);

  // update the right fencepost meta data
  xd_mem_block_header *right_fencepost = xd_block_get_next(chunk_header);
  right_fencepost->prev_size = chunk_size;
  xd_recent_chunk_right_fencepost = right_fencepost;

  // insert the coalesced block into the free list
  xd_free_list_insert(chunk_header);

  // colaescing succeeded
  return true;
}  // xd_heap_chunk_try_coalesce()

// ========================
// non-static functions
// ========================

void *xd_malloc(size_t size) {
  if (size == 0) {
    return NULL;
  }

  pthread_mutex_lock(&xd_malloc_mutex);

  // make sure there is enough space for the next/prev pointers
  // to be used when the block is freed
  if (size < XD_MIN_ALLOC_SIZE) {
    size = XD_MIN_ALLOC_SIZE;
  }

  // roundup to multiple of XD_ALIGNMENT
  if (size % XD_ALIGNMENT != 0) {
    size += XD_ALIGNMENT - (size % XD_ALIGNMENT);
  }

  // find the first block in the free list with the required size
  xd_mem_block_header *block_header = xd_free_list_find(size);
  if (block_header == NULL) {
    // no block with enough size was found, get more heap memory from the OS
    xd_mem_block_header *chunk_header = xd_heap_chunk_create(size);

    // out-of-memory failure
    if (chunk_header == NULL) {
      errno = ENOMEM;
      pthread_mutex_unlock(&xd_malloc_mutex);
      return NULL;
    }

    // coalesce or insert to free list
    if (!xd_heap_chunk_try_coalesce(chunk_header)) {
      xd_free_list_insert(chunk_header);
      xd_recent_chunk_right_fencepost = xd_block_get_next(chunk_header);
    }

    block_header = xd_free_list_find(size);
  }

  // remove the block from the free list and get its size
  xd_free_list_remove(block_header);
  size_t block_size = xd_block_get_size(block_header);

  if (block_size - size >= sizeof(xd_mem_block_header)) {
    // block size is enough to be split
    xd_block_split(block_header, size);
  }

  xd_block_set_state(block_header, XD_MEM_BLOCK_ALLOCATED);

  pthread_mutex_unlock(&xd_malloc_mutex);
  return (void *)block_header->data;
}  // xd_malloc()

void xd_free(void *ptr) {
  if (ptr == NULL) {
    return;
  }
  pthread_mutex_lock(&xd_malloc_mutex);

  xd_mem_block_header *header = xd_block_get_header_from_data(ptr);

  // double free is fatal abort
  if (xd_block_get_state(header) == XD_MEM_BLOCK_UNALLOCATED) {
    pthread_mutex_unlock(&xd_malloc_mutex);
    fprintf(stderr, "xd_free(): double free detected\n");
    abort();
  }

  // get previous and next blocks
  xd_mem_block_header *prev = xd_block_get_prev(header);
  xd_mem_block_header *next = xd_block_get_next(header);
  xd_mem_block_state prev_state = xd_block_get_state(prev);
  xd_mem_block_state next_state = xd_block_get_state(next);

  // coalesce with previous and/or next block if possible
  if (prev_state == XD_MEM_BLOCK_UNALLOCATED &&
      next_state == XD_MEM_BLOCK_UNALLOCATED) {
    xd_block_coalesce_with_prev_and_next(header);
  }
  else if (prev_state == XD_MEM_BLOCK_UNALLOCATED) {
    xd_block_coalesce_with_prev(header);
  }
  else if (next_state == XD_MEM_BLOCK_UNALLOCATED) {
    xd_block_coalesce_with_next(header);
  }
  else {
    xd_block_set_state(header, XD_MEM_BLOCK_UNALLOCATED);
    xd_free_list_insert(header);
  }

  pthread_mutex_unlock(&xd_malloc_mutex);
}  // xd_free()

void *xd_calloc(size_t n, size_t size) {
  if (n == 0 || size == 0) {
    return NULL;
  }
  if (SIZE_MAX / n < size) {
    return NULL;
  }
  size_t total_size = n * size;
  void *ptr = xd_malloc(total_size);
  if (ptr == NULL) {
    return NULL;
  }
  memset(ptr, 0, total_size);
  return ptr;
}  // xd_calloc()

void *xd_realloc(void *ptr, size_t size) {
  if (size == 0) {
    xd_free(ptr);
    return NULL;
  }
  if (ptr == NULL) {
    return xd_malloc(size);
  }

  xd_mem_block_header *header = xd_block_get_header_from_data(ptr);
  size_t old_size = xd_block_get_size(header);

  // TODO: Optimization

  // allocate-copy-free
  void *new_ptr = xd_malloc(size);
  if (new_ptr == NULL) {
    return NULL;
  }
  memcpy(new_ptr, ptr, old_size);
  xd_free(ptr);
  return new_ptr;
}  // xd_realloc()

void xd_heap_headers_dump(FILE *out, void *start, void *end) {
  if (start == NULL) {
    start = xd_heap_start_address;
  }
  if (end == NULL) {
    end = sbrk(0);
  }
  xd_mem_block_header *header = (xd_mem_block_header *)start;
  while (header != NULL && (void *)header < end) {
    xd_block_header_dump(out, header);
    header = xd_block_get_next(header);
    if (header != NULL) {
      fprintf(out, "-----------------\n");
    }
  }
}  // xd_heap_headers_dump()

void xd_free_list_headers_dump(FILE *out) {
  xd_mem_block_header *header = xd_free_list_head;
  while (header != NULL) {
    xd_block_header_dump(out, header);
    header = header->next;
    if (header != NULL) {
      fprintf(out, "-----------------\n");
    }
  }
}  // xd_free_list_headers_dump()

// ========================
// Debug/Test Functions
// ========================

/**
 * @brief Calculates the relative address of a memory block header from the
 * start of the heap.
 *
 * @param header Pointer to the memory block header.
 *
 * @return The offset of the header from the start of the heap.
 */
static inline uintptr_t xd_block_header_relative_address(
    xd_mem_block_header *header) {
  return (uintptr_t)((xd_byte *)header - (xd_byte *)xd_heap_start_address);
}  // xd_block_header_relative_address()

/**
 * @brief Dumps the contents of a memory block header to the passed output
 * stream.
 *
 * @param out Pointer to the output file stream.
 * @param header Pointer to the memory block header to be dumped.
 */
static inline void xd_block_header_dump(FILE *out,
                                        xd_mem_block_header *header) {
  if (header == NULL) {
    fprintf(out, "[NULL]");
    return;
  }

  switch (xd_block_get_state(header)) {
    case XD_MEM_BLOCK_UNALLOCATED:
      fprintf(out, "[UNALLOCATED]\n");
      break;
    case XD_MEM_BLOCK_ALLOCATED:
      fprintf(out, "[ALLOCATED]\n");
      break;
    case XD_MEM_BLOCK_FENCEPOST:
      fprintf(out, "[FENCEPOST]\n");
      break;
    default:
      fprintf(out, "[INVALID BLOCK]\n");
      break;
  }
  fprintf(out, "  address:   %" PRIuPTR "\n",
          (uintptr_t)((xd_byte *)header - (xd_byte *)xd_heap_start_address));
  fprintf(out, "  size:      %zu\n", xd_block_get_size(header));
  fprintf(out, "  prev_size: %zu\n", header->prev_size);

  if (xd_block_get_state(header) == XD_MEM_BLOCK_UNALLOCATED) {
    if (header->prev == NULL) {
      fprintf(out, "  prev:   NULL\n");
    }
    else {
      fprintf(out, "  prev:  %" PRIuPTR "\n",
              xd_block_header_relative_address(header->prev));
    }
    if (header->next == NULL) {
      fprintf(out, "  next:   NULL\n");
    }
    else {
      fprintf(out, "  next:  %" PRIuPTR "\n",
              xd_block_header_relative_address(header->next));
    }
  }
}  // xd_block_header_dump()
