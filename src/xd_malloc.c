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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Used to calculate the size/state of a memory block.
 */
#define XD_STATE_MASK (0b111)

// ========================
// Global Variables
// ========================

/**
 * @brief Pointer to the location of the heap prior to any sbrk calls.
 */
void *xd_heap_start_address = NULL;

/**
 * @brief Pointer to the head of the free list.
 */
xd_mem_block_header *xd_free_list_head = NULL;

/**
 * @brief Mutex to ensure thread safety for the free list.
 */
static pthread_mutex_t xd_free_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// ========================
// Function Declarations
// ========================

// constructor for the library
static void xd_malloc_init() __attribute__((constructor));

// helpers
static inline xd_mem_block_state xd_block_get_state(
    const xd_mem_block_header *header);
static inline void xd_block_set_state(xd_mem_block_header *header,
                                      xd_mem_block_state state);
static inline size_t xd_block_get_size(const xd_mem_block_header *header);
static inline void xd_block_set_size(xd_mem_block_header *header, size_t size);

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
  pthread_mutex_init(&xd_free_list_mutex, NULL);

  // disable stdout buffer so it won't call malloc
  setvbuf(stdout, NULL, _IONBF, 0);

  // store the start adress of the heap
  xd_heap_start_address = sbrk(0);
}  // xd_malloc_init()

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
 * @brief Sets the state of a memory block.
 *
 * @param header Pointer to the memory block header.
 *
 * @param state The new state to set.
 */
static inline void xd_block_set_state(xd_mem_block_header *header,
                                      xd_mem_block_state state) {
  header->size = xd_block_get_size(header) | state;
}  // xd_block_set_state()

/**
 * @brief Gets the size of a memory block (excluding header, only user data).
 *
 * @param header Pointer to the memory block header.
 *
 * @return The size of the memory block.
 */
static inline size_t xd_block_get_size(const xd_mem_block_header *header) {
  return header->size & ~XD_STATE_MASK;
}  // xd_block_get_size()

/**
 * @brief Sets the size of a memory block (excluding header, only user data).
 *
 * @param header Pointer to the memory block header.
 *
 * @param size The size to set.
 */
static inline void xd_block_set_size(xd_mem_block_header *header, size_t size) {
  header->size = (size & ~XD_STATE_MASK) | xd_block_get_state(header);
}  // xd_block_set_size()
