/**
 * @file test_equipment.c
 * @brief Phase 3 TDD: equipment slots, stat bonuses, and defense tests.
 *
 * Written BEFORE the real implementation — all tests should FAIL when
 * linked against Phase 2 code, and pass after Phase 3 implementation.
 *
 * Tests:
 *   1  player_init sets all equipment slots to -1
 *   2  player_equip weapon increases player.atk by atk_bonus
 *   3  player_equip body armor increases player.def by def_bonus
 *   4  player_equip helmet increases player.def by helmet def_bonus
 *   5  player_equip weapon sets equipment[EQUIP_SLOT_WEAPON] to inv_idx
 *   6  player_equip armor sets equipment[EQUIP_SLOT_BODY] to inv_idx
 *   7  player_equip helmet sets equipment[EQUIP_SLOT_HEAD] to inv_idx
 *   8  player_equip out-of-range index returns -1
 *   9  player_equip ITEM_POTION returns -1
 *  10  player_unequip weapon restores player.atk
 *  11  player_unequip armor restores player.def
 *  12  player_equip second weapon replaces first (old bonus removed, new applied)
 *  13  monster attack is reduced by player.def
 *  14  monster attack cannot reduce player.hp below: damage floored at 0
 */
#include <stdio.h>
#include "turn_manager.h"   /* game_state_t, includes player.h → item.h */

/* ── Minimal test framework ───────────────────────────────────────────── */
#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

/* ── Helpers: build equipment items ──────────────────────────────────── */

static item_t make_weapon(void)
{
    item_t it;
    it.type       = ITEM_WEAPON;
    it.atk_bonus  = ITEM_WEAPON_ATK_BONUS;
    it.def_bonus  = 0;
    it.hp_restore = 0;
    it.name[0] = 'W'; it.name[1] = '\0';
    return it;
}

static item_t make_armor(void)
{
    item_t it;
    it.type       = ITEM_ARMOR;
    it.atk_bonus  = 0;
    it.def_bonus  = ITEM_ARMOR_DEF_BONUS;
    it.hp_restore = 0;
    it.name[0] = 'A'; it.name[1] = '\0';
    return it;
}

static item_t make_helmet(void)
{
    item_t it;
    it.type       = ITEM_HELMET;
    it.atk_bonus  = 0;
    it.def_bonus  = ITEM_HELMET_DEF_BONUS;
    it.hp_restore = 0;
    it.name[0] = 'H'; it.name[1] = '\0';
    return it;
}

static item_t make_potion(void)
{
    item_t it;
    it.type       = ITEM_POTION;
    it.atk_bonus  = 0;
    it.def_bonus  = 0;
    it.hp_restore = ITEM_POTION_HP_RESTORE;
    it.name[0] = 'P'; it.name[1] = '\0';
    return it;
}

/* ── Test 1: player_init sets all equipment slots to -1 ──────────────── */

static int test_player_init_equipment_empty(void)
{
    player_t p;
    int      i;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    for (i = 0; i < EQUIP_SLOT_COUNT; i++) {
        TEST_ASSERT(p.equipment[i] == -1,
                    "all equipment slots must be -1 after player_init");
    }
    return 0;
}

/* ── Test 2: equip weapon increases atk ──────────────────────────────── */

static int test_player_equip_weapon_increases_atk(void)
{
    player_t p;
    item_t   it = make_weapon();
    int      base_atk;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    base_atk = p.atk;

    TEST_ASSERT(player_equip(&p, 0) == 0, "player_equip must return 0");
    TEST_ASSERT(p.atk == base_atk + ITEM_WEAPON_ATK_BONUS,
                "equipping weapon must add atk_bonus to player.atk");
    return 0;
}

/* ── Test 3: equip armor increases def ───────────────────────────────── */

static int test_player_equip_armor_increases_def(void)
{
    player_t p;
    item_t   it = make_armor();
    int      base_def;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    base_def = p.def;

    TEST_ASSERT(player_equip(&p, 0) == 0, "player_equip must return 0");
    TEST_ASSERT(p.def == base_def + ITEM_ARMOR_DEF_BONUS,
                "equipping armor must add def_bonus to player.def");
    return 0;
}

