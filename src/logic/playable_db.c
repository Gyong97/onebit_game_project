/**
 * @file playable_db.c
 * @brief Playable class dictionary implementation for the OneBit roguelike.
 *
 * One row per class.  To add a new class:
 *   1. Add a new value to playable_type_t (before PLAYABLE_TYPE_COUNT).
 *   2. Add a row to g_playable_table below.
 *   3. Define CLASSNAME_INIT_* constants in playable_db.h.
 *   4. Document the new class in docs/game_playable.md.
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#include <stddef.h>   /* NULL   */
#include <string.h>   /* strncpy */
#include "playable_db.h"

/* ── Static class table ───────────────────────────────────────────────── */

static const playable_def_t g_playable_table[PLAYABLE_TYPE_COUNT] = {
    /* WARRIOR — balanced melee fighter, the default starting class */
    {
        PLAYABLE_WARRIOR,
        "Warrior",
        WARRIOR_INIT_HP,
        WARRIOR_INIT_MAXHP,
        WARRIOR_INIT_ATK,
        WARRIOR_INIT_DEF
    }
};

/* ── Public API ───────────────────────────────────────────────────────── */

int playable_db_count(void)
{
    return PLAYABLE_TYPE_COUNT;
}

int playable_db_get(playable_type_t type, playable_def_t *p_out)
{
    if (p_out == NULL) {
        return -1;
    }
    if ((int)type < 0 || (int)type >= PLAYABLE_TYPE_COUNT) {
        return -1;
    }
    *p_out = g_playable_table[(int)type];
    return 0;
}
