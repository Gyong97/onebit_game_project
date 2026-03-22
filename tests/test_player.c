/**
 * @file test_player.c
 * @brief TDD tests for the player module (Plan 2).
 *
 * Written BEFORE the real implementation — all tests should FAIL
 * when linked against the stub player.c, and pass after the real
 * implementation is in place.
 *
 * Return convention: 0 = pass, -1 = fail.
 */
#include <stdio.h>
#include "player.h"

/* ── Minimal test framework ───────────────────────────────────────────── */
#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

/* ── Helper: build a clean map for movement tests ─────────────────────── */
static int make_map(map_t *p_map)
{
    return map_init(p_map);
}

/* ── Test cases: player_init ──────────────────────────────────────────── */

static int test_player_init_hp(void)
{
    player_t p;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(p.hp == PLAYER_INIT_HP,
                "initial hp must equal PLAYER_INIT_HP");
    return 0;
}

static int test_player_init_max_hp(void)
{
    player_t p;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(p.max_hp == PLAYER_INIT_MAXHP,
                "initial max_hp must equal PLAYER_INIT_MAXHP");
    return 0;
}

static int test_player_init_atk(void)
{
    player_t p;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(p.atk == PLAYER_INIT_ATK,
                "initial atk must equal PLAYER_INIT_ATK");
    return 0;
}

static int test_player_init_position(void)
{
    player_t p;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(p.x == PLAYER_INIT_X,
                "initial x must equal PLAYER_INIT_X");
    TEST_ASSERT(p.y == PLAYER_INIT_Y,
                "initial y must equal PLAYER_INIT_Y");
    return 0;
}

static int test_player_init_null(void)
{
    TEST_ASSERT(player_init(NULL) == -1,
                "player_init(NULL) must return -1");
    return 0;
}

/* ── Test cases: player_move — basic directional movement ─────────────── */

static int test_player_move_up(void)
{
    player_t p;
    map_t map;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 4;
    p.y = 7; /* below buffer zone — move up should not trigger scroll */
    map_set_tile(&map, 4, 7, TILE_PLAYER);
    TEST_ASSERT(player_move(&p, ACTION_MOVE_UP, &map) == 0,
                "move up on floor should return 0");
    TEST_ASSERT(p.y == 6, "y must decrease by 1 after move up");
    TEST_ASSERT(p.x == 4, "x must not change after move up");
    return 0;
}

static int test_player_move_down(void)
{
    player_t p;
    map_t map;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 4;
    p.y = 5;
    TEST_ASSERT(player_move(&p, ACTION_MOVE_DOWN, &map) == 0,
                "move down on floor should return 0");
    TEST_ASSERT(p.y == 6, "y must increase by 1 after move down");
    TEST_ASSERT(p.x == 4, "x must not change after move down");
    return 0;
}

static int test_player_move_left(void)
{
    player_t p;
    map_t map;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 5;
    p.y = 5;
    TEST_ASSERT(player_move(&p, ACTION_MOVE_LEFT, &map) == 0,
                "move left on floor should return 0");
    TEST_ASSERT(p.x == 4, "x must decrease by 1 after move left");
    TEST_ASSERT(p.y == 5, "y must not change after move left");
    return 0;
}

static int test_player_move_right(void)
{
    player_t p;
    map_t map;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 5;
    p.y = 5;
    TEST_ASSERT(player_move(&p, ACTION_MOVE_RIGHT, &map) == 0,
                "move right on floor should return 0");
    TEST_ASSERT(p.x == 6, "x must increase by 1 after move right");
    TEST_ASSERT(p.y == 5, "y must not change after move right");
    return 0;
}

/* ── Test cases: wall collision ───────────────────────────────────────── */

static int test_player_blocked_by_left_wall(void)
{
    player_t p;
    map_t map;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 1; /* one step from left wall (x=0) */
    p.y = 5;
    TEST_ASSERT(player_move(&p, ACTION_MOVE_LEFT, &map) == 1,
                "move into left wall must be blocked (return 1)");
    TEST_ASSERT(p.x == 1, "x must not change when blocked by wall");
    return 0;
}

static int test_player_blocked_by_right_wall(void)
{
    player_t p;
    map_t map;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = MAP_WIDTH - 2; /* one step from right wall (x=MAP_WIDTH-1) */
    p.y = 5;
    TEST_ASSERT(player_move(&p, ACTION_MOVE_RIGHT, &map) == 1,
                "move into right wall must be blocked (return 1)");
    TEST_ASSERT(p.x == MAP_WIDTH - 2, "x must not change when blocked by wall");
    return 0;
}

/* ── Test cases: viewport boundary ───────────────────────────────────── */

static int test_player_blocked_at_viewport_bottom(void)
{
    player_t p;
    map_t map;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 4;
    p.y = VIEWPORT_H - 1; /* bottom row of viewport */
    TEST_ASSERT(player_move(&p, ACTION_MOVE_DOWN, &map) == 1,
                "move down past viewport bottom must be blocked (return 1)");
    TEST_ASSERT(p.y == VIEWPORT_H - 1, "y must not change when blocked at bottom");
    return 0;
}

/* ── Test cases: scroll trigger ───────────────────────────────────────── */

static int test_player_scroll_at_top(void)
{
    player_t p;
    map_t map;
    long prev_scroll;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 4;
    p.y = 0; /* top row of viewport */
    prev_scroll = map.scroll_count;

    TEST_ASSERT(player_move(&p, ACTION_MOVE_UP, &map) == 0,
                "move up at top must trigger scroll and return 0");
    TEST_ASSERT(p.y == 0,
                "player y must remain 0 after scroll");
    TEST_ASSERT(map.scroll_count == prev_scroll + 1,
                "scroll_count must increment by 1 when player scrolls");
    return 0;
}

