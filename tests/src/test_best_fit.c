/*
 * ==============================================================================
 * File: test_best_fit.c
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
 * @brief Used for testing `xd_malloc()` when using the Best-Fit allocation
 * policy.
 *
 * @note This program must be compiled with when compiling with
 * `-DXD_USE_BEST_FIT` in order for the test to work correctly.
 */
int main() {
  xd_malloc(16);
  void *ptr2 = xd_malloc(128);
  xd_malloc(16);
  void *ptr4 = xd_malloc(32);
  xd_malloc(16);

  xd_free(ptr4);
  xd_free(ptr2);

  void *ptr = xd_malloc(32);
  assert(ptr == ptr4);

  puts("PASSED");
  exit(EXIT_SUCCESS);
}  // main()
