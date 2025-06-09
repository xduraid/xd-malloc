/*
 * ==============================================================================
 * File: test_stress.c
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
#include "xd_malloc_test_utils.h"

#define ALLOC_COUNT (100000000)

int *ptrs[ALLOC_COUNT];

/**
 * @brief Used for stress testing `xd_malloc()`.
 * - Assuming 64-bit architecture.
 * - We have 100,000,000 allocation, each of size 32 bytes (16 header + 16 data)
 * - Total bytes allocated = 100,000,000 * 32 = 3,200,000,000
 * - We also need two headers for the fenceposts each of size 16, hence total
 *   bytes allocated including the fenceposts = 3,200,000,032
 * - Arena size is 4096
 * - Number of arenas required = ceil(3,200,000,032 / 4096) = 781,251
 * - Total bytes requested from the OS = 781,251 * 4096 = 3,200,004,096
 * - Two fenceposts and block header will take 48 bytes, hence at the end we
 *   will have a single free block of size 3,200,004,096 - 48 = 3,200,004,048
 *   and the heap dump will be:
 *   [FENCEPOST   at 0 with size 0]
 *   [UNALLOCATED at 16 with size 3,200,004,048]
 *   [FENCEPOST   at 3,200,004,080 with size 0]
 * - Same is calculated for 32-bit architecture.
 */
int main() {
  for (int i = 0; i < ALLOC_COUNT; i++) {
    ptrs[i] = xd_malloc(sizeof(int));
    *ptrs[i] = i;
  }

  for (int i = 0; i < ALLOC_COUNT; i++) {
    assert(*ptrs[i] == i);
    xd_free(ptrs[i]);
  }

  xd_heap_headers_dump(stdout, NULL, NULL);
  xd_free_list_headers_dump(stdout);
}  // main()
