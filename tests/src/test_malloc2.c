/*
 * ==============================================================================
 * File: test_malloc2.c
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

/**
 * @brief Used for testing `xd_malloc()`:
 * - left and right fenceposts are correctly initialized
 * - arena size is multiple of `XD_ARENA_SIZE`
 * - data can be stored and retrieved from allocated block
 * - block splitting works
 */
int main() {
  void *ptr = xd_malloc(1);

  for (size_t i = 0; i < xd_block_get_size(ptr); i++) {
    *((char *)(ptr) + i) = 'x';
  }

  xd_heap_headers_dump(stdout, NULL, NULL);
  xd_free_list_headers_dump(stdout);

  for (size_t i = 0; i < xd_block_get_size(ptr); i++) {
    assert(*((char *)(ptr) + i) == 'x');
  }

  exit(EXIT_SUCCESS);
}  // main()
