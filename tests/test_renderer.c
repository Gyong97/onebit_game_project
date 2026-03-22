/**
 * @file test_renderer.c
 * @brief Unit tests for the renderer interface (Phase 1).
 *
 * Tests follow the TDD convention: each test_* function returns 0 on pass
 * and -1 on failure, printing a diagnostic before returning.
 */
#include <stdio.h>
#include <string.h>
#include "renderer.h"

/* ── Minimal test framework ───────────────────────────────────────────── */
#define TEST_ASSERT(cond, msg)                                        \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__);\
            return -1;                                                \
        }                                                             \
    } while (0)

/* ── Test cases ───────────────────────────────────────────────────────── */

static int test_renderer_init_returns_zero(void)
{
    int result = renderer_init();
    TEST_ASSERT(result == 0, "renderer_init() should return 0 on success");
    renderer_destroy();
    return 0;
}

static int test_renderer_draw_null_returns_error(void)
{
    int result;
    renderer_init();
    result = renderer_draw(NULL);
    TEST_ASSERT(result < 0, "renderer_draw(NULL) should return a negative error code");
    renderer_destroy();
    return 0;
}

static int test_renderer_draw_valid_frame_returns_zero(void)
{
    render_frame_t frame;
    int row;
    int col;
    int result;

    renderer_init();

    for (row = 0; row < MAP_HEIGHT; row++) {
        for (col = 0; col < MAP_WIDTH; col++) {
            if (row == 0 || row == MAP_HEIGHT - 1 ||
                col == 0 || col == MAP_WIDTH - 1) {
                frame.tiles[row][col] = TILE_WALL;
            } else {
                frame.tiles[row][col] = TILE_FLOOR;
            }
        }
    }
    frame.tiles[1][1]   = TILE_PLAYER;
    frame.player_hp     = 100;
    frame.player_max_hp = 100;
    frame.player_atk    = 10;
    frame.message[0]    = '\0';

    result = renderer_draw(&frame);
    TEST_ASSERT(result == 0, "renderer_draw() with valid frame should return 0");
    renderer_destroy();
    return 0;
}

static int test_renderer_clear_returns_zero(void)
{
    int result;
    renderer_init();
    result = renderer_clear();
    TEST_ASSERT(result == 0, "renderer_clear() should return 0 on success");
    renderer_destroy();
    return 0;
}

/* ── Test runner ──────────────────────────────────────────────────────── */

typedef struct {
    const char *name;
    int (*fn)(void);
} test_case_t;

int main(void)
{
    int i;
    int failed = 0;

    test_case_t tests[] = {
        { "test_renderer_init_returns_zero",         test_renderer_init_returns_zero         },
        { "test_renderer_draw_null_returns_error",   test_renderer_draw_null_returns_error   },
        { "test_renderer_draw_valid_frame_returns_zero", test_renderer_draw_valid_frame_returns_zero },
        { "test_renderer_clear_returns_zero",        test_renderer_clear_returns_zero        },
    };

    printf("=== Renderer Tests ===\n");
    for (i = 0; i < (int)(sizeof(tests) / sizeof(tests[0])); i++) {
        if (tests[i].fn() == 0) {
            printf("  [PASS] %s\n", tests[i].name);
        } else {
            printf("  [FAIL] %s\n", tests[i].name);
            failed++;
        }
    }

    if (failed == 0) {
        printf("All %d tests passed.\n", (int)(sizeof(tests) / sizeof(tests[0])));
        return 0;
    }
    printf("%d test(s) failed.\n", failed);
    return 1;
}
