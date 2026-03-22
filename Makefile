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
GAME_OBJS := $(GAME_SRCS:.c=.o)

# ── Output binaries ───────────────────────────────────────────────────────────
TARGET := $(BIN_DIR)/onebit

# Each tests/test_*.c is compiled into its own binary: bin/test_*
TEST_CSRCS := $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS  := $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(TEST_CSRCS))

# ── Phony targets ─────────────────────────────────────────────────────────────
.PHONY: all test clean

# ── Build targets ─────────────────────────────────────────────────────────────
all: $(BIN_DIR) $(TARGET)

test: $(BIN_DIR) $(TEST_BINS)
	@failed=0; \
	for t in $(TEST_BINS); do \
		echo "--- Running $$t ---"; \
		$$t || failed=$$((failed + 1)); \
	done; \
	[ $$failed -eq 0 ] && echo "=== All test suites passed. ===" \
	|| (echo "=== $$failed suite(s) FAILED. ===" && exit 1)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(GAME_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Each test binary links its own .c against all library sources
$(BIN_DIR)/test_%: $(TEST_DIR)/test_%.c $(LIB_SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ── Pattern rule: compile each .c to its .o in-place ─────────────────────────
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ── Clean ─────────────────────────────────────────────────────────────────────
clean:
	rm -rf $(BIN_DIR)
	find . -name "*.o" -delete
