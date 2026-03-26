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
    /* id=0  WEAPON  Short Sword */
    { ITEM_WEAPON, 5,  0,  0,  "Short Sword" },
    /* id=1  WEAPON  Battle Axe  */
    { ITEM_WEAPON, 8,  0,  0,  "Battle Axe"  },
    /* id=2  ARMOR   Chain Mail  */
    { ITEM_ARMOR,  0,  3,  0,  "Chain Mail"  },
    /* id=3  ARMOR   Plate Mail  */
    { ITEM_ARMOR,  0,  5,  0,  "Plate Mail"  },
    /* id=4  HELMET  Iron Helm   */
    { ITEM_HELMET, 0,  2,  0,  "Iron Helm"   },
    /* id=5  HELMET  Great Helm  */
    { ITEM_HELMET, 0,  4,  0,  "Great Helm"  },
    /* id=6  POTION  Health Pot  */
    { ITEM_POTION, 0,  0, 20,  "Health Pot"  },
    /* id=7  POTION  Elixir      */
    { ITEM_POTION, 0,  0, 40,  "Elixir"      }
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
