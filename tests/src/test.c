/*
 * ==============================================================================
 * File: test.c
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

#include <stdio.h>
#include <stdlib.h>

#include "xd_malloc.h"

int main() {
  int *integer = (int *)xd_malloc(sizeof(int));
  *integer = -99;
  printf("%d\n", *integer);
  xd_free(integer);
  exit(EXIT_SUCCESS);
}
