/**
 * @file test_map.c
 * @brief TDD tests for the scroll-map module (Plan 2).
 *
 * Written BEFORE the real implementation — all tests should FAIL
 * when linked against the stub map.c, and pass after the real
 * implementation is in place.
 *
 * Return convention: 0 = pass, -1 = fail.
 */
#include <stdio.h>
#include <stdlib.h>  /* srand */
#include "map.h"

/* ── Minimal test framework ───────────────────────────────────────────── */
#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

/* ── Test cases ───────────────────────────────────────────────────────── */

/* map_init: left and right border columns must be TILE_WALL */
static int test_map_init_border_walls(void)
{
    map_t map;
    int r;
    tile_type_t tile;

    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");

    for (r = 0; r < VIEWPORT_H; r++) {
        TEST_ASSERT(map_get_tile(&map, 0, r, &tile) == 0,
                    "map_get_tile x=0 should succeed");
        TEST_ASSERT(tile == TILE_WALL,
                    "x=0 must be TILE_WALL after init");

        TEST_ASSERT(map_get_tile(&map, MAP_WIDTH - 1, r, &tile) == 0,
                    "map_get_tile x=MAP_WIDTH-1 should succeed");
        TEST_ASSERT(tile == TILE_WALL,
                    "x=MAP_WIDTH-1 must be TILE_WALL after init");
    }
    return 0;
}

/* map_init: interior cells must be TILE_FLOOR */
static int test_map_init_interior_floor(void)
{
    map_t map;
    int r;
    int c;
    tile_type_t tile;

    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");

    for (r = 0; r < VIEWPORT_H; r++) {
        for (c = 1; c < MAP_WIDTH - 1; c++) {
            TEST_ASSERT(map_get_tile(&map, c, r, &tile) == 0,
                        "map_get_tile interior should succeed");
            TEST_ASSERT(tile == TILE_FLOOR,
                        "interior cell must be TILE_FLOOR after init");
        }
    }
    return 0;
}

/* map_init: scroll_count must start at 0 */
static int test_map_init_scroll_count_zero(void)
{
    map_t map;
    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");
    TEST_ASSERT(map.scroll_count == 0,
                "scroll_count must be 0 after map_init");
    return 0;
}

/* map_scroll: scroll_count increments by 1 */
static int test_map_scroll_increments_count(void)
{
    map_t map;
    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");
    TEST_ASSERT(map_scroll(&map) == 0, "map_scroll should return 0");
    TEST_ASSERT(map.scroll_count == 1,
                "scroll_count must be 1 after first scroll");
    TEST_ASSERT(map_scroll(&map) == 0, "map_scroll should return 0");
    TEST_ASSERT(map.scroll_count == 2,
                "scroll_count must be 2 after second scroll");
    return 0;
}

/* map_scroll: new top row (rows[0]) must have border walls */
static int test_map_scroll_new_top_row_has_walls(void)
{
    map_t map;
    tile_type_t tile;

    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");
    TEST_ASSERT(map_scroll(&map) == 0, "map_scroll should return 0");

    TEST_ASSERT(map_get_tile(&map, 0, 0, &tile) == 0,
                "map_get_tile new top-left should succeed");
    TEST_ASSERT(tile == TILE_WALL,
                "new top row x=0 must be TILE_WALL after scroll");

    TEST_ASSERT(map_get_tile(&map, MAP_WIDTH - 1, 0, &tile) == 0,
                "map_get_tile new top-right should succeed");
    TEST_ASSERT(tile == TILE_WALL,
                "new top row x=MAP_WIDTH-1 must be TILE_WALL after scroll");
    return 0;
}

/* map_scroll: new top row interior must contain only valid terrain tiles */
static int test_map_scroll_new_top_row_interior_floor(void)
{
    map_t map;
    tile_type_t tile;
    int c;

    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");
    TEST_ASSERT(map_scroll(&map) == 0, "map_scroll should return 0");

    /* Interior cells must be terrain only — never an entity or coin */
    for (c = 1; c < MAP_WIDTH - 1; c++) {
        TEST_ASSERT(map_get_tile(&map, c, 0, &tile) == 0,
                    "map_get_tile new top interior should succeed");
        TEST_ASSERT(tile == TILE_FLOOR || tile == TILE_WALL,
                    "new top row interior must be TILE_FLOOR or TILE_WALL");
        TEST_ASSERT(tile != TILE_PLAYER && tile != TILE_MONSTER,
                    "new top row interior must not contain entities");
    }
    return 0;
}

