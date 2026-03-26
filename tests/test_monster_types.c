/**
 * @file test_monster_types.c
 * @brief TDD Red: monster type diversification tests.
 *
 * Tests:
 *  1. monster_init_typed GOBLIN → correct hp/atk/type
 *  2. monster_init_typed SLIME  → correct hp/atk/type (high HP)
 *  3. monster_init_typed BAT    → correct hp/atk/type (high ATK)
 *  4. monster_init() defaults to GOBLIN stats
 *  5. monster_init_typed NULL guard
 *  6. turn_manager_spawn_monster_typed SLIME has SLIME stats (unscaled)
 *  7. turn_manager_spawn_monster_typed BAT   has BAT   stats (unscaled)
 *  8. turn_manager_spawn_monster (random) always has a valid type
 *  9. Scaled SLIME at depth 20 has hp > SLIME_INIT_HP
 * 10. Scaled BAT   at depth 20 has atk > BAT_INIT_ATK
 */
#include <stdio.h>
#include <assert.h>
#include "monster.h"
#include "turn_manager.h"
#include "map.h"

/* ── 1. monster_init_typed GOBLIN ────────────────────────────────────── */
static void test_goblin_stats(void)
{
    monster_t m;
    assert(monster_init_typed(&m, 1, 2, MONSTER_TYPE_GOBLIN) == 0);
    assert(m.hp    == GOBLIN_INIT_HP);
    assert(m.atk   == GOBLIN_INIT_ATK);
    assert(m.type  == MONSTER_TYPE_GOBLIN);
    assert(m.alive == 1);
    assert(m.x == 1 && m.y == 2);
    printf("[PASS] monster_init_typed GOBLIN: hp=%d atk=%d\n",
           m.hp, m.atk);
}

/* ── 2. monster_init_typed SLIME ─────────────────────────────────────── */
static void test_slime_stats(void)
{
    monster_t m;
    assert(monster_init_typed(&m, 0, 0, MONSTER_TYPE_SLIME) == 0);
    assert(m.hp    == SLIME_INIT_HP);
    assert(m.atk   == SLIME_INIT_ATK);
    assert(m.type  == MONSTER_TYPE_SLIME);
    assert(m.hp    >  GOBLIN_INIT_HP);   /* slime is tankier */
    assert(m.atk   <  GOBLIN_INIT_ATK);  /* slime hits softer */
    printf("[PASS] monster_init_typed SLIME: hp=%d atk=%d\n",
           m.hp, m.atk);
}

/* ── 3. monster_init_typed BAT ───────────────────────────────────────── */
static void test_bat_stats(void)
{
    monster_t m;
    assert(monster_init_typed(&m, 0, 0, MONSTER_TYPE_BAT) == 0);
    assert(m.hp    == BAT_INIT_HP);
    assert(m.atk   == BAT_INIT_ATK);
    assert(m.type  == MONSTER_TYPE_BAT);
    assert(m.hp    <  GOBLIN_INIT_HP);   /* bat is fragile */
    assert(m.atk   >  GOBLIN_INIT_ATK);  /* bat hits hard  */
    printf("[PASS] monster_init_typed BAT: hp=%d atk=%d\n",
           m.hp, m.atk);
}

/* ── 4. monster_init() defaults to GOBLIN ────────────────────────────── */
static void test_init_defaults_to_goblin(void)
{
    monster_t m;
    assert(monster_init(&m, 0, 0) == 0);
    assert(m.type == MONSTER_TYPE_GOBLIN);
    assert(m.hp   == GOBLIN_INIT_HP);
    assert(m.atk  == GOBLIN_INIT_ATK);
    /* MONSTER_INIT_HP/ATK are aliases for GOBLIN values */
    assert(m.hp   == MONSTER_INIT_HP);
    assert(m.atk  == MONSTER_INIT_ATK);
    printf("[PASS] monster_init: defaults to GOBLIN (hp=%d atk=%d)\n",
           m.hp, m.atk);
}

/* ── 5. monster_init_typed NULL guard ────────────────────────────────── */
static void test_init_typed_null(void)
{
    assert(monster_init_typed(NULL, 0, 0, MONSTER_TYPE_GOBLIN) == -1);
    printf("[PASS] monster_init_typed: NULL guard\n");
}

