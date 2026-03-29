/**
 * @file test_turn.c
 * @brief TDD tests for the turn_manager module (Plan 3).
 *
 * Written BEFORE the real implementation — all tests should FAIL when
 * linked against stub turn_manager.c, and pass after implementation.
 *
 * Return convention: 0 = pass, -1 = fail.
 */
#include <stdio.h>
#include "turn_manager.h"

/* ── Minimal test framework ───────────────────────────────────────────── */
#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

static int iabs(int v) { return v < 0 ? -v : v; }

/* ── turn_manager_init tests ──────────────────────────────────────────── */

static int test_turn_init_places_player_tile(void)
{
    game_state_t state;
    tile_type_t  tile;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    TEST_ASSERT(map_get_tile(&state.map,
                             state.player.x, state.player.y, &tile) == 0,
                "map_get_tile at player pos should succeed");
    TEST_ASSERT(tile == TILE_PLAYER,
                "map must have TILE_PLAYER at player starting position");
    return 0;
}

static int test_turn_init_monsters_empty(void)
{
    game_state_t state;
    int i;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        TEST_ASSERT(state.monsters[i].alive == 0,
                    "all monster slots must have alive==0 after init");
    }
    return 0;
}

static int test_turn_init_null(void)
{
    TEST_ASSERT(turn_manager_init(NULL) == -1,
                "turn_manager_init(NULL) must return -1");
    return 0;
}

/* ── turn_manager_player_act basic movement tests ─────────────────────── */

static int test_turn_player_act_moves_player(void)
{
    game_state_t state;
    int before_y;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    before_y = state.player.y;

    TEST_ASSERT(turn_manager_player_act(&state, ACTION_MOVE_UP) == 0,
                "player_act(UP) from open floor should return 0");
    TEST_ASSERT(state.player.y == before_y - 1,
                "player y must decrease by 1 after ACTION_MOVE_UP");
    return 0;
}

static int test_turn_player_act_updates_player_tile(void)
{
    game_state_t state;
    tile_type_t  tile;
    int before_x;
    int before_y;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    before_x = state.player.x;
    before_y = state.player.y;

    turn_manager_player_act(&state, ACTION_MOVE_UP);

    /* Old position must be floor */
    TEST_ASSERT(map_get_tile(&state.map, before_x, before_y, &tile) == 0,
                "map_get_tile at old player pos should succeed");
    TEST_ASSERT(tile == TILE_FLOOR,
                "old player position must be TILE_FLOOR after move");

    /* New position must be player */
    TEST_ASSERT(map_get_tile(&state.map,
                             state.player.x, state.player.y, &tile) == 0,
                "map_get_tile at new player pos should succeed");
    TEST_ASSERT(tile == TILE_PLAYER,
                "new player position must be TILE_PLAYER after move");
    return 0;
}

static int test_turn_player_act_blocked_returns_1(void)
{
    game_state_t state;
    int result;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");

    /* Move player to bottom row (PLAYER_INIT_Y = VIEWPORT_H-2, one step down) */
    turn_manager_player_act(&state, ACTION_MOVE_DOWN);
    /* Player is now at VIEWPORT_H-1; next DOWN must be blocked */
    result = turn_manager_player_act(&state, ACTION_MOVE_DOWN);

    TEST_ASSERT(result == 1,
                "player_act blocked at viewport bottom must return 1");
    return 0;
}

/* When player is blocked, monsters must NOT move */
static int test_turn_player_blocked_monsters_stay(void)
{
    game_state_t state;
    int before_x;
    int before_y;
    int result;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");

    /* Move player to bottom row so next DOWN is blocked */
    turn_manager_player_act(&state, ACTION_MOVE_DOWN);

    /* Spawn a monster now (after the first monsters_act which had nothing) */
    TEST_ASSERT(turn_manager_spawn_monster(&state,
                                           PLAYER_INIT_X + 2,
                                           VIEWPORT_H - 3) == 0,
                "spawn_monster should return 0");
    before_x = state.monsters[0].x;
    before_y = state.monsters[0].y;

    /* Block the player */
    result = turn_manager_player_act(&state, ACTION_MOVE_DOWN);
    TEST_ASSERT(result == 1, "blocked player_act must return 1");
    TEST_ASSERT(state.monsters[0].x == before_x,
                "monster x must not change when player is blocked");
    TEST_ASSERT(state.monsters[0].y == before_y,
                "monster y must not change when player is blocked");
    return 0;
}

/* ── turn_manager_spawn_monster tests ────────────────────────────────── */

static int test_turn_spawn_monster_sets_alive(void)
{
    game_state_t state;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    TEST_ASSERT(turn_manager_spawn_monster(&state, 4, 3) == 0,
                "spawn_monster should return 0");
    TEST_ASSERT(state.monsters[0].alive != 0,
                "spawned monster must have alive != 0");
    return 0;
}

