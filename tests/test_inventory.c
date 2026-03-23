/**
 * @file test_inventory.c
 * @brief Phase 2 TDD: coin collection and inventory system tests.
 *
 * Written BEFORE the real implementation — all tests should FAIL when
 * linked against Phase 1 code, and pass after Phase 2 implementation.
 *
 * Tests:
 *   1  player_init zeroes coins field
 *   2  player_init zeroes def field
 *   3  player_init zeroes inventory_count
 *   4  player_add_item increments inventory_count
 *   5  player_add_item stores item type correctly
 *   6  player_add_item stores item bonuses correctly
 *   7  player_inventory_full returns 1 when no room
 *   8  player_add_item null player returns -1
 *   9  player_add_item null item returns -1
 *  10  stepping on TILE_COIN increments player.coins
 *  11  turn_manager_open_chest adds item to inventory
 *  12  chest-generated item has a valid type (not ITEM_NONE)
 */
#include <stdio.h>
#include "turn_manager.h"   /* includes player.h → item.h */

/* ── Minimal test framework ───────────────────────────────────────────── */
#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

/* ── Helper: build a clean item ───────────────────────────────────────── */
static item_t make_potion(void)
{
    item_t it;
    it.type       = ITEM_POTION;
    it.atk_bonus  = 0;
    it.def_bonus  = 0;
    it.hp_restore = ITEM_POTION_HP_RESTORE;
    it.name[0]    = 'P'; it.name[1] = '\0';
    return it;
}

/* ── Test 1: player_init zeroes coins ────────────────────────────────── */

static int test_player_init_coins_zero(void)
{
    player_t p;
    p.coins = 99; /* sentinel: force non-zero before init */
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(p.coins == 0, "coins must be 0 after player_init");
    return 0;
}

/* ── Test 2: player_init zeroes def ──────────────────────────────────── */

static int test_player_init_def_zero(void)
{
    player_t p;
    p.def = 99;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(p.def == PLAYER_INIT_DEF, "def must equal PLAYER_INIT_DEF after init");
    return 0;
}

/* ── Test 3: player_init zeroes inventory_count ─────────────────────── */

static int test_player_init_inventory_empty(void)
{
    player_t p;
    p.inventory_count = 99;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(p.inventory_count == 0,
                "inventory_count must be 0 after player_init");
    return 0;
}

/* ── Test 4: player_add_item increments inventory_count ──────────────── */

static int test_player_add_item_increments_count(void)
{
    player_t p;
    item_t   it = make_potion();

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(player_add_item(&p, &it) == 0,
                "player_add_item must return 0 on success");
    TEST_ASSERT(p.inventory_count == 1,
                "inventory_count must be 1 after adding one item");
    return 0;
}

/* ── Test 5: player_add_item stores item type ────────────────────────── */

static int test_player_add_item_stores_type(void)
{
    player_t p;
    item_t   it = make_potion();

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(player_add_item(&p, &it) == 0, "add_item should return 0");
    TEST_ASSERT(p.inventory[0].type == ITEM_POTION,
                "stored item type must match the added item's type");
    return 0;
}

/* ── Test 6: player_add_item stores item bonuses ─────────────────────── */

static int test_player_add_item_stores_bonuses(void)
{
    player_t p;
    item_t   it = make_potion();

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    TEST_ASSERT(p.inventory[0].hp_restore == ITEM_POTION_HP_RESTORE,
                "stored item hp_restore must match the added item");
    return 0;
}

/* ── Test 7: inventory full returns 1 ───────────────────────────────── */

static int test_player_inventory_full_returns_error(void)
{
    player_t p;
    item_t   it = make_potion();
    int      i;
    int      result;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    for (i = 0; i < INVENTORY_MAX; i++) {
        TEST_ASSERT(player_add_item(&p, &it) == 0,
                    "adding items up to INVENTORY_MAX must return 0");
    }
    result = player_add_item(&p, &it);
    TEST_ASSERT(result == 1,
                "player_add_item must return 1 when inventory is full");
    TEST_ASSERT(p.inventory_count == INVENTORY_MAX,
                "inventory_count must not exceed INVENTORY_MAX");
    return 0;
}

