/**
 * @file monster.h
 * @brief Monster entity interface for the OneBit roguelike.
 *
 * Defines the monster data structure, initial-stat constants, and the
 * single-step tracking AI.  Each monster occupies one slot in the pool
 * managed by turn_manager_t.  Slots are reused via the alive flag.
 *
 * monster_step() return codes:
 *   0  — monster acted (moved toward player, or attacked — Phase 4 damage)
 *   1  — blocked on all axes (no move possible)
 *  -1  — error (NULL argument)
 */
#ifndef MONSTER_H
#define MONSTER_H

#include "player.h"  /* player_t (read-only reference for AI targeting) */
#include "map.h"     /* map_t, tile_type_t */

/* ── Monster constants (per game spec) ───────────────────────────────── */
#define MONSTER_INIT_HP    20  /* starting hit points     */
#define MONSTER_INIT_ATK    5  /* attack power            */
#define MONSTER_MAX_COUNT   8  /* max live monsters in viewport at once */
#define MONSTER_SPAWN_PCT  30  /* % chance to spawn per interior column  */

/**
 * @brief Per-monster state.
 *
 * alive == 0 means this pool slot is free and must be ignored by callers.
 */
typedef struct {
    int x;     /* column in viewport [0, MAP_WIDTH)  */
    int y;     /* row    in viewport [0, VIEWPORT_H) */
    int hp;    /* current hit points                 */
    int atk;   /* attack power                       */
    int alive; /* non-zero while the monster is active */
} monster_t;

/* ── Monster API ──────────────────────────────────────────────────────── */

/**
 * @brief Initialise a monster slot at the given map position.
 *
 * Sets hp = MONSTER_INIT_HP, atk = MONSTER_INIT_ATK, alive = 1.
 * Does NOT write TILE_MONSTER to the map (caller's responsibility).
 *
 * @param p_monster  Output slot; must not be NULL.
 * @param x          Column to spawn at.
 * @param y          Row    to spawn at.
 * @return 0 on success, -1 if p_monster is NULL.
 */
int monster_init(monster_t *p_monster, int x, int y);

/**
 * @brief Move this monster one cell toward the player (tracking AI).
 *
 * Movement priority:
 *   - Primary axis: the one with the larger absolute distance.
 *     Horizontal wins when |dx| == |dy|.
 *   - If the primary axis is blocked (TILE_WALL / TILE_MONSTER / TILE_CHEST),
 *     the secondary axis is tried as a fallback.
 *   - If both axes are blocked the monster stays in place (return 0).
 *
 * When the target cell contains TILE_PLAYER the monster performs an
 * attack: p_player->hp -= p_monster->atk, and the monster stays in place.
 *
 * Map tiles are updated on a successful move:
 *   old position → TILE_FLOOR, new position → TILE_MONSTER.
 *
 * @param p_monster  Monster to move; must not be NULL and must be alive.
 * @param p_player   Target player (read-only); must not be NULL.
 * @param p_map      Current map (read/write); must not be NULL.
 * @return 0 (acted), 1 (blocked), -1 (error).
 */
int monster_step(monster_t *p_monster, player_t *p_player,
                 map_t *p_map);

#endif /* MONSTER_H */
