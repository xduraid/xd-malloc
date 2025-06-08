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
BUILD_DIR = build
LIB_DIR = lib

CC = gcc
CC_FLAGS = -std=gnu11
CC_DEP_FLAGS = -MMD -MP
CC_WARN_FLAGS = -Wall -Wextra -Werror
CC_DEBUG_FLAGS = -g -O0 -DDEBUG
CC_RELEASE_FLAGS = -O2
CC_INC_FLAGS = -I$(INCLUDE_DIR)

AR = ar
AR_FLAGS = rcs

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TARGET = $(LIB_DIR)/libxd_malloc.a

DEPS = $(OBJS:.o=.d)
-include $(DEPS)

.SUFFIXES:
.SECONDARY:
.PHONY: all rebuild release debug clean deep_clean run_tests help

all: release

$(TARGET): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(AR) $(AR_FLAGS) $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CC_FLAGS) $(CC_WARN_FLAGS) $(CC_INC_FLAGS) $(CC_DEP_FLAGS) -c $< -o $@

rebuild: deep_clean all

release: CC_FLAGS += $(CC_RELEASE_FLAGS)
release: deep_clean $(TARGET)

debug: CC_FLAGS += $(CC_DEBUG_FLAGS)
debug: deep_clean $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

deep_clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)

# run all tests
run_tests:
	$(MAKE) $@ -C ./tests

help:
	@echo "Available targets:"
	@echo "  all         - Build the static library file (default: release)"
	@echo "  rebuild     - Clean and rebuild"
	@echo "  release     - Build with release flags"
	@echo "  debug       - Build with debug flags"
	@echo "  clean       - Remove intermediate build artifacts"
	@echo "  deep_clean  - Remove all generated files"
	@echo "  run_tests   - Run all tests"
	@echo "  help        - Show this message"
