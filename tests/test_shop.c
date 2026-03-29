/**
 * @file test_shop.c
 * @brief TDD Red: shop system tests.
 *
 * Tests:
 *  1. TILE_SHOP has a value greater than TILE_COIN
 *  2. spawn_row at scroll_count=20 places TILE_SHOP in y=0
 *  3. spawn_row at scroll_count=10 does NOT place TILE_SHOP in y=0
 *  4. spawn_row at scroll_count=40 places TILE_SHOP in y=0
 *  5. player_move into TILE_SHOP returns PLAYER_MOVE_SHOP
 *  6. enter_shop: opens shop UI (active=1), coins/inventory unchanged
 *  7. enter_shop: opens shop even with 0 coins (buy/sell handled later)
 *  8. enter_shop: sets TILE_SHOP_OPEN on entry (one-time visit)
 *  9. enter_shop: NULL guard returns -1
 * 10. player_act: bumping into shop returns TURN_SHOP_OPEN, no coin deduction
 */
#include <stdio.h>
#include <assert.h>
#include "renderer.h"
#include "map.h"
#include "player.h"
#include "turn_manager.h"

/* ── helper: count TILE_SHOP cells in row y=0 ────────────────────────── */
static int count_shop_in_top_row(const game_state_t *p_state)
{
    int         x;
    int         count = 0;
    tile_type_t tile;

    for (x = 1; x < MAP_WIDTH - 1; x++) {
        map_get_tile(&p_state->map, x, 0, &tile);
        if (tile == TILE_SHOP) {
            count++;
        }
    }
    return count;
}

/* ── 1. TILE_SHOP > TILE_COIN ────────────────────────────────────────── */
static void test_tile_shop_value(void)
{
    assert(TILE_SHOP > TILE_COIN);
    printf("[PASS] TILE_SHOP (%d) > TILE_COIN (%d)\n",
           (int)TILE_SHOP, (int)TILE_COIN);
}

/* ── 2. spawn_row at scroll_count=20 places TILE_SHOP ───────────────── */
static void test_shop_spawns_at_depth_20(void)
{
    game_state_t state;

    turn_manager_init(&state);
    state.map.scroll_count = 20;
    turn_manager_spawn_row(&state);

    assert(count_shop_in_top_row(&state) == 1);
    printf("[PASS] spawn_row: TILE_SHOP placed at scroll_count=20\n");
}

/* ── 3. spawn_row at scroll_count=10 does NOT place TILE_SHOP ────────── */
static void test_no_shop_at_depth_10(void)
{
    game_state_t state;

    turn_manager_init(&state);
    state.map.scroll_count = 10;
    turn_manager_spawn_row(&state);

    assert(count_shop_in_top_row(&state) == 0);
    printf("[PASS] spawn_row: no TILE_SHOP at scroll_count=10\n");
}

/* ── 4. spawn_row at scroll_count=40 places TILE_SHOP ───────────────── */
static void test_shop_spawns_at_depth_40(void)
{
    game_state_t state;

    turn_manager_init(&state);
    state.map.scroll_count = 40;
    turn_manager_spawn_row(&state);

    assert(count_shop_in_top_row(&state) == 1);
    printf("[PASS] spawn_row: TILE_SHOP placed at scroll_count=40\n");
}

/* ── 5. player_move into TILE_SHOP returns PLAYER_MOVE_SHOP ─────────── */
static void test_player_move_shop_return_code(void)
{
    map_t    map;
    player_t player;
    int      result;

    map_init(&map);
    player_init(&player);

    /* Place player and shop adjacent */
    player.x = 2;
    player.y = 5;
    map_set_tile(&map, 2, 5, TILE_PLAYER);
    map_set_tile(&map, 2, 4, TILE_SHOP);

    result = player_move(&player, ACTION_MOVE_UP, &map);
    assert(result == PLAYER_MOVE_SHOP);
    printf("[PASS] player_move: returns PLAYER_MOVE_SHOP when bumping shop\n");
}

