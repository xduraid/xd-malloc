/*
 * ==============================================================================
 * File: test_malloc3.c
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
 * - arena size is multiple of `XD_ARENA_SIZE`
 * - arena coalescing works correctly
 */
int main() {
  xd_malloc(16);
  xd_malloc(4017);

  xd_heap_headers_dump(stdout, NULL, NULL);
  xd_free_list_headers_dump(stdout);

  exit(EXIT_SUCCESS);
}  // main()
