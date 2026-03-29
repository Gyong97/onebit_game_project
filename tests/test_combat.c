/**
 * @file test_combat.c
 * @brief Phase 4 TDD: combat, chest, and game-over tests.
 *
 * Written BEFORE the real implementation — all tests should FAIL when
 * linked against Phase 3 stubs, and pass after Phase 4 implementation.
 *
 * Tests:
 *   1  player_move into TILE_MONSTER  returns PLAYER_MOVE_ATTACK
 *   2  player_move into TILE_CHEST    returns PLAYER_MOVE_CHEST
 *   3  player attack reduces monster HP by player.atk
 *   4  monster killed (hp <= 0) has alive == 0
 *   5  dead monster tile becomes TILE_FLOOR
 *   6  monster_step into TILE_PLAYER  reduces player.hp by monster.atk
 *   7  turn_manager_player_act returns TURN_GAME_OVER when player.hp <= 0
 *   8  turn_manager_open_chest sets tile to TILE_FLOOR
 *   9  turn_manager_open_chest improves player HP or ATK
 *  10  player_act into TILE_CHEST calls open_chest (tile becomes floor)
 *  11  player attack triggers monster turn (non-attacked monster acts)
 */
#include <stdio.h>
#include <stdlib.h>   /* srand */
#include "turn_manager.h"

/* ── Minimal test framework ───────────────────────────────────────────── */
#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

/* ── Test 1: player_move into monster returns PLAYER_MOVE_ATTACK ──────── */

static int test_player_move_attack_return_code(void)
{
    game_state_t state;
    int          result;

    turn_manager_init(&state);
    /* Spawn a monster directly above the player */
    turn_manager_spawn_monster(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 1);

    result = player_move(&state.player, ACTION_MOVE_UP, &state.map);
    TEST_ASSERT(result == PLAYER_MOVE_ATTACK,
                "player_move into TILE_MONSTER must return PLAYER_MOVE_ATTACK");
    return 0;
}

/* ── Test 2: player_move into chest returns PLAYER_MOVE_CHEST ─────────── */

static int test_player_move_chest_return_code(void)
{
    game_state_t state;
    int          result;

    turn_manager_init(&state);
    map_set_tile(&state.map, PLAYER_INIT_X, PLAYER_INIT_Y - 1, TILE_CHEST);

    result = player_move(&state.player, ACTION_MOVE_UP, &state.map);
    TEST_ASSERT(result == PLAYER_MOVE_CHEST,
                "player_move into TILE_CHEST must return PLAYER_MOVE_CHEST");
    return 0;
}

/* ── Test 3: player attack reduces monster HP ─────────────────────────── */

static int test_player_attack_damages_monster(void)
{
    game_state_t state;
    int          initial_hp;

    turn_manager_init(&state);
    turn_manager_spawn_monster(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 1);
    initial_hp = state.monsters[0].hp; /* MONSTER_INIT_HP = 20 */

    /* Player attacks the monster directly above */
    turn_manager_player_act(&state, ACTION_MOVE_UP);

    TEST_ASSERT(state.monsters[0].hp == initial_hp - state.player.atk,
                "player attack must reduce monster.hp by player.atk");
    return 0;
}

/* ── Test 4: monster killed (hp <= 0) becomes alive == 0 ─────────────── */

static int test_player_kills_monster_alive_cleared(void)
{
    game_state_t state;

    turn_manager_init(&state);
    turn_manager_spawn_monster(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 1);
    /* Force monster HP so one player hit kills it */
    state.monsters[0].hp = state.player.atk; /* 10 */

    turn_manager_player_act(&state, ACTION_MOVE_UP);

    TEST_ASSERT(state.monsters[0].alive == 0,
                "monster with hp <= 0 after player attack must have alive == 0");
    return 0;
}

/* ── Test 5: dead monster tile becomes TILE_FLOOR ─────────────────────── */

static int test_dead_monster_tile_becomes_floor(void)
{
    game_state_t state;
    tile_type_t  tile;
    int          mx;
    int          my;

    turn_manager_init(&state);
    turn_manager_spawn_monster(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 1);
    mx = state.monsters[0].x;
    my = state.monsters[0].y;
    state.monsters[0].hp = state.player.atk; /* one-shot */

    turn_manager_player_act(&state, ACTION_MOVE_UP);

    map_get_tile(&state.map, mx, my, &tile);
    TEST_ASSERT(tile == TILE_FLOOR,
                "tile at dead monster position must become TILE_FLOOR");
    return 0;
}

/* ── Test 6: monster_step into TILE_PLAYER reduces player.hp ─────────── */

static int test_monster_step_damages_player(void)
{
    game_state_t state;
    int          initial_hp;

    turn_manager_init(&state);
    /* Monster directly above player: dy=1 primary, will step into TILE_PLAYER */
    turn_manager_spawn_monster(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 1);
    initial_hp = state.player.hp;

    turn_manager_monsters_act(&state);

    TEST_ASSERT(state.player.hp == initial_hp - state.monsters[0].atk,
                "monster_step into TILE_PLAYER must reduce player.hp by monster.atk");
    return 0;
}

/* ── Test 7: TURN_GAME_OVER returned when player.hp reaches 0 ─────────── */