/* map_scroll: with enough scrolls, internal obstacle walls must appear */
static int test_map_scroll_generates_obstacles(void)
{
    map_t       map;
    tile_type_t tile;
    int         s;
    int         c;
    int         found_wall;

    srand(42); /* deterministic seed */
    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");

    found_wall = 0;
    for (s = 0; s < 50 && !found_wall; s++) {
        TEST_ASSERT(map_scroll(&map) == 0, "map_scroll should return 0");
        for (c = 1; c < MAP_WIDTH - 1; c++) {
            TEST_ASSERT(map_get_tile(&map, c, 0, &tile) == 0,
                        "map_get_tile should succeed");
            if (tile == TILE_WALL) {
                found_wall = 1;
                break;
            }
        }
    }

    TEST_ASSERT(found_wall,
                "map_scroll must generate internal TILE_WALL obstacles with OBSTACLE_SPAWN_PCT > 0");
    return 0;
}

/* map_scroll: old rows shift down — row that was at rows[0] is now at rows[1] */
static int test_map_scroll_old_row_shifts_down(void)
{
    map_t map;
    tile_type_t old_tile;
    tile_type_t new_tile;

    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");

    /* Plant a sentinel tile in the interior of the current top row */
    TEST_ASSERT(map_set_tile(&map, 1, 0, TILE_CHEST) == 0,
                "map_set_tile should return 0");
    TEST_ASSERT(map_get_tile(&map, 1, 0, &old_tile) == 0,
                "map_get_tile should succeed");
    TEST_ASSERT(old_tile == TILE_CHEST, "sentinel tile must be TILE_CHEST");

    /* After scroll, the old rows[0] becomes rows[1] */
    TEST_ASSERT(map_scroll(&map) == 0, "map_scroll should return 0");
    TEST_ASSERT(map_get_tile(&map, 1, 1, &new_tile) == 0,
                "map_get_tile shifted row should succeed");
    TEST_ASSERT(new_tile == TILE_CHEST,
                "old top row must appear at rows[1] after scroll");
    return 0;
}

/* map_get_tile: NULL map should return -1 */
static int test_map_get_tile_null_map(void)
{
    tile_type_t tile;
    int result = map_get_tile(NULL, 0, 0, &tile);
    TEST_ASSERT(result == -1,
                "map_get_tile(NULL, ...) must return -1");
    return 0;
}

/* map_get_tile: out-of-bounds x should return -1 */
static int test_map_get_tile_oob_x(void)
{
    map_t map;
    tile_type_t tile;
    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");
    TEST_ASSERT(map_get_tile(&map, MAP_WIDTH, 0, &tile) == -1,
                "map_get_tile x=MAP_WIDTH must return -1");
    TEST_ASSERT(map_get_tile(&map, -1, 0, &tile) == -1,
                "map_get_tile x=-1 must return -1");
    return 0;
}

/* map_get_tile: out-of-bounds y should return -1 */
static int test_map_get_tile_oob_y(void)
{
    map_t map;
    tile_type_t tile;
    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");
    TEST_ASSERT(map_get_tile(&map, 0, VIEWPORT_H, &tile) == -1,
                "map_get_tile y=VIEWPORT_H must return -1");
    TEST_ASSERT(map_get_tile(&map, 0, -1, &tile) == -1,
                "map_get_tile y=-1 must return -1");
    return 0;
}

/* map_set_tile: valid write and read-back */
static int test_map_set_tile_valid(void)
{
    map_t map;
    tile_type_t tile;
    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");
    TEST_ASSERT(map_set_tile(&map, 3, 3, TILE_MONSTER) == 0,
                "map_set_tile should return 0");
    TEST_ASSERT(map_get_tile(&map, 3, 3, &tile) == 0,
                "map_get_tile after set should succeed");
    TEST_ASSERT(tile == TILE_MONSTER,
                "tile must equal TILE_MONSTER after map_set_tile");
    return 0;
}

/* map_set_tile: NULL map should return -1 */
static int test_map_set_tile_null_map(void)
{
    int result = map_set_tile(NULL, 0, 0, TILE_FLOOR);
    TEST_ASSERT(result == -1,
                "map_set_tile(NULL, ...) must return -1");
    return 0;
}

