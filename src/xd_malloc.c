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