static int test_player_scroll_multiple(void)
{
    player_t p;
    map_t map;
    int i;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 4;
    p.y = 0;

    for (i = 0; i < 5; i++) {
        TEST_ASSERT(player_move(&p, ACTION_MOVE_UP, &map) == 0,
                    "repeated scroll must return 0 each time");
    }
    TEST_ASSERT(map.scroll_count == 5,
                "scroll_count must equal number of scrolls performed");
    TEST_ASSERT(p.y == 0,
                "player y must remain 0 after multiple scrolls");
    return 0;
}

/* ── Test cases: 5-buffer scroll ─────────────────────────────────────── */

/*
 * 5-buffer rule: when player moves UP and lands in the buffer zone
 * (new_y < SCROLL_BUFFER), map_scroll() fires automatically and
 * player.y is pushed back up to SCROLL_BUFFER.
 *
 * Setup: player at y=SCROLL_BUFFER (=5). Target y=4 (<5 — buffer zone).
 * Expected: scroll fires (scroll_count+1), player.y == SCROLL_BUFFER (5).
 */
static int test_player_5buffer_scroll_triggers(void)
{
    player_t p;
    map_t    map;
    long     prev_scroll;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 4;
    p.y = SCROLL_BUFFER; /* y=5 — one step above buffer zone */
    map_set_tile(&map, 4, SCROLL_BUFFER, TILE_PLAYER);
    prev_scroll = map.scroll_count;

    TEST_ASSERT(player_move(&p, ACTION_MOVE_UP, &map) == 0,
                "moving into buffer zone must return 0");
    TEST_ASSERT(map.scroll_count == prev_scroll + 1,
                "5-buffer scroll must fire when player enters buffer zone");
    TEST_ASSERT(p.y == SCROLL_BUFFER,
                "player y must be restored to SCROLL_BUFFER after 5-buffer scroll");
    return 0;
}

/* ── Test cases: TILE_COIN passability ───────────────────────────────── */

/*
 * Player moving onto a TILE_COIN must succeed (return 0), player moves
 * to the coin tile, coin tile becomes TILE_PLAYER, old tile becomes
 * TILE_FLOOR.
 *
 * Uses y=7 → y=6 (both >= SCROLL_BUFFER=5) to avoid triggering 5-buffer.
 */
static int test_player_coin_walkable(void)
{
    player_t    p;
    map_t       map;
    tile_type_t tile;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(make_map(&map) == 0,  "map_init should return 0");
    p.x = 4;
    p.y = 7;
    map_set_tile(&map, 4, 7, TILE_PLAYER);
    map_set_tile(&map, 4, 6, TILE_COIN); /* coin directly above player */

    TEST_ASSERT(player_move(&p, ACTION_MOVE_UP, &map) == 0,
                "stepping onto TILE_COIN must return 0 (passable)");
    TEST_ASSERT(p.y == 6,
                "player must move to coin tile position");
    TEST_ASSERT(map_get_tile(&map, 4, 6, &tile) == 0, "get tile must succeed");
    TEST_ASSERT(tile == TILE_PLAYER,
                "coin tile must become TILE_PLAYER after collection");
    TEST_ASSERT(map_get_tile(&map, 4, 7, &tile) == 0, "get old tile must succeed");
    TEST_ASSERT(tile == TILE_FLOOR,
                "old player tile must become TILE_FLOOR after move");
    return 0;
}

/* ── Test cases: null pointer safety ─────────────────────────────────── */

static int test_player_move_null_player(void)
{
    map_t map;
    TEST_ASSERT(make_map(&map) == 0, "map_init should return 0");
    TEST_ASSERT(player_move(NULL, ACTION_MOVE_UP, &map) == -1,
                "player_move(NULL, ...) must return -1");
    return 0;
}

static int test_player_move_null_map(void)
{
    player_t p;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(player_move(&p, ACTION_MOVE_UP, NULL) == -1,
                "player_move(..., NULL) must return -1");
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
        { "test_player_init_hp",                    test_player_init_hp                    },
        { "test_player_init_max_hp",                test_player_init_max_hp                },
        { "test_player_init_atk",                   test_player_init_atk                   },
        { "test_player_init_position",              test_player_init_position              },
        { "test_player_init_null",                  test_player_init_null                  },
        { "test_player_move_up",                    test_player_move_up                    },
        { "test_player_move_down",                  test_player_move_down                  },
        { "test_player_move_left",                  test_player_move_left                  },
        { "test_player_move_right",                 test_player_move_right                 },
        { "test_player_blocked_by_left_wall",       test_player_blocked_by_left_wall       },
        { "test_player_blocked_by_right_wall",      test_player_blocked_by_right_wall      },
        { "test_player_blocked_at_viewport_bottom", test_player_blocked_at_viewport_bottom },
        { "test_player_scroll_at_top",              test_player_scroll_at_top              },
        { "test_player_scroll_multiple",            test_player_scroll_multiple            },
        { "test_player_5buffer_scroll_triggers",    test_player_5buffer_scroll_triggers    },
        { "test_player_coin_walkable",              test_player_coin_walkable              },
        { "test_player_move_null_player",           test_player_move_null_player           },
        { "test_player_move_null_map",              test_player_move_null_map              },
    };

    printf("=== Player Tests ===\n");
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
