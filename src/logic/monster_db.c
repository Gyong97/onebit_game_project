/**
 * @file monster_db.c
 * @brief Monster dictionary implementation for the OneBit roguelike.
 *
 * One row per monster species.  To add a new monster:
 *   1. Add a new value to monster_type_t (before MONSTER_TYPE_COUNT).
 *   2. Define NEWTYPE_INIT_HP / NEWTYPE_INIT_ATK in monster.h.
 *   3. Add a row to g_monster_table below.
 *   4. Document the new monster in docs/game_monster.md.
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#include <stddef.h>   /* NULL   */
#include "monster_db.h"

/* ── Static monster table ─────────────────────────────────────────────── */

static const monster_def_t g_monster_table[MONSTER_TYPE_COUNT] = {
    /* GOBLIN — balanced: default monster, medium range, ground */
    { MONSTER_TYPE_GOBLIN, "Goblin", GOBLIN_INIT_HP, GOBLIN_INIT_ATK,
      GOBLIN_PERCEPTION_RANGE, 0 },
    /* SLIME  — high HP, low ATK, short range, ground           */
    { MONSTER_TYPE_SLIME,  "Slime",  SLIME_INIT_HP,  SLIME_INIT_ATK,
      SLIME_PERCEPTION_RANGE,  0 },
    /* BAT    — low HP, high ATK, wide range, flying            */
    { MONSTER_TYPE_BAT,    "Bat",    BAT_INIT_HP,    BAT_INIT_ATK,
      BAT_PERCEPTION_RANGE,    1 },
};

/* ── Public API ───────────────────────────────────────────────────────── */

int monster_db_count(void)
{
    return MONSTER_TYPE_COUNT;
}

int monster_db_get(monster_type_t type, monster_def_t *p_out)
{
    if (p_out == NULL) {
        return -1;
    }
    if ((int)type < 0 || (int)type >= MONSTER_TYPE_COUNT) {
        return -1;
    }
    *p_out = g_monster_table[(int)type];
    return 0;
}