static int test_game_over_return_code(void)
{
    game_state_t state;
    int          result;

    turn_manager_init(&state);
    /*
     * Monster at (PLAYER_INIT_X, PLAYER_INIT_Y - 1).
     * Player moves LEFT to (PLAYER_INIT_X - 1, PLAYER_INIT_Y).
     * Monster then steps: dx>0, dy=1 → prefer vertical (|dy|>|dx| if dy only) or
     * horizontal tie — check: dx=-1, dy=1 prefer_horiz (|dx|>=|dy| → |1|>=|1|).
     * Primary: (px-1+1, py-1) = (px, py-1) → TILE_FLOOR. Monster moves to player's
     * old spot. No attack.
     *
     * Instead, put monster directly to the left of player's destination:
     * Player at (4,8), monster at (2,8). Player moves LEFT to (3,8).
     * Monster at (2,8), player now at (3,8). dx=1, dy=0. prefer_horiz.
     * Primary: (3,8) = TILE_PLAYER → attack! player.hp -= monster.atk.
     */
    turn_manager_spawn_monster(&state, PLAYER_INIT_X - 2, PLAYER_INIT_Y);
    state.player.hp = state.monsters[0].atk; /* one monster hit kills */

    result = turn_manager_player_act(&state, ACTION_MOVE_LEFT);

    TEST_ASSERT(result == TURN_GAME_OVER,
                "turn_manager_player_act must return TURN_GAME_OVER when player.hp reaches 0");
    return 0;
}

/* ── Test 8: open_chest sets tile to TILE_FLOOR ────────────────────────── */

static int test_open_chest_removes_tile(void)
{
    game_state_t state;
    tile_type_t  tile;
    int          cx;
    int          cy;

    turn_manager_init(&state);
    cx = PLAYER_INIT_X;
    cy = PLAYER_INIT_Y - 1;
    map_set_tile(&state.map, cx, cy, TILE_CHEST);

    turn_manager_open_chest(&state, cx, cy);

    map_get_tile(&state.map, cx, cy, &tile);
    TEST_ASSERT(tile == TILE_CHEST_OPEN,
                "turn_manager_open_chest must replace TILE_CHEST with TILE_CHEST_OPEN");
    return 0;
}

/* ── Test 9: open_chest adds item to player inventory ───────────────────── */

static int test_open_chest_improves_stats(void)
{
    game_state_t state;

    turn_manager_init(&state);
    map_set_tile(&state.map, PLAYER_INIT_X, PLAYER_INIT_Y - 1, TILE_CHEST);
    turn_manager_open_chest(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 1);

    TEST_ASSERT(state.player.inventory_count > 0,
                "turn_manager_open_chest must add an item to player inventory");
    return 0;
}

/* ── Test 10: player_act into chest triggers open (tile becomes floor) ── */

static int test_player_act_chest_opens_tile(void)
{
    game_state_t state;
    tile_type_t  tile;
    int          cx;
    int          cy;

    turn_manager_init(&state);
    cx = PLAYER_INIT_X;
    cy = PLAYER_INIT_Y - 1;
    map_set_tile(&state.map, cx, cy, TILE_CHEST);

    turn_manager_player_act(&state, ACTION_MOVE_UP);

    map_get_tile(&state.map, cx, cy, &tile);
    TEST_ASSERT(tile == TILE_CHEST_OPEN,
                "player_act into TILE_CHEST must open chest (tile becomes TILE_CHEST_OPEN)");
    return 0;
}

/* ── Test 11: player attack triggers monster turn ──────────────────────── */

static int test_player_attack_triggers_monster_turn(void)
{
    game_state_t state;
    int          initial_hp;

    turn_manager_init(&state);
    /*
     * Spawn a SLIME (HP=40) adjacent above the player.
     * Player ATK=10: SLIME survives with HP=30 and retaliates.
     * Using a typed spawn for deterministic HP (random type could be BAT
     * HP=10 which the player kills in one hit, preventing retaliation).
     */
    turn_manager_spawn_monster_typed(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 1,
                                     MONSTER_TYPE_SLIME);
    initial_hp = state.player.hp;

    turn_manager_player_act(&state, ACTION_MOVE_UP);

    /* Monster A (still alive with 10 hp) attacked back → player hp decreased */
    TEST_ASSERT(state.player.hp < initial_hp,
                "after player attack, monster turn must execute (player.hp must decrease)");
    return 0;
}

/* ── Main ─────────────────────────────────────────────────────────────── */

int main(void)
{
    int passed = 0;
    int failed = 0;

#define RUN(fn)                                     \
    do {                                            \
        printf("  %-50s", #fn);                     \
        if (fn() == 0) { printf("PASS\n"); passed++; } \
        else            { printf("FAIL\n"); failed++; } \
    } while (0)

    printf("=== test_combat ===\n");
    RUN(test_player_move_attack_return_code);
    RUN(test_player_move_chest_return_code);
    RUN(test_player_attack_damages_monster);
    RUN(test_player_kills_monster_alive_cleared);
    RUN(test_dead_monster_tile_becomes_floor);
    RUN(test_monster_step_damages_player);
    RUN(test_game_over_return_code);
    RUN(test_open_chest_removes_tile);
    RUN(test_open_chest_improves_stats);
    RUN(test_player_act_chest_opens_tile);
    RUN(test_player_attack_triggers_monster_turn);

    printf("Result: %d passed, %d failed\n", passed, failed);
    return (failed > 0) ? 1 : 0;
}
