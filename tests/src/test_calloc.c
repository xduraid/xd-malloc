/*
 * ==============================================================================
 * File: test_calloc.c
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

#include "xd_malloc.h"

#define ARRAY_SIZE (10)

/**
 * @brief Used for testing `xd_calloc()`.
 * - Assuming 64-bit architecture.
 * - Allocating 10 * 4 bytes = 40 bytes.
 * - We will have:
 *   [Fencepost   at 0  with size 0]
 *   [Allocated   at 16 with size 40]
 *   [Unallocated at 72 with size 3992]
 *   [Fencepost   at 4080]
 * - After freeing the block we will have a single unallocated block with two
 *   fenceposts.
 * - Same for 32-bit architecture.
 */
int main() {
  int *arr = xd_calloc(ARRAY_SIZE, sizeof(int));

  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    arr[i] = (int)i;
  }

  xd_heap_headers_dump(stdout, NULL, NULL);
  xd_free_list_headers_dump(stdout);

  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    assert(arr[i] == (int)i);
  }

  xd_free(arr);

  xd_heap_headers_dump(stdout, NULL, NULL);
  xd_free_list_headers_dump(stdout);

  exit(EXIT_SUCCESS);
}  // main()
