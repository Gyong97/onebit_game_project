/**
 * @file monster_db.h
 * @brief Monster dictionary (database) for the OneBit roguelike.
 *
 * Provides a static, type-indexed table of all monster templates.
 * Callers use monster_db_get() to look up a monster definition by its
 * monster_type_t value.  Adding a new monster species requires only:
 *   1. A new entry in the monster_type_t enum (monster.h).
 *   2. A new row in the g_monster_table (monster_db.c).
 *   3. An entry in docs/game_monster.md.
 *
 * monster_db_get() return codes:
 *   0  — success; *p_out is populated
 *  -1  — error (type out of range or p_out is NULL)
 */
#ifndef MONSTER_DB_H
#define MONSTER_DB_H

#include "monster.h"  /* monster_type_t, MONSTER_TYPE_COUNT, GOBLIN_INIT_* etc. */

/* ── Database constants ───────────────────────────────────────────────── */
#define MONSTER_NAME_MAX 16  /* max characters in a monster name (incl. '\0') */

/**
 * @brief Static definition of one monster species.
 *
 * base_hp and base_atk are the un-scaled starting values.
 * Depth-scaling is applied by turn_manager at spawn time.
 */
typedef struct {
    monster_type_t  type;
    char            name[MONSTER_NAME_MAX];
    int             base_hp;
    int             base_atk;
} monster_def_t;

/* ── Monster DB API ───────────────────────────────────────────────────── */

/**
 * @brief Return the number of monster types in the database.
 * @return MONSTER_TYPE_COUNT (always).
 */
int monster_db_count(void);

/**
 * @brief Look up one monster definition by its type enum value.
 *
 * @param type   Monster species [0, MONSTER_TYPE_COUNT).
 * @param p_out  Output definition; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int monster_db_get(monster_type_t type, monster_def_t *p_out);

#endif /* MONSTER_DB_H */
