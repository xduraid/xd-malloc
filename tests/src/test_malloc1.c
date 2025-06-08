/*
 * ==============================================================================
 * File: test_malloc1.c
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

#define ALLOC_COUNT (999)

/**
 * @brief Used for testing `xd_malloc()`:
 * - allocations are aligned to `XD_ALIGNMENT`
 * - allocation sizes are at least `XD_MIN_ALLOC_SIZE`
 * - allocation sizes are multiples `XD_MIN_ALLOC_SIZE`
 */
int main() {
  void *ptrs[ALLOC_COUNT];
  for (size_t i = 0; i < ALLOC_COUNT; i++) {
    ptrs[i] = xd_malloc(i + 1);

    // test alignment
    xd_mem_block_header *header = xd_block_get_header_from_data(ptrs[i]);
    assert((intptr_t)ptrs[i] % XD_ALIGNMENT == 0);
    assert(xd_block_get_size(header) >= XD_MIN_ALLOC_SIZE);
    assert(xd_block_get_size(header) % XD_ALIGNMENT == 0);
  }

  puts("PASSED");
  exit(EXIT_SUCCESS);
}  // main()