/* ── Test 8: null player returns -1 ─────────────────────────────────── */

static int test_player_add_item_null_player(void)
{
    item_t it = make_potion();
    TEST_ASSERT(player_add_item(NULL, &it) == -1,
                "player_add_item(NULL, item) must return -1");
    return 0;
}

/* ── Test 9: null item returns -1 ───────────────────────────────────── */

static int test_player_add_item_null_item(void)
{
    player_t p;
    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    TEST_ASSERT(player_add_item(&p, NULL) == -1,
                "player_add_item(player, NULL) must return -1");
    return 0;
}

/* ── Test 10: stepping on TILE_COIN increments player.coins ──────────── */

static int test_player_coin_increments_counter(void)
{
    game_state_t state;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    /* Place a coin directly above the player (y=7, below buffer zone) */
    state.player.y = 7;
    map_set_tile(&state.map, state.player.x, 7, TILE_PLAYER);
    map_set_tile(&state.map, state.player.x, 6, TILE_COIN);

    TEST_ASSERT(state.player.coins == 0,
                "coins must be 0 before collecting");
    turn_manager_player_act(&state, ACTION_MOVE_UP);
    TEST_ASSERT(state.player.coins == 1,
                "coins must be 1 after stepping on TILE_COIN");
    return 0;
}

/* ── Test 11: open_chest adds item to inventory ──────────────────────── */

static int test_chest_adds_item_to_inventory(void)
{
    game_state_t state;
    int          cx;
    int          cy;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    cx = state.player.x;
    cy = state.player.y - 1;
    map_set_tile(&state.map, cx, cy, TILE_CHEST);

    TEST_ASSERT(state.player.inventory_count == 0,
                "inventory must be empty before opening chest");
    turn_manager_open_chest(&state, cx, cy);
    TEST_ASSERT(state.player.inventory_count == 1,
                "inventory_count must be 1 after opening a chest");
    return 0;
}

/* ── Test 12: chest item has a valid type ────────────────────────────── */

static int test_chest_item_has_valid_type(void)
{
    game_state_t state;
    int          cx;
    int          cy;
    item_type_t  itype;

    TEST_ASSERT(turn_manager_init(&state) == 0,
                "turn_manager_init should return 0");
    cx = state.player.x;
    cy = state.player.y - 1;
    map_set_tile(&state.map, cx, cy, TILE_CHEST);

    turn_manager_open_chest(&state, cx, cy);

    itype = state.player.inventory[0].type;
    TEST_ASSERT(itype == ITEM_WEAPON || itype == ITEM_ARMOR || itype == ITEM_POTION,
                "chest item type must be WEAPON, ARMOR, or POTION");
    return 0;
}

/* ── Main ─────────────────────────────────────────────────────────────── */

int main(void)
{
    int passed = 0;
    int failed = 0;

#define RUN(fn)                                         \
    do {                                                \
        printf("  %-55s", #fn);                         \
        if (fn() == 0) { printf("PASS\n"); passed++; } \
        else           { printf("FAIL\n"); failed++; }  \
    } while (0)

    printf("=== test_inventory ===\n");
    RUN(test_player_init_coins_zero);
    RUN(test_player_init_def_zero);
    RUN(test_player_init_inventory_empty);
    RUN(test_player_add_item_increments_count);
    RUN(test_player_add_item_stores_type);
    RUN(test_player_add_item_stores_bonuses);
    RUN(test_player_inventory_full_returns_error);
    RUN(test_player_add_item_null_player);
    RUN(test_player_add_item_null_item);
    RUN(test_player_coin_increments_counter);
    RUN(test_chest_adds_item_to_inventory);
    RUN(test_chest_item_has_valid_type);

    printf("Result: %d passed, %d failed\n", passed, failed);
    return (failed > 0) ? 1 : 0;
}
