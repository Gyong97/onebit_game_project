/**
 * @file playable_db.h
 * @brief Playable character class dictionary for the OneBit roguelike.
 *
 * Provides a static, type-indexed table of all playable class templates.
 * Callers use playable_db_get() to look up a class definition by its
 * playable_type_t value.  Adding a new class requires only:
 *   1. A new entry in the playable_type_t enum.
 *   2. A new row in the g_playable_table (playable_db.c).
 *   3. An entry in docs/game_playable.md.
 *
 * playable_db_get() return codes:
 *   0  — success; *p_out is populated
 *  -1  — error (type out of range or p_out is NULL)
 */
#ifndef PLAYABLE_DB_H
#define PLAYABLE_DB_H

/* ── Warrior (default class) base stats ──────────────────────────────── */
#define WARRIOR_INIT_HP     100  /* starting HP                */
#define WARRIOR_INIT_MAXHP  100  /* starting maximum HP        */
#define WARRIOR_INIT_ATK     10  /* starting attack power      */
#define WARRIOR_INIT_DEF      0  /* starting defense (no armor)*/

/* ── Database constants ───────────────────────────────────────────────── */
#define PLAYABLE_NAME_MAX 16  /* max characters in a class name (incl. '\0') */

/**
 * @brief All playable character classes.
 *
 * PLAYABLE_TYPE_COUNT is a sentinel — always equals the number of classes.
 */
typedef enum {
    PLAYABLE_WARRIOR    = 0, /* balanced melee fighter (default) */
    PLAYABLE_TYPE_COUNT = 1  /* sentinel — number of playable classes */
} playable_type_t;

/**
 * @brief Static definition of one playable class.
 *
 * These values describe the character's starting state at the beginning
 * of every new run.
 */
typedef struct {
    playable_type_t type;
    char            name[PLAYABLE_NAME_MAX];
    int             init_hp;
    int             init_max_hp;
    int             init_atk;
    int             init_def;
} playable_def_t;

/* ── Playable DB API ──────────────────────────────────────────────────── */

/**
 * @brief Return the number of playable classes in the database.
 * @return PLAYABLE_TYPE_COUNT (always).
 */
int playable_db_count(void);

/**
 * @brief Look up one class definition by its type enum value.
 *
 * @param type   Class index [0, PLAYABLE_TYPE_COUNT).
 * @param p_out  Output definition; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int playable_db_get(playable_type_t type, playable_def_t *p_out);

#endif /* PLAYABLE_DB_H */