/* ── 6. enter_shop: opens shop UI mode regardless of coins ───────────── */
static void test_enter_shop_success(void)
{
    game_state_t state;
    int          inv_before;
    int          coins_before;

    turn_manager_init(&state);
    state.player.coins = SHOP_ITEM_COST + 5;
    coins_before        = state.player.coins;
    inv_before          = state.player.inventory_count;

    map_set_tile(&state.map, 1, 0, TILE_SHOP);
    assert(turn_manager_enter_shop(&state, 1, 0) == TURN_SHOP_OPEN);

    /* Entering the shop does NOT deduct coins or add items */
    assert(state.player.coins == coins_before);
    assert(state.player.inventory_count == inv_before);
    /* Shop UI is now active */
    assert(state.shop.active == 1);
    printf("[PASS] enter_shop: shop.active=1, coins/inventory unchanged\n");
}

/* ── 7. enter_shop: opens shop even with no coins ────────────────────── */
static void test_enter_shop_insufficient_coins(void)
{
    game_state_t state;
    int          coins_before;
    int          inv_before;

    turn_manager_init(&state);
    state.player.coins = 0;   /* no coins at all */
    coins_before        = state.player.coins;
    inv_before          = state.player.inventory_count;

    map_set_tile(&state.map, 1, 0, TILE_SHOP);
    assert(turn_manager_enter_shop(&state, 1, 0) == TURN_SHOP_OPEN);

    /* Shop still opens; buy/sell restricted later by shop_buy/sell */
    assert(state.shop.active == 1);
    assert(state.player.coins == coins_before);
    assert(state.player.inventory_count == inv_before);
    printf("[PASS] enter_shop: shop opens even with 0 coins\n");
}

/* ── 8. enter_shop: removes shop tile on purchase ────────────────────── */
static void test_enter_shop_removes_tile(void)
{
    game_state_t state;
    tile_type_t  tile;

    turn_manager_init(&state);
    state.player.coins = SHOP_ITEM_COST;

    map_set_tile(&state.map, 3, 0, TILE_SHOP);
    turn_manager_enter_shop(&state, 3, 0);

    map_get_tile(&state.map, 3, 0, &tile);
    assert(tile == TILE_SHOP_OPEN);
    printf("[PASS] enter_shop: shop tile replaced with TILE_SHOP_OPEN on purchase\n");
}

/* ── 9. enter_shop: NULL guard ───────────────────────────────────────── */
static void test_enter_shop_null(void)
{
    assert(turn_manager_enter_shop(NULL, 0, 0) == -1);
    printf("[PASS] enter_shop: NULL guard returns -1\n");
}

/* ── 10. player_act: bumping into shop opens shop UI (no coin deduction) */
static void test_player_act_shop_deducts_coins(void)
{
    game_state_t state;
    int          coins_before;
    int          turn_result;

    turn_manager_init(&state);
    state.player.coins = SHOP_ITEM_COST + 10;
    coins_before        = state.player.coins;

    /* Place shop directly above player */
    map_set_tile(&state.map,
                 state.player.x,
                 state.player.y - 1,
                 TILE_SHOP);

    turn_result = turn_manager_player_act(&state, ACTION_MOVE_UP);

    /* Shop opens — no coin deduction on entry */
    assert(turn_result == TURN_SHOP_OPEN);
    assert(state.shop.active == 1);
    assert(state.player.coins == coins_before);
    printf("[PASS] player_act: shop bump opens shop UI, coins unchanged\n");
}

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void)
{
    printf("=== test_shop ===\n");
    test_tile_shop_value();
    test_shop_spawns_at_depth_20();
    test_no_shop_at_depth_10();
    test_shop_spawns_at_depth_40();
    test_player_move_shop_return_code();
    test_enter_shop_success();
    test_enter_shop_insufficient_coins();
    test_enter_shop_removes_tile();
    test_enter_shop_null();
    test_player_act_shop_deducts_coins();
    printf("=== All shop tests passed. ===\n");
    return 0;
}