/* ── Test 4: equip helmet increases def ──────────────────────────────── */

static int test_player_equip_helmet_increases_def(void)
{
    player_t p;
    item_t   it = make_helmet();
    int      base_def;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    base_def = p.def;

    TEST_ASSERT(player_equip(&p, 0) == 0, "player_equip must return 0");
    TEST_ASSERT(p.def == base_def + ITEM_HELMET_DEF_BONUS,
                "equipping helmet must add def_bonus to player.def");
    return 0;
}

/* ── Test 5: equip weapon sets EQUIP_SLOT_WEAPON ─────────────────────── */

static int test_player_equip_sets_weapon_slot(void)
{
    player_t p;
    item_t   it = make_weapon();

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    TEST_ASSERT(player_equip(&p, 0) == 0, "player_equip must return 0");
    TEST_ASSERT(p.equipment[EQUIP_SLOT_WEAPON] == 0,
                "equipment[EQUIP_SLOT_WEAPON] must equal the inventory index");
    return 0;
}

/* ── Test 6: equip armor sets EQUIP_SLOT_BODY ────────────────────────── */

static int test_player_equip_sets_body_slot(void)
{
    player_t p;
    item_t   it = make_armor();

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    TEST_ASSERT(player_equip(&p, 0) == 0, "player_equip must return 0");
    TEST_ASSERT(p.equipment[EQUIP_SLOT_BODY] == 0,
                "equipment[EQUIP_SLOT_BODY] must equal the inventory index");
    return 0;
}

/* ── Test 7: equip helmet sets EQUIP_SLOT_HEAD ───────────────────────── */

static int test_player_equip_sets_head_slot(void)
{
    player_t p;
    item_t   it = make_helmet();

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    TEST_ASSERT(player_equip(&p, 0) == 0, "player_equip must return 0");
    TEST_ASSERT(p.equipment[EQUIP_SLOT_HEAD] == 0,
                "equipment[EQUIP_SLOT_HEAD] must equal the inventory index");
    return 0;
}

/* ── Test 8: out-of-range index returns -1 ───────────────────────────── */

static int test_player_equip_invalid_index_returns_error(void)
{
    player_t p;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    /* No items in inventory: inv_idx 0 is invalid */
    TEST_ASSERT(player_equip(&p, 0) == -1,
                "player_equip with out-of-range index must return -1");
    TEST_ASSERT(player_equip(&p, -1) == -1,
                "player_equip with negative index must return -1");
    return 0;
}

/* ── Test 9: equip ITEM_POTION returns -1 ────────────────────────────── */

static int test_player_equip_potion_returns_error(void)
{
    player_t p;
    item_t   it = make_potion();

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    TEST_ASSERT(player_equip(&p, 0) == -1,
                "player_equip of ITEM_POTION must return -1");
    return 0;
}

/* ── Test 10: unequip weapon restores atk ────────────────────────────── */

static int test_player_unequip_weapon_restores_atk(void)
{
    player_t p;
    item_t   it = make_weapon();
    int      base_atk;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    base_atk = p.atk;
    player_equip(&p, 0);

    TEST_ASSERT(player_unequip(&p, EQUIP_SLOT_WEAPON) == 0,
                "player_unequip must return 0");
    TEST_ASSERT(p.atk == base_atk,
                "unequipping weapon must restore player.atk to original value");
    TEST_ASSERT(p.equipment[EQUIP_SLOT_WEAPON] == -1,
                "EQUIP_SLOT_WEAPON must be -1 after unequip");
    return 0;
}

/* ── Test 11: unequip armor restores def ─────────────────────────────── */

static int test_player_unequip_armor_restores_def(void)
{
    player_t p;
    item_t   it = make_armor();
    int      base_def;

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &it);
    base_def = p.def;
    player_equip(&p, 0);

    TEST_ASSERT(player_unequip(&p, EQUIP_SLOT_BODY) == 0,
                "player_unequip must return 0");
    TEST_ASSERT(p.def == base_def,
                "unequipping armor must restore player.def to original value");
    TEST_ASSERT(p.equipment[EQUIP_SLOT_BODY] == -1,
                "EQUIP_SLOT_BODY must be -1 after unequip");
    return 0;
}

