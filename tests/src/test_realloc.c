/*
 * ==============================================================================
 * File: test_realloc.c
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
 * @brief Used for testing `xd_realloc(B, 64)` in the following heap scenario:
 * - Assuming 64-bit architecture.
 * - After setup the blocks we will have:
 *   [FENCEPOST    at 0    with size 0]
 *  A[ALLOCATED    at 16   with size 16]
 *  B[ALLOCATED    at 48   with size 16]
 *  C[ALLOCATED    at 80   with size 16]
 *   [UNALLOCATED  at 112  with size 3952]
 *   [FENCEPOST    at 4080 with size 0]
 *
 * - After reallocating B, we are supposed to have:
 *   [FENCEPOST    at 0    with size 0]
 *   [ALLOCATED    at 16   with size 16]
 *   [UNALLOCATED  at 48   with size 16]
 *   [ALLOCATED    at 80   with size 16]
 *   [ALLOCATED    at 112  with size 64]
 *   [UNALLOCATED  at 192  with size 3872]
 *   [FENCEPOST    at 4080 with size 0]
 *
 * - Same is calculated for 32-bit architecture.
 */
int main() {
  // setup start
  xd_malloc(16);
  void *ptr = xd_malloc(16);
  xd_malloc(16);

  puts("");
  puts("BEFORE REALLOC");
  puts("");

  xd_heap_headers_dump(stdout, NULL, NULL);
  xd_free_list_headers_dump(stdout);
  // setup end

  xd_realloc(ptr, 64);

  puts("");
  puts("AFTER REALLOC");
  puts("");

  xd_heap_headers_dump(stdout, NULL, NULL);
  xd_free_list_headers_dump(stdout);

  exit(EXIT_SUCCESS);
}  // main()
