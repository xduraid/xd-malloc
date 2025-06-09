/*
 * ==============================================================================
 * File: test_sbrk_protection.c
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xd_malloc.h"

/**
 * @brief Used for testing all `xd_malloc` library functions:
 * - changing the heap break from outside the `xd_malloc` library will break the
 *   allocator and functions will stop working and return `NULL`.
 * - after returning the heap break to its supposed location the allocator will
 *   start working again.
 */
int main() {
  void *ptr1 = xd_malloc(16);
  void *ptr2 = xd_calloc(1, 16);
  void *ptr3 = xd_realloc(NULL, 32);

  // functions working originally
  assert(ptr1 != NULL);
  assert(ptr2 != NULL);
  assert(ptr3 != NULL);

  // change the heap break
  sbrk(1);

  ptr1 = xd_malloc(16);
  ptr2 = xd_calloc(1, 16);
  ptr3 = xd_realloc(NULL, 32);

  // functions stop working
  assert(ptr1 == NULL);
  assert(ptr2 == NULL);
  assert(ptr3 == NULL);

  // fix the heap break
  sbrk(-1);

  ptr1 = xd_malloc(16);
  ptr2 = xd_calloc(1, 16);
  ptr3 = xd_realloc(NULL, 32);

  // functions start working after returning the heap break
  assert(ptr1 != NULL);
  assert(ptr2 != NULL);
  assert(ptr3 != NULL);

  puts("PASSED");

  exit(EXIT_SUCCESS);
}  // main()
