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

CC = gcc
CC_FLAGS = -std=gnu11 \
					 -g -O0 -DDEBUG \
					 -Wall -Wextra -Werror \
					 -I$(INCLUDE_DIR) -I$(MAIN_INCLUDE_DIR)

MAIN_SRCS = $(wildcard $(MAIN_SRC_DIR)/*.c)
SRCS = $(wildcard $(SRC_DIR)/*.c)
BINS = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(SRCS))

.SUFFIXES:
.SECONDARY:
.PHONY: all rebuild clean run_tests run_% help

all: $(BINS)

$(BIN_DIR)/%: $(SRC_DIR)/%.c $(MAIN_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CC_FLAGS) -o $@ $^

rebuild: clean all

clean:
	rm -rf $(BIN_DIR)

# run all tests
run_tests: $(BINS)
	@for test in $(BINS); do \
		echo "--------------------------------------------"; \
		echo ">> Running $$test..."; \
		echo "--------------------------------------------";  \
		./$$test || exit 1; \
	done; \
	echo "--------------------------------------------";  \

# run a specific test
run_%: $(BIN_DIR)/%
	@echo "--------------------------------------------"; \
	echo ">> Running $<..."; \
	echo "--------------------------------------------";  \
	./$< || exit 1; \
	echo "--------------------------------------------";  \

help:
	@echo "Available targets:"
	@echo "  all         - Build test executables"
	@echo "  rebuild     - Clean and rebuild"
	@echo "  clean       - Remove all generated files"
	@echo "  run_tests   - Run all tests"
	@echo "  run_[test]  - Run a specific test"
	@echo "  help        - Show this message"