/* ── 6. turn_manager_spawn_monster_typed SLIME ───────────────────────── */
static void test_spawn_typed_slime(void)
{
    game_state_t state;
    monster_t   *p_m = NULL;
    int          i;

    turn_manager_init(&state);
    map_set_tile(&state.map, 1, 0, TILE_FLOOR);
    assert(turn_manager_spawn_monster_typed(&state, 1, 0, MONSTER_TYPE_SLIME) == 0);

    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (state.monsters[i].alive &&
            state.monsters[i].x == 1 && state.monsters[i].y == 0) {
            p_m = &state.monsters[i];
            break;
        }
    }
    assert(p_m != NULL);
    assert(p_m->type == MONSTER_TYPE_SLIME);
    assert(p_m->hp   == SLIME_INIT_HP);
    assert(p_m->atk  == SLIME_INIT_ATK);
    printf("[PASS] spawn_typed SLIME: hp=%d atk=%d\n", p_m->hp, p_m->atk);
}

/* ── 7. turn_manager_spawn_monster_typed BAT ─────────────────────────── */
static void test_spawn_typed_bat(void)
{
    game_state_t state;
    monster_t   *p_m = NULL;
    int          i;

    turn_manager_init(&state);
    map_set_tile(&state.map, 2, 0, TILE_FLOOR);
    assert(turn_manager_spawn_monster_typed(&state, 2, 0, MONSTER_TYPE_BAT) == 0);

    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (state.monsters[i].alive &&
            state.monsters[i].x == 2 && state.monsters[i].y == 0) {
            p_m = &state.monsters[i];
            break;
        }
    }
    assert(p_m != NULL);
    assert(p_m->type == MONSTER_TYPE_BAT);
    assert(p_m->hp   == BAT_INIT_HP);
    assert(p_m->atk  == BAT_INIT_ATK);
    printf("[PASS] spawn_typed BAT: hp=%d atk=%d\n", p_m->hp, p_m->atk);
}

/* ── 8. turn_manager_spawn_monster (random) always has a valid type ──── */
static void test_spawn_random_valid_type(void)
{
    game_state_t state;
    int          i;
    int          j;

    for (j = 0; j < 30; j++) {
        turn_manager_init(&state);
        map_set_tile(&state.map, 1, 0, TILE_FLOOR);
        assert(turn_manager_spawn_monster(&state, 1, 0) == 0);

        for (i = 0; i < MONSTER_MAX_COUNT; i++) {
            if (state.monsters[i].alive &&
                state.monsters[i].x == 1 && state.monsters[i].y == 0) {
                assert(state.monsters[i].type >= 0);
                assert(state.monsters[i].type < MONSTER_TYPE_COUNT);
                break;
            }
        }
    }
    printf("[PASS] spawn_monster (random): always valid type (30 trials)\n");
}

/* ── 9. Scaled SLIME at depth 20 has hp > SLIME_INIT_HP ─────────────── */
static void test_scaled_slime(void)
{
    game_state_t state;
    monster_t   *p_m = NULL;
    int          i;

    turn_manager_init(&state);
    state.map.scroll_count = 20; /* depth = 2 */
    map_set_tile(&state.map, 1, 0, TILE_FLOOR);
    assert(turn_manager_spawn_monster_typed(&state, 1, 0, MONSTER_TYPE_SLIME) == 0);

    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (state.monsters[i].alive &&
            state.monsters[i].x == 1 && state.monsters[i].y == 0) {
            p_m = &state.monsters[i];
            break;
        }
    }
    assert(p_m != NULL);
    /* depth=2: hp = int(40 * 1.1^2) = int(48.4) = 48 > 40 */
    assert(p_m->hp > SLIME_INIT_HP);
    printf("[PASS] scaled SLIME: hp=%d at depth 20\n", p_m->hp);
}

/* ── 10. Scaled BAT at depth 20 has atk > BAT_INIT_ATK ─────────────── */
static void test_scaled_bat(void)
{
    game_state_t state;
    monster_t   *p_m = NULL;
    int          i;

    turn_manager_init(&state);
    state.map.scroll_count = 20; /* depth = 2 */
    map_set_tile(&state.map, 2, 0, TILE_FLOOR);
    assert(turn_manager_spawn_monster_typed(&state, 2, 0, MONSTER_TYPE_BAT) == 0);

    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (state.monsters[i].alive &&
            state.monsters[i].x == 2 && state.monsters[i].y == 0) {
            p_m = &state.monsters[i];
            break;
        }
    }
    assert(p_m != NULL);
    /* depth=2: atk = int(10 * 1.1^2) = int(12.1) = 12 > 10 */
    assert(p_m->atk > BAT_INIT_ATK);
    printf("[PASS] scaled BAT: atk=%d at depth 20\n", p_m->atk);
}

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void)
{
    printf("=== test_monster_types ===\n");
    test_goblin_stats();
    test_slime_stats();
    test_bat_stats();
    test_init_defaults_to_goblin();
    test_init_typed_null();
    test_spawn_typed_slime();
    test_spawn_typed_bat();
    test_spawn_random_valid_type();
    test_scaled_slime();
    test_scaled_bat();
    printf("=== All monster type tests passed. ===\n");
    return 0;
}
