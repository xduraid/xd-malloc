#
#  ==============================================================================
#  File: Makefile
#  Author: Duraid Maihoub
#  Date: 1 June 2025
#  Description: Part of the xd-malloc project.
#  Repository: https://github.com/xduraid/xd-malloc
#  ==============================================================================
#  Copyright (c) 2025 Duraid Maihoub
#
#  xd-malloc is distributed under the MIT License. See the LICENSE file
#  for more information.
#  ==============================================================================
#

SRC_DIR = src
INCLUDE_DIR = include
BIN_DIR = bin
MAIN_INCLUDE_DIR = ../include
MAIN_SRC_DIR = ../src
OUTPUT_DIR = output

CC = gcc
CC_FLAGS = -std=gnu11 \
					 -g -O0 -DDEBUG \
					 -Wall -Wextra -Werror \
					 -I$(INCLUDE_DIR) -I$(MAIN_INCLUDE_DIR)

MAIN_SRCS = $(wildcard $(MAIN_SRC_DIR)/*.c)
SRCS = $(wildcard $(SRC_DIR)/*.c)

BINS_32bit = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%_32bit, $(SRCS))
BINS_64bit = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%_64bit, $(SRCS))

.SUFFIXES:
.SECONDARY:
.PHONY: all rebuild clean run_tests run_% help

all: $(BINS_64bit) $(BINS_32bit)

$(BIN_DIR)/test_best_fit_32bit: $(SRC_DIR)/test_best_fit.c $(MAIN_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CC_FLAGS) -m32 -DXD_USE_BEST_FIT -o $@ $^

$(BIN_DIR)/test_best_fit_64bit: $(SRC_DIR)/test_best_fit.c $(MAIN_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CC_FLAGS) -m64 -DXD_USE_BEST_FIT  -o $@ $^

$(BIN_DIR)/%_32bit: $(SRC_DIR)/%.c $(MAIN_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CC_FLAGS) -m32 -o $@ $^

$(BIN_DIR)/%_64bit: $(SRC_DIR)/%.c $(MAIN_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CC_FLAGS) -m64 -o $@ $^

rebuild: clean all

clean:
	rm -rf $(BIN_DIR) $(OUTPUT_DIR)

# run all tests
run_tests: clean all
	chmod +x ./run_all_tests.sh
	@./run_all_tests.sh || true

# run a specific test
run_%: $(BIN_DIR)/%
	chmod +x ./run_test.sh
	@./run_test.sh $< || true

help:
	@echo "Available targets:"
	@echo "  all         - Build test executables"
	@echo "  rebuild     - Clean and rebuild"
	@echo "  clean       - Remove all generated files"
	@echo "  run_tests   - Run all tests"
	@echo "  run_[test]  - Run a specific test"
	@echo "  help        - Show this message"
