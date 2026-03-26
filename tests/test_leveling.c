/**
 * @file test_leveling.c
 * @brief TDD Red: leveling, XP, and monster scaling system tests.
 *
 * Tests:
 *  1. player_init sets level=1, xp=0
 *  2. player_gain_xp accumulates xp without triggering levelup
 *  3. Exact 50 XP causes level 1→2 with correct stat bonuses and full heal
 *  4. Leftover XP is carried over after levelup
 *  5. Enough XP for two consecutive levelups works correctly
 *  6. Killing a monster via turn_manager grants XP to the player
 *  7. Monsters spawned at depth>=20 have scaled HP and ATK
 */
#include <stdio.h>
#include <assert.h>
#include "player.h"
#include "turn_manager.h"
#include "monster.h"
#include "map.h"

/* ── 1. player_init initialises level and xp ─────────────────────────── */
static void test_player_init_level_xp(void)
{
    player_t p;
    assert(player_init(&p) == 0);
    assert(p.level == 1);
    assert(p.xp    == 0);
    printf("[PASS] player_init: level=1, xp=0\n");
}

/* ── 2. XP accumulates without levelup ───────────────────────────────── */
static void test_gain_xp_no_levelup(void)
{
    player_t p;
    player_init(&p);
    assert(player_gain_xp(&p, 10) == 0);
    assert(p.xp    == 10);
    assert(p.level == 1);
    printf("[PASS] gain_xp: xp accumulates below threshold\n");
}

/* ── 3. Exact 50 XP triggers level 1→2 with stat bonuses ────────────── */
static void test_levelup_exact(void)
{
    player_t p;
    int old_max_hp, old_atk, old_def;

    player_init(&p);
    old_max_hp = p.max_hp;
    old_atk    = p.atk;
    old_def    = p.def;

    /* Damage player so full-heal reward is observable */
    p.hp = 50;

    assert(player_gain_xp(&p, 50) == 0); /* level 1 → 2 needs 50 XP */
    assert(p.level  == 2);
    assert(p.xp     == 0);
    assert(p.max_hp == old_max_hp + LEVELUP_MAXHP_BONUS);
    assert(p.hp     == p.max_hp);        /* full heal on levelup */
    assert(p.atk    == old_atk + LEVELUP_ATK_BONUS);
    assert(p.def    == old_def + LEVELUP_DEF_BONUS);
    printf("[PASS] levelup: level 1->2 at exactly 50 xp, stats correct\n");
}

/* ── 4. Leftover XP is carried over after levelup ────────────────────── */
static void test_levelup_leftover_xp(void)
{
    player_t p;
    player_init(&p);
    assert(player_gain_xp(&p, 75) == 0); /* 50 used, 25 leftover */
    assert(p.level == 2);
    assert(p.xp    == 25);
    printf("[PASS] levelup: leftover xp=%d carried over\n", p.xp);
}

/* ── 5. Two consecutive levelups from a single call ─────────────────── */
static void test_double_levelup(void)
{
    player_t p;
    player_init(&p);
    /* Level 1→2: 50 XP, level 2→3: 100 XP, total needed: 150 */
    assert(player_gain_xp(&p, 150) == 0);
    assert(p.level == 3);
    printf("[PASS] double levelup: level 1->3 with 150 xp\n");
}

/* ── 6. Killing a monster via turn_manager grants XP ─────────────────── */
static void test_kill_grants_xp(void)
{
    game_state_t state;
    int xp_before, px, py, i;

    turn_manager_init(&state);
    px = state.player.x;
    py = state.player.y;

    /* Spawn a monster directly above the player */
    assert(turn_manager_spawn_monster(&state, px, py - 1) == 0);

    /* Set monster HP to 1 so the player one-shots it */
    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (state.monsters[i].alive &&
            state.monsters[i].x == px &&
            state.monsters[i].y == py - 1) {
            state.monsters[i].hp = 1;
            break;
        }
    }

    xp_before = state.player.xp;
    turn_manager_player_act(&state, ACTION_MOVE_UP);

    /* Player should have gained XP for the kill */
    assert(state.player.xp > xp_before);
    printf("[PASS] kill monster: xp gained (was %d, now %d)\n",
           xp_before, state.player.xp);
}

/* ── 7. Monsters spawned at depth>=20 have scaled stats ─────────────── */
static void test_monster_scaling(void)
{
    game_state_t state;
    monster_t   *p_m = NULL;
    int          i;

    turn_manager_init(&state);

    /* Simulate the player having scrolled 20 rows (depth = 20/10 = 2) */
    state.map.scroll_count = 20;

    /* Ensure spawn cell is clear floor; use GOBLIN for deterministic stats */
    map_set_tile(&state.map, 1, 0, TILE_FLOOR);
    assert(turn_manager_spawn_monster_typed(&state, 1, 0, MONSTER_TYPE_GOBLIN) == 0);

    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (state.monsters[i].alive &&
            state.monsters[i].x == 1 &&
            state.monsters[i].y == 0) {
            p_m = &state.monsters[i];
            break;
        }
    }

    assert(p_m != NULL);

    /*
     * GOBLIN at depth=2: hp  = (int)(20 * 1.1^2) = (int)(24.2) = 24  > 20
     *                    atk = (int)( 5 * 1.1^2) = (int)( 6.05) =  6  >  5
     */
    assert(p_m->hp  > GOBLIN_INIT_HP);
    assert(p_m->atk > GOBLIN_INIT_ATK);
    printf("[PASS] monster scaling: hp=%d atk=%d at scroll=20\n",
           p_m->hp, p_m->atk);
}

/* ── NULL-safety guards ───────────────────────────────────────────────── */
static void test_gain_xp_null(void)
{
    assert(player_gain_xp(NULL, 10) == -1);
    printf("[PASS] player_gain_xp: NULL guard\n");
}

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void)
{
    printf("=== test_leveling ===\n");
    test_player_init_level_xp();
    test_gain_xp_no_levelup();
    test_levelup_exact();
    test_levelup_leftover_xp();
    test_double_levelup();
    test_kill_grants_xp();
    test_monster_scaling();
    test_gain_xp_null();
    printf("=== All leveling tests passed. ===\n");
    return 0;
}
