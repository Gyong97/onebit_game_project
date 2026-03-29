/**
 * @file item_db.c
 * @brief Static item database implementation for the OneBit roguelike.
 *
 * Eight item entries cover all four item types (WEAPON, ARMOR, HELMET,
 * POTION).  Each entry has exactly one non-zero bonus field matching its
 * type, per item.h contract.
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#include <stddef.h>   /* NULL   */
#include <string.h>   /* strncpy */
#include "item_db.h"

/* ── Static item table ────────────────────────────────────────────────── */

static const item_t g_item_table[ITEM_DB_COUNT] = {
    /* id=0  WEAPON  Short Sword  — atk+5, buy 8 */
    { ITEM_WEAPON, 5, 0,  0,  "Short Sword",  8 },
    /* id=1  WEAPON  Battle Axe   — atk+8, buy 12 */
    { ITEM_WEAPON, 8, 0,  0,  "Battle Axe",  12 },
    /* id=2  ARMOR   Chain Mail   — def+3, buy 6 */
    { ITEM_ARMOR,  0, 3,  0,  "Chain Mail",   6 },
    /* id=3  ARMOR   Plate Mail   — def+5, buy 10 */
    { ITEM_ARMOR,  0, 5,  0,  "Plate Mail",  10 },
    /* id=4  HELMET  Iron Helm    — def+2, buy 5 */
    { ITEM_HELMET, 0, 2,  0,  "Iron Helm",    5 },
    /* id=5  HELMET  Great Helm   — def+4, buy 8 */
    { ITEM_HELMET, 0, 4,  0,  "Great Helm",   8 },
    /* id=6  POTION  Health Pot   — hp+20, buy 5 */
    { ITEM_POTION, 0, 0, 20,  "Health Pot",   5 },
    /* id=7  POTION  Elixir       — hp+40, buy 9 */
    { ITEM_POTION, 0, 0, 40,  "Elixir",       9 }
};

/* ── Public API ───────────────────────────────────────────────────────── */

int item_db_count(void)
{
    return ITEM_DB_COUNT;
}

int item_db_get(int id, item_t *p_out)
{
    if (p_out == NULL) {
        return -1;
    }
    if (id < 0 || id >= ITEM_DB_COUNT) {
        return -1;
    }
    *p_out = g_item_table[id];
    return 0;
}