/* map_set_tile: out-of-bounds coordinates should return -1 */
static int test_map_set_tile_oob(void)
{
    map_t map;
    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");
    TEST_ASSERT(map_set_tile(&map, MAP_WIDTH, 0, TILE_FLOOR) == -1,
                "map_set_tile x=MAP_WIDTH must return -1");
    TEST_ASSERT(map_set_tile(&map, 0, VIEWPORT_H, TILE_FLOOR) == -1,
                "map_set_tile y=VIEWPORT_H must return -1");
    return 0;
}

/* ── Phase 4 tests ───────────────────────────────────────────────────── */

/* MAP_WIDTH == 11, MAP_HEIGHT == 20, MAP_BUFFER_H == 10, MAP_TOTAL_H == 30 */
static int test_map_dimensions(void)
{
    TEST_ASSERT(MAP_WIDTH    == 11, "MAP_WIDTH must be 11");
    TEST_ASSERT(MAP_HEIGHT   == 20, "MAP_HEIGHT must be 20");
    TEST_ASSERT(MAP_BUFFER_H == 10, "MAP_BUFFER_H must be 10");
    TEST_ASSERT(MAP_TOTAL_H  == 30, "MAP_TOTAL_H must be 30 (MAP_HEIGHT + MAP_BUFFER_H)");
    return 0;
}

/* map_get_tile must accept y up to MAP_TOTAL_H-1 (all 30 rows accessible) */
static int test_map_total_height_accessible(void)
{
    map_t       map;
    tile_type_t tile;

    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");

    /* Last row of total map must be accessible */
    TEST_ASSERT(map_get_tile(&map, 1, MAP_TOTAL_H - 1, &tile) == 0,
                "map_get_tile y=MAP_TOTAL_H-1 must succeed");

    /* One beyond must fail */
    TEST_ASSERT(map_get_tile(&map, 1, MAP_TOTAL_H, &tile) == -1,
                "map_get_tile y=MAP_TOTAL_H must return -1 (out of bounds)");
    return 0;
}

/* After scroll, new top row (row 0) must always have at least one passable cell */
static int test_map_scroll_row_always_passable(void)
{
    map_t       map;
    tile_type_t tile;
    int         s;
    int         c;
    int         has_floor;

    srand(12345);
    TEST_ASSERT(map_init(&map) == 0, "map_init should return 0");

    for (s = 0; s < 200; s++) {
        TEST_ASSERT(map_scroll(&map) == 0, "map_scroll should return 0");
        has_floor = 0;
        for (c = 1; c < MAP_WIDTH - 1; c++) {
            if (map_get_tile(&map, c, 0, &tile) == 0 && tile == TILE_FLOOR) {
                has_floor = 1;
                break;
            }
        }
        TEST_ASSERT(has_floor,
                    "new row after scroll must always have at least one TILE_FLOOR");
    }
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
        { "test_map_init_border_walls",              test_map_init_border_walls              },
        { "test_map_init_interior_floor",            test_map_init_interior_floor            },
        { "test_map_init_scroll_count_zero",         test_map_init_scroll_count_zero         },
        { "test_map_scroll_increments_count",        test_map_scroll_increments_count        },
        { "test_map_scroll_new_top_row_has_walls",   test_map_scroll_new_top_row_has_walls   },
        { "test_map_scroll_new_top_row_interior_floor", test_map_scroll_new_top_row_interior_floor },
        { "test_map_scroll_generates_obstacles",     test_map_scroll_generates_obstacles     },
        { "test_map_scroll_old_row_shifts_down",     test_map_scroll_old_row_shifts_down     },
        { "test_map_get_tile_null_map",              test_map_get_tile_null_map              },
        { "test_map_get_tile_oob_x",                 test_map_get_tile_oob_x                 },
        { "test_map_get_tile_oob_y",                 test_map_get_tile_oob_y                 },
        { "test_map_set_tile_valid",                 test_map_set_tile_valid                 },
        { "test_map_set_tile_null_map",              test_map_set_tile_null_map              },
        { "test_map_set_tile_oob",                   test_map_set_tile_oob                   },
        { "test_map_dimensions",                     test_map_dimensions                     },
        { "test_map_total_height_accessible",        test_map_total_height_accessible        },
        { "test_map_scroll_row_always_passable",     test_map_scroll_row_always_passable     },
    };

    printf("=== Map Tests ===\n");
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
