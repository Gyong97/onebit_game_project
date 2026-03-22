/**
 * @file player.h
 * @brief Player entity interface for the OneBit roguelike.
 *
 * Defines the player data structure and movement API.
 * All game-logic rules (wall collision, viewport scroll trigger,
 * no-backtrack enforcement) live in player.c; this header is
 * purely the contract.
 *
 * Movement return codes:
 *   0  — player successfully moved (or scroll occurred at top)
 *   1  — move was blocked (wall, viewport boundary)
 *  -1  — error (NULL pointer argument)
 */
#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"    /* map_t, MAP_WIDTH, VIEWPORT_H */
#include "input.h"  /* action_t */

/* ── Initial player stats (per game spec) ────────────────────────────── */
#define PLAYER_INIT_HP    100
#define PLAYER_INIT_MAXHP 100
#define PLAYER_INIT_ATK    10

/* Starting position: horizontally centred, second row from bottom */
#define PLAYER_INIT_X  4
#define PLAYER_INIT_Y  (VIEWPORT_H - 2)

/**
 * @brief All data owned by the player entity.
 */
typedef struct {
    int x;       /* column in viewport [0, MAP_WIDTH)  */
    int y;       /* row    in viewport [0, VIEWPORT_H) */
    int hp;      /* current hit points                 */
    int max_hp;  /* maximum hit points                 */
    int atk;     /* attack power                       */
} player_t;

/* ── Player API ───────────────────────────────────────────────────────── */

/**
 * @brief Initialise a player with default stats and starting position.
 *
 * @param p_player  Output player to initialise; must not be NULL.
 * @return 0 on success, -1 if p_player is NULL.
 */
int player_init(player_t *p_player);

/**
 * @brief Attempt to move the player one cell in the given direction.
 *
 * Rules applied in order:
 *  1. Moving up from y=0  → map_scroll(p_map), player stays at y=0 (return 0).
 *  2. Moving down from y=VIEWPORT_H-1 → blocked, no backtracking (return 1).
 *  3. Target tile = TILE_WALL → blocked (return 1).
 *  4. Target tile = TILE_FLOOR → player moves to (new_x, new_y) (return 0).
 *
 * @param p_player  Player to move; must not be NULL.
 * @param action    Directional action (ACTION_MOVE_*).
 * @param p_map     Current map; must not be NULL.
 * @return 0 (moved/scrolled), 1 (blocked), -1 (error).
 */
int player_move(player_t *p_player, action_t action, map_t *p_map);

#endif /* PLAYER_H */
