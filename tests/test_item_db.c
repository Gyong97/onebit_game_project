/**
 * @file test_item_db.c
 * @brief TDD Red: item database module tests.
 *
 * Tests:
 *  1. item_db_count() returns ITEM_DB_COUNT
 *  2. item_db_get() with a valid ID returns 0
 *  3. item_db_get() out-of-range returns -1
 *  4. item_db_get() with NULL output returns -1
 *  5. All items have a non-empty name
 *  6. ITEM_WEAPON entries have atk_bonus > 0, others 0
 *  7. ITEM_ARMOR and ITEM_HELMET entries have def_bonus > 0, others 0
 *  8. ITEM_POTION entries have hp_restore > 0, others 0
 *  9. turn_manager_open_chest uses item_db (item type is always valid)
 * 10. open_chest item_db items cover all 4 types across the table
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "item_db.h"
#include "turn_manager.h"

/* ── 1. item_db_count returns ITEM_DB_COUNT ───────────────────────────── */
static void test_count(void)
{
    assert(item_db_count() == ITEM_DB_COUNT);
    printf("[PASS] item_db_count: returns ITEM_DB_COUNT (%d)\n",
           ITEM_DB_COUNT);
}

/* ── 2. item_db_get valid IDs return 0 ───────────────────────────────── */
static void test_get_valid(void)
{
    int    i;
    item_t out;
    for (i = 0; i < ITEM_DB_COUNT; i++) {
        assert(item_db_get(i, &out) == 0);
    }
    printf("[PASS] item_db_get: all %d IDs return 0\n", ITEM_DB_COUNT);
}

/* ── 3. item_db_get out-of-range returns -1 ──────────────────────────── */
static void test_get_oob(void)
{
    item_t out;
    assert(item_db_get(-1,          &out) == -1);
    assert(item_db_get(ITEM_DB_COUNT, &out) == -1);
    printf("[PASS] item_db_get: out-of-range returns -1\n");
}

/* ── 4. item_db_get NULL output returns -1 ───────────────────────────── */
static void test_get_null(void)
{
    assert(item_db_get(0, NULL) == -1);
    printf("[PASS] item_db_get: NULL output returns -1\n");
}

/* ── 5. All items have a non-empty name ──────────────────────────────── */
static void test_all_have_names(void)
{
    int    i;
    item_t out;
    for (i = 0; i < ITEM_DB_COUNT; i++) {
        item_db_get(i, &out);
        assert(out.name[0] != '\0');
    }
    printf("[PASS] item_db_get: all items have non-empty names\n");
}

/* ── 6. WEAPON items have atk_bonus > 0, others zeroed ──────────────── */
static void test_weapon_bonus(void)
{
    int    i;
    item_t out;
    for (i = 0; i < ITEM_DB_COUNT; i++) {
        item_db_get(i, &out);
        if (out.type == ITEM_WEAPON) {
            assert(out.atk_bonus  > 0);
            assert(out.def_bonus  == 0);
            assert(out.hp_restore == 0);
        }
    }
    printf("[PASS] item_db_get: weapon items have atk_bonus > 0\n");
}

/* ── 7. ARMOR / HELMET items have def_bonus > 0, others zeroed ───────── */
static void test_armor_helmet_bonus(void)
{
    int    i;
    item_t out;
    for (i = 0; i < ITEM_DB_COUNT; i++) {
        item_db_get(i, &out);
        if (out.type == ITEM_ARMOR || out.type == ITEM_HELMET) {
            assert(out.def_bonus  > 0);
            assert(out.atk_bonus  == 0);
            assert(out.hp_restore == 0);
        }
    }
    printf("[PASS] item_db_get: armor/helmet items have def_bonus > 0\n");
}

/* ── 8. POTION items have hp_restore > 0, others zeroed ─────────────── */
static void test_potion_bonus(void)
{
    int    i;
    item_t out;
    for (i = 0; i < ITEM_DB_COUNT; i++) {
        item_db_get(i, &out);
        if (out.type == ITEM_POTION) {
            assert(out.hp_restore > 0);
            assert(out.atk_bonus  == 0);
            assert(out.def_bonus  == 0);
        }
    }
    printf("[PASS] item_db_get: potion items have hp_restore > 0\n");
}

/* ── 9. open_chest always produces a valid item type ─────────────────── */
static void test_open_chest_valid_type(void)
{
    game_state_t state;
    item_type_t  t;
    int          cx;
    int          cy;
    int          i;

    /* Run multiple times to exercise the random path */
    for (i = 0; i < 20; i++) {
        turn_manager_init(&state);
        cx = state.player.x;
        cy = state.player.y - 1;
        map_set_tile(&state.map, cx, cy, TILE_CHEST);
        turn_manager_open_chest(&state, cx, cy);

        t = state.player.inventory[0].type;
        assert(t == ITEM_WEAPON || t == ITEM_ARMOR ||
               t == ITEM_HELMET || t == ITEM_POTION);
    }
    printf("[PASS] open_chest: always produces a valid item type\n");
}

/* ── 10. item_db covers all 4 item types ─────────────────────────────── */
static void test_db_covers_all_types(void)
{
    int    i;
    item_t out;
    int    has_weapon = 0, has_armor = 0, has_helmet = 0, has_potion = 0;

    for (i = 0; i < ITEM_DB_COUNT; i++) {
        item_db_get(i, &out);
        switch (out.type) {
            case ITEM_WEAPON: has_weapon  = 1; break;
            case ITEM_ARMOR:  has_armor   = 1; break;
            case ITEM_HELMET: has_helmet  = 1; break;
            case ITEM_POTION: has_potion  = 1; break;
            default: break;
        }
    }
    assert(has_weapon && has_armor && has_helmet && has_potion);
    printf("[PASS] item_db: covers WEAPON, ARMOR, HELMET, POTION types\n");
}

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void)
{
    printf("=== test_item_db ===\n");
    test_count();
    test_get_valid();
    test_get_oob();
    test_get_null();
    test_all_have_names();
    test_weapon_bonus();
    test_armor_helmet_bonus();
    test_potion_bonus();
    test_open_chest_valid_type();
    test_db_covers_all_types();
    printf("=== All item_db tests passed. ===\n");
    return 0;
}