/* ── Test 12: equip second weapon replaces first ─────────────────────── */

static int test_player_equip_replaces_existing_weapon(void)
{
    player_t p;
    item_t   w1 = make_weapon();
    item_t   w2;
    int      base_atk;

    /* Second weapon with a different bonus */
    w2.type       = ITEM_WEAPON;
    w2.atk_bonus  = ITEM_WEAPON_ATK_BONUS + 3; /* bigger bonus */
    w2.def_bonus  = 0;
    w2.hp_restore = 0;
    w2.name[0] = 'X'; w2.name[1] = '\0';

    TEST_ASSERT(player_init(&p) == 0, "player_init should return 0");
    player_add_item(&p, &w1);
    player_add_item(&p, &w2);
    base_atk = p.atk;

    /* Equip first weapon (inv slot 0) */
    player_equip(&p, 0);
    /* Equip second weapon (inv slot 1) — should replace slot 0 item */
    TEST_ASSERT(player_equip(&p, 1) == 0,
                "equipping second weapon must return 0");
    TEST_ASSERT(p.equipment[EQUIP_SLOT_WEAPON] == 1,
                "weapon slot must now hold inventory index 1");
    TEST_ASSERT(p.atk == base_atk + w2.atk_bonus,
                "player.atk must equal base + second weapon bonus only");
    return 0;
}

/* ── Test 13: monster attack reduced by player.def ──────────────────────*/

static int test_monster_damage_reduced_by_def(void)
{
    game_state_t state;
    int          hp_before;
    int          expected_damage;

    turn_manager_init(&state);
    state.player.def = 2; /* manually set def */

    /* Spawn monster directly above player and make it attack */
    turn_manager_spawn_monster(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 1);
    state.monsters[0].x = state.player.x;
    state.monsters[0].y = state.player.y + 1; /* one below player */
    map_set_tile(&state.map, state.monsters[0].x, state.monsters[0].y,
                 TILE_MONSTER);

    hp_before       = state.player.hp;
    expected_damage = state.monsters[0].atk - state.player.def;

    monster_step(&state.monsters[0], &state.player, &state.map);

    TEST_ASSERT(state.player.hp == hp_before - expected_damage,
                "monster damage must be reduced by player.def");
    return 0;
}

/* ── Test 14: monster damage cannot go below 0 ───────────────────────── */

static int test_monster_damage_not_below_zero(void)
{
    game_state_t state;
    int          hp_before;

    turn_manager_init(&state);
    /* def >= monster atk → 0 damage */
    state.player.def = state.monsters[0].atk + 99;

    turn_manager_spawn_monster(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 1);
    state.monsters[0].x = state.player.x;
    state.monsters[0].y = state.player.y + 1;
    map_set_tile(&state.map, state.monsters[0].x, state.monsters[0].y,
                 TILE_MONSTER);

    hp_before = state.player.hp;
    monster_step(&state.monsters[0], &state.player, &state.map);

    TEST_ASSERT(state.player.hp == hp_before,
                "monster damage must not reduce hp when def >= atk (0 damage)");
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

    printf("=== test_equipment ===\n");
    RUN(test_player_init_equipment_empty);
    RUN(test_player_equip_weapon_increases_atk);
    RUN(test_player_equip_armor_increases_def);
    RUN(test_player_equip_helmet_increases_def);
    RUN(test_player_equip_sets_weapon_slot);
    RUN(test_player_equip_sets_body_slot);
    RUN(test_player_equip_sets_head_slot);
    RUN(test_player_equip_invalid_index_returns_error);
    RUN(test_player_equip_potion_returns_error);
    RUN(test_player_unequip_weapon_restores_atk);
    RUN(test_player_unequip_armor_restores_def);
    RUN(test_player_equip_replaces_existing_weapon);
    RUN(test_monster_damage_reduced_by_def);
    RUN(test_monster_damage_not_below_zero);

    printf("Result: %d passed, %d failed\n", passed, failed);
    return (failed > 0) ? 1 : 0;
}
