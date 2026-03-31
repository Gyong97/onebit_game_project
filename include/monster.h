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

/* ── Monster type enum ────────────────────────────────────────────────── */

/**
 * @brief Distinct monster species.
 *
 * MONSTER_TYPE_COUNT is a sentinel — always equals the number of types.
 */
typedef enum {
    MONSTER_TYPE_GOBLIN = 0,  /* balanced: default monster            */
    MONSTER_TYPE_SLIME  = 1,  /* high HP, low ATK                     */
    MONSTER_TYPE_BAT    = 2,  /* low HP,  high ATK                    */
    MONSTER_TYPE_COUNT  = 3   /* sentinel — number of monster types   */
} monster_type_t;

/* ── Per-type base stats ──────────────────────────────────────────────── */
#define GOBLIN_INIT_HP   20  /* goblin starting HP  */
#define GOBLIN_INIT_ATK   5  /* goblin starting ATK */

#define SLIME_INIT_HP    40  /* slime starting HP   (> GOBLIN)  */
#define SLIME_INIT_ATK    3  /* slime starting ATK  (< GOBLIN)  */

#define BAT_INIT_HP      10  /* bat starting HP     (< GOBLIN)  */
#define BAT_INIT_ATK     10  /* bat starting ATK    (> GOBLIN)  */

/* ── Per-type perception ranges (Manhattan distance threshold) ────────── */
#define GOBLIN_PERCEPTION_RANGE  6  /* goblin starts chasing at ≤6 tiles  */
#define SLIME_PERCEPTION_RANGE   4  /* slime has shorter perception range  */
#define BAT_PERCEPTION_RANGE     8  /* bat has widest perception range     */

/* ── Monster AI state ────────────────────────────────────────────────── */
typedef enum {
    MONSTER_STATE_IDLE    = 0,  /* player outside range: random walk */
    MONSTER_STATE_CHASING = 1   /* player inside range:  BFS chase   */
} monster_state_t;

/* ── Legacy aliases (kept for existing code that uses MONSTER_INIT_*) ── */
#define MONSTER_INIT_HP    GOBLIN_INIT_HP
#define MONSTER_INIT_ATK   GOBLIN_INIT_ATK

/* ── Monster pool constants ───────────────────────────────────────────── */
#define MONSTER_MAX_COUNT   8  /* max live monsters in viewport at once */
#define MONSTER_SPAWN_PCT  30  /* % chance to spawn per interior column  */

/**
 * @brief Per-monster state.
 *
 * alive == 0 means this pool slot is free and must be ignored by callers.
 */
typedef struct {
    int             x;               /* column in viewport [0, MAP_WIDTH)       */
    int             y;               /* row    in viewport [0, VIEWPORT_H)      */
    int             hp;              /* current hit points                      */
    int             max_hp;          /* maximum HP — set at spawn (after scale) */
    int             atk;             /* attack power                            */
    int             alive;           /* non-zero while the monster is active    */
    monster_type_t  type;            /* species of this monster                 */
    monster_state_t state;           /* AI state: IDLE or CHASING               */
    int             can_fly;         /* 1 = ignores walls during movement       */
    int             perception_range;/* Manhattan distance chase threshold       */
    tile_type_t     tile_under;      /* tile beneath this monster (restored on move) */
} monster_t;

/* ── Monster API ──────────────────────────────────────────────────────── */

/**
 * @brief Initialise a monster slot as a GOBLIN at the given map position.
 *
 * Sets hp = GOBLIN_INIT_HP, atk = GOBLIN_INIT_ATK, type = MONSTER_TYPE_GOBLIN,
 * alive = 1.  Does NOT write TILE_MONSTER to the map (caller's responsibility).
 *
 * Equivalent to monster_init_typed(p_monster, x, y, MONSTER_TYPE_GOBLIN).
 *
 * @param p_monster  Output slot; must not be NULL.
 * @param x          Column to spawn at.
 * @param y          Row    to spawn at.
 * @return 0 on success, -1 if p_monster is NULL.
 */
int monster_init(monster_t *p_monster, int x, int y);

/**
 * @brief Initialise a monster slot of the given type at the given position.
 *
 * Selects base stats according to the type:
 *   GOBLIN → GOBLIN_INIT_HP / GOBLIN_INIT_ATK
 *   SLIME  → SLIME_INIT_HP  / SLIME_INIT_ATK
 *   BAT    → BAT_INIT_HP    / BAT_INIT_ATK
 *
 * Does NOT write TILE_MONSTER to the map (caller's responsibility).
 *
 * @param p_monster  Output slot; must not be NULL.
 * @param x          Column to spawn at.
 * @param y          Row    to spawn at.
 * @param type       Monster species to initialise.
 * @return 0 on success, -1 if p_monster is NULL.
 */
int monster_init_typed(monster_t *p_monster, int x, int y,
                       monster_type_t type);

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
