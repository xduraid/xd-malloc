#!/bin/bash

#
#  ==============================================================================
#  File: run_test.sh
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
COLOR_FG_MAGENTA="\x1b[95m" # Magenta foreground
COLOR_RESET="\x1b[0m"       # Reset all colors

mkdir -p "./output"

test_executable="$1"
test_name="$(basename $test_executable)"
output_file="./output/$test_name.txt"
expected_output_file="./expected_output/$test_name.txt"

echo -en "Running $COLOR_FG_MAGENTA$test_name$COLOR_RESET: "

if [ ! -f "$test_executable" ]; then
  echo -e $COLOR_FG_RED"File '$test_executable' is not found!"
  exit 1
fi

if [ ! -x "$test_executable" ]; then
  echo -e $COLOR_FG_RED"File '$test_executable' cannot be executed!"
  exit 1
fi

if [ ! -f "$expected_output_file" ]; then
  echo -e $COLOR_FG_RED"Expected output file '$expected_output_file' is not found!"
  exit 1
fi


./"$test_executable" > "$output_file"
if ! diff "$output_file" "$expected_output_file" >& /dev/null ; then
  echo -e $COLOR_FG_RED"FAILED"$COLOR_RESET
  exit 1
fi

echo -e $COLOR_FG_GREEN"PASSED"$COLOR_RESET
exit 0
