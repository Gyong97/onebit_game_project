# ─────────────────────────────────────────────────────────────────────────────
# Makefile — OneBit Roguelike
# Targets:  all (default), test, clean
# ─────────────────────────────────────────────────────────────────────────────

CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -Iinclude
LDFLAGS :=

# ── Directory layout ─────────────────────────────────────────────────────────
SRC_DIR      := src
LOGIC_DIR    := $(SRC_DIR)/logic
RENDERER_DIR := $(SRC_DIR)/renderer
TEST_DIR     := tests
BIN_DIR      := bin

# ── Source file lists ─────────────────────────────────────────────────────────
# Library sources (logic + renderer) — shared between game and test builds
LIB_SRCS  := $(wildcard $(LOGIC_DIR)/*.c) \
             $(wildcard $(RENDERER_DIR)/*.c)

# Game executable: entry point + library sources
GAME_SRCS := $(SRC_DIR)/main.c $(LIB_SRCS)

# Test executable: test sources + library sources (no main.c from src/)
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c) $(LIB_SRCS)

# ── Derived object file lists ─────────────────────────────────────────────────
GAME_OBJS := $(GAME_SRCS:.c=.o)
TEST_OBJS := $(TEST_SRCS:.c=.o)

# ── Output binaries ───────────────────────────────────────────────────────────
TARGET      := $(BIN_DIR)/onebit
TEST_TARGET := $(BIN_DIR)/test_runner

# ── Phony targets ─────────────────────────────────────────────────────────────
.PHONY: all test clean

# ── Build targets ─────────────────────────────────────────────────────────────
all: $(BIN_DIR) $(TARGET)

test: $(BIN_DIR) $(TEST_TARGET)
	./$(TEST_TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(GAME_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ── Pattern rule: compile each .c to its .o in-place ─────────────────────────
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ── Clean ─────────────────────────────────────────────────────────────────────
clean:
	rm -rf $(BIN_DIR)
	find . -name "*.o" -delete