static int test_turn_spawn_monster_places_tile(void)
{
    game_state_t state;
    tile_type_t  tile;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    TEST_ASSERT(turn_manager_spawn_monster(&state, 4, 3) == 0,
                "spawn_monster should return 0");
    TEST_ASSERT(map_get_tile(&state.map, 4, 3, &tile) == 0,
                "map_get_tile at spawn position should succeed");
    TEST_ASSERT(tile == TILE_MONSTER,
                "map must have TILE_MONSTER at spawn position");
    return 0;
}

static int test_turn_spawn_monster_pool_full(void)
{
    game_state_t state;
    int i;
    int result;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");

    /* Fill all slots (interior columns 1..MONSTER_MAX_COUNT) */
    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        TEST_ASSERT(turn_manager_spawn_monster(&state, 1 + i, 3) == 0,
                    "filling pool should return 0 for each spawn");
    }

    /* Next spawn must fail with non-zero */
    result = turn_manager_spawn_monster(&state, 1, 5);
    TEST_ASSERT(result != 0,
                "spawn when pool is full must return non-zero");
    return 0;
}

/* ── turn_manager_alive_count tests ──────────────────────────────────── */

static int test_turn_alive_count_zero_after_init(void)
{
    game_state_t state;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    TEST_ASSERT(turn_manager_alive_count(&state) == 0,
                "alive_count must be 0 after init (no monsters spawned)");
    return 0;
}

static int test_turn_alive_count_after_spawn(void)
{
    game_state_t state;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    turn_manager_spawn_monster(&state, 3, 3);
    turn_manager_spawn_monster(&state, 5, 3);
    TEST_ASSERT(turn_manager_alive_count(&state) == 2,
                "alive_count must equal the number of spawned monsters");
    return 0;
}

/* ── Full turn integration: monster tracks player ────────────────────── */

/*
 * Setup: player at PLAYER_INIT pos (4,8); monster spawned 3 rows above (4,5).
 * Player acts UP → moves to (4,7).
 * Monster acts: dx=0, dy=+2 (player below) → steps down to (4,6).
 * Manhattan distance drops from 3 to 1.
 */
static int test_turn_monster_tracks_player(void)
{
    game_state_t state;
    int dist_before;
    int dist_after;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    /* Spawn monster 3 rows above player's initial position (same column) */
    TEST_ASSERT(turn_manager_spawn_monster(&state,
                                           PLAYER_INIT_X,
                                           PLAYER_INIT_Y - 3) == 0,
                "spawn_monster should return 0");

    dist_before = iabs(state.player.x - state.monsters[0].x)
                + iabs(state.player.y - state.monsters[0].y);

    /* Player acts (moves up) → monster also gets a turn */
    TEST_ASSERT(turn_manager_player_act(&state, ACTION_MOVE_UP) == 0,
                "player_act(UP) should return 0");

    dist_after = iabs(state.player.x - state.monsters[0].x)
               + iabs(state.player.y - state.monsters[0].y);

    TEST_ASSERT(dist_after < dist_before,
                "monster must be closer to player after a full turn");
    return 0;
}

/* ── Scroll integration tests ────────────────────────────────────────── */

/*
 * After shift, a monster that was at VIEWPORT_H-1 must have alive==0.
 * Tests turn_manager_shift_monsters() directly to avoid spawn_row()
 * potentially re-using the freed slot before we can inspect it.
 */
static int test_turn_scroll_removes_monster_at_bottom(void)
{
    game_state_t state;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");

    /* Spawn monster at very bottom row */
    TEST_ASSERT(turn_manager_spawn_monster(&state, 4, VIEWPORT_H - 1) == 0,
                "spawn_monster at bottom row should return 0");
    TEST_ASSERT(state.monsters[0].alive != 0,
                "monster must be alive before shift");

    /* Call shift directly (no spawn_row, no monsters_act — clean isolation) */
    TEST_ASSERT(turn_manager_shift_monsters(&state) == 0,
                "turn_manager_shift_monsters should return 0");

    TEST_ASSERT(state.monsters[0].alive == 0,
                "monster shifted past viewport bottom must be removed (alive==0)");
    return 0;
}

/* Scroll increments scroll_count */
static int test_turn_scroll_increments_scroll_count(void)
{
    game_state_t state;
    long before;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    before = state.map.scroll_count;

    /* Teleport player to y=0 and scroll */
    map_set_tile(&state.map, state.player.x, state.player.y, TILE_FLOOR);
    state.player.y = 0;
    map_set_tile(&state.map, state.player.x, 0, TILE_PLAYER);

    turn_manager_player_act(&state, ACTION_MOVE_UP);

    TEST_ASSERT(state.map.scroll_count == before + 1,
                "scroll_count must increase by 1 after scroll");
    return 0;
}

