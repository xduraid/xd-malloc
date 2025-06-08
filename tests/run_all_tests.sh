#!/bin/bash

#
#  ==============================================================================
#  File: run_all_tests.sh
#  Author: Duraid Maihoub
#  Date: 7 June 2025
#  Description: Part of the xd-malloc project.
#  Repository: https://github.com/xduraid/xd-malloc
#  ==============================================================================
#  Copyright (c) 2025 Duraid Maihoub
#
#  xd-malloc is distributed under the MIT License. See the LICENSE file
#  for more information.
#  ==============================================================================
#

COLOR_FG_RED="\x1b[91m"     # Red foreground
COLOR_FG_GREEN="\x1b[92m"   # Green foreground
COLOR_FG_YELLOW="\x1b[93m"  # Yellow foreground
COLOR_RESET="\x1b[0m"       # Reset all colors

shopt -s nullglob
test_executables=(./bin/*)

if (( ${#test_executables[@]} == 0 )); then
  echo -e $COLOR_FG_RED"Test executables are not compiled!"$COLOR_RESET
  echo -e $COLOR_FG_RED"Cannot run the tests"$COLOR_RESET
  exit 1
fi

failed=0
echo "=========================="
echo -e $COLOR_FG_YELLOW"TESTING STARTED"$COLOR_RESET
echo "=========================="

for exec in "${test_executables[@]}"; do
  if ! ./run_test.sh "$exec"; then
    failed=$((failed + 1))
  fi
done

echo "=========================="
if [ "$failed" -eq 0 ]; then
  echo -e $COLOR_FG_GREEN"PASSED ALL TESTS"$COLOR_RESET
else
  echo -e $COLOR_FG_RED"FAILED SOME TESTS"$COLOR_RESET
fi
echo "=========================="

exit 0
