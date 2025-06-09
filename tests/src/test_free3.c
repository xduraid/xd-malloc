/*
 * ==============================================================================
 * File: test_free3.c
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

#include <stdio.h>
#include <stdlib.h>

#include "xd_malloc.h"

/**
 * @brief Used for testing `xd_free(B)` in the following heap scenario:
 * - Assuming 64-bit architecture.
 * - After allocating the blocks we will have:
 *   [FENCEPOST    at 0    with size 0]
 *  A[UNALLOCATED  at 16   with size 16]
 *  B[ALLOCATED    at 48   with size 16]
 *  C[UNALLOCATED  at 80   with size 3984]
 *   [FENCEPOST    at 4080 with size 0]
 *
 * - After freeing B, we are supposed to have:
 *   [FENCEPOST    at 0    with size 0]
 *   [UNALLOCATED  at 16   with size 4048]
 *   [FENCEPOST    at 4080 with size 0]
 *
 * - Same is calculated for 32-bit architecture.
 */
int main() {
  // setup start
  void *ptr1 = xd_malloc(16);
  void *ptr2 = xd_malloc(16);

  puts("");
  puts("BEFORE FREE");
  puts("");

  xd_free(ptr1);

  xd_heap_headers_dump(stdout, NULL, NULL);
  xd_free_list_headers_dump(stdout);
  // setup end

  xd_free(ptr2);

  puts("");
  puts("AFTER FREE");
  puts("");

  xd_heap_headers_dump(stdout, NULL, NULL);
  xd_free_list_headers_dump(stdout);

  exit(EXIT_SUCCESS);
}  // main()