/* After scroll, surviving monsters have their y incremented */
static int test_turn_scroll_shifts_surviving_monsters(void)
{
    game_state_t state;
    int before_y;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");

    /* Spawn monster in the middle of the viewport */
    TEST_ASSERT(turn_manager_spawn_monster(&state, 4, VIEWPORT_H / 2) == 0,
                "spawn_monster should return 0");
    before_y = state.monsters[0].y; /* = VIEWPORT_H/2 = 5 */

    /* Teleport player to y=0 */
    map_set_tile(&state.map, state.player.x, state.player.y, TILE_FLOOR);
    state.player.y = 0;
    map_set_tile(&state.map, state.player.x, 0, TILE_PLAYER);

    turn_manager_player_act(&state, ACTION_MOVE_UP);

    /*
     * After scroll+shift: monster.y = before_y + 1.
     * After monsters_act: monster steps toward player (y may decrease again).
     * Net result: monster.y >= before_y (shifted down, then possibly stepped up).
     * At minimum, alive must still be true (not at bottom).
     */
    TEST_ASSERT(state.monsters[0].alive != 0,
                "middle-viewport monster must still be alive after scroll");
    TEST_ASSERT(state.monsters[0].y > before_y || state.monsters[0].y == before_y,
                "monster y must be >= before_y after shift+act");
    return 0;
}

/* ── Phase 4: open tile tests ─────────────────────────────────────────── */

/* After open_chest, tile must be TILE_CHEST_OPEN (not TILE_FLOOR) */
static int test_turn_open_chest_leaves_chest_open(void)
{
    game_state_t state;
    tile_type_t  tile;
    int          cx;
    int          cy;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    cx = PLAYER_INIT_X;
    cy = PLAYER_INIT_Y - 1;
    map_set_tile(&state.map, cx, cy, TILE_CHEST);

    turn_manager_open_chest(&state, cx, cy);

    TEST_ASSERT(map_get_tile(&state.map, cx, cy, &tile) == 0,
                "map_get_tile after open_chest should succeed");
    TEST_ASSERT(tile == TILE_CHEST_OPEN,
                "open_chest must replace TILE_CHEST with TILE_CHEST_OPEN");
    return 0;
}

/* After enter_shop (successful purchase), tile must be TILE_SHOP_OPEN */
static int test_turn_enter_shop_leaves_shop_open(void)
{
    game_state_t state;
    tile_type_t  tile;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    state.player.coins = SHOP_ITEM_COST;
    map_set_tile(&state.map, 3, 0, TILE_SHOP);

    turn_manager_enter_shop(&state, 3, 0);

    TEST_ASSERT(map_get_tile(&state.map, 3, 0, &tile) == 0,
                "map_get_tile after enter_shop should succeed");
    TEST_ASSERT(tile == TILE_SHOP_OPEN,
                "enter_shop must replace TILE_SHOP with TILE_SHOP_OPEN");
    return 0;
}

/* ── NULL argument safety ─────────────────────────────────────────────── */

static int test_turn_player_act_null(void)
{
    TEST_ASSERT(turn_manager_player_act(NULL, ACTION_MOVE_UP) == -1,
                "turn_manager_player_act(NULL, ...) must return -1");
    return 0;
}

static int test_turn_monsters_act_null(void)
{
    TEST_ASSERT(turn_manager_monsters_act(NULL) == -1,
                "turn_manager_monsters_act(NULL) must return -1");
    return 0;
}

static int test_turn_spawn_null(void)
{
    TEST_ASSERT(turn_manager_spawn_monster(NULL, 3, 3) == -1,
                "turn_manager_spawn_monster(NULL, ...) must return -1");
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
        { "test_turn_init_places_player_tile",         test_turn_init_places_player_tile         },
        { "test_turn_init_monsters_empty",             test_turn_init_monsters_empty             },
        { "test_turn_init_null",                       test_turn_init_null                       },
        { "test_turn_player_act_moves_player",         test_turn_player_act_moves_player         },
        { "test_turn_player_act_updates_player_tile",  test_turn_player_act_updates_player_tile  },
        { "test_turn_player_act_blocked_returns_1",    test_turn_player_act_blocked_returns_1    },
        { "test_turn_player_blocked_monsters_stay",    test_turn_player_blocked_monsters_stay    },
        { "test_turn_spawn_monster_sets_alive",        test_turn_spawn_monster_sets_alive        },
        { "test_turn_spawn_monster_places_tile",       test_turn_spawn_monster_places_tile       },
        { "test_turn_spawn_monster_pool_full",         test_turn_spawn_monster_pool_full         },
        { "test_turn_alive_count_zero_after_init",     test_turn_alive_count_zero_after_init     },
        { "test_turn_alive_count_after_spawn",         test_turn_alive_count_after_spawn         },
        { "test_turn_monster_tracks_player",           test_turn_monster_tracks_player           },
        { "test_turn_scroll_removes_monster_at_bottom",test_turn_scroll_removes_monster_at_bottom},
        { "test_turn_scroll_increments_scroll_count",  test_turn_scroll_increments_scroll_count  },
        { "test_turn_scroll_shifts_surviving_monsters",test_turn_scroll_shifts_surviving_monsters},
        { "test_turn_player_act_null",                 test_turn_player_act_null                 },
        { "test_turn_monsters_act_null",               test_turn_monsters_act_null               },
        { "test_turn_spawn_null",                      test_turn_spawn_null                      },
        { "test_turn_open_chest_leaves_chest_open",    test_turn_open_chest_leaves_chest_open    },
        { "test_turn_enter_shop_leaves_shop_open",     test_turn_enter_shop_leaves_shop_open     },
    };

    printf("=== Turn Manager Tests ===\n");
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
