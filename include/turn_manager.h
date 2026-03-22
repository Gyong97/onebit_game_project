/**
 * @file turn_manager.h
 * @brief Turn orchestrator for the OneBit roguelike.
 *
 * game_state_t is the single authoritative game state: it owns the map,
 * player, and monster pool.  All turn processing routes through
 * turn_manager_player_act(), which chains:
 *
 *   player acts → [if scroll] monster shift + spawn → monsters act
 *
 * Callers (main game loop) only need turn_manager_init() to set up and
 * turn_manager_player_act() each input cycle.  The render layer reads
 * game_state_t.map and entity fields to build render_frame_t.
 *
 * Return codes for turn_manager_player_act():
 *   0               — player acted, game continues normally
 *   1               — player was blocked; no monster turn took place
 *   TURN_GAME_OVER  — player's HP reached 0 after monsters acted
 *  -1               — error (NULL argument or internal failure)
 */
#ifndef TURN_MANAGER_H
#define TURN_MANAGER_H

/* ── Turn result codes ────────────────────────────────────────────────── */
#define TURN_GAME_OVER   2   /* player HP reached 0 — game ends           */

/* ── Spawn constants ─────────────────────────────────────────────────── */
#define CHEST_SPAWN_PCT  15  /* % chance per interior column on new row   */
#define CHEST_HP_REWARD  20  /* HP healed when opening a chest            */
#define CHEST_ATK_REWARD  5  /* ATK gained when opening a chest           */
#define COIN_SPAWN_PCT   15  /* % chance per floor cell to spawn a coin   */

#include "monster.h"       /* monster_t, MONSTER_MAX_COUNT */
#include "player.h"        /* player_t */
#include "map.h"           /* map_t */
#include "input.h"         /* action_t */

/**
 * @brief Complete game state — the single source of truth.
 */
typedef struct {
    map_t     map;                          /* viewport tile grid */
    player_t  player;                       /* player entity      */
    monster_t monsters[MONSTER_MAX_COUNT];  /* monster pool       */
} game_state_t;

/* ── Turn manager API ─────────────────────────────────────────────────── */

/**
 * @brief Initialise a fresh game state.
 *
 * Calls map_init(), player_init(), clears the monster pool, and places
 * TILE_PLAYER at the player's starting position in the map.
 *
 * @param p_state  Output state to initialise; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int turn_manager_init(game_state_t *p_state);

/**
 * @brief Process one full turn for the player and (if applicable) monsters.
 *
 * Flow:
 *  1. Call player_move() with the given action.
 *  2. If player was BLOCKED (player_move returns 1): return 1 immediately.
 *  3. If player ACTED (player_move returns 0):
 *       a. Detect scroll: if scroll_count increased, call
 *          turn_manager_shift_monsters() then turn_manager_spawn_row().
 *       b. Call turn_manager_monsters_act().
 *       c. Return 0.
 *
 * @param p_state  Current game state; must not be NULL.
 * @param action   Player input to process.
 * @return 0 (player acted), 1 (player blocked), -1 (error).
 */
int turn_manager_player_act(game_state_t *p_state, action_t action);

/**
 * @brief Execute one step for every alive monster.
 *
 * Iterates the monster pool and calls monster_step() for each alive slot.
 *
 * @param p_state  Current game state; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int turn_manager_monsters_act(game_state_t *p_state);

/**
 * @brief Shift all alive monsters down by one row (called after scroll).
 *
 * For each alive monster:
 *  - Clears TILE_MONSTER at current (x, y).
 *  - Increments y.
 *  - If new y >= VIEWPORT_H: marks monster dead (alive = 0).
 *  - Otherwise: places TILE_MONSTER at new (x, y).
 *
 * @param p_state  Current game state; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int turn_manager_shift_monsters(game_state_t *p_state);

/**
 * @brief Attempt to spawn a single monster at (x, y).
 *
 * Finds the first free slot (alive == 0), calls monster_init(), and places
 * TILE_MONSTER on the map.  Does nothing if the position already has a
 * non-floor tile.
 *
 * @param p_state  Game state; must not be NULL.
 * @param x        Target column [0, MAP_WIDTH).
 * @param y        Target row    [0, VIEWPORT_H).
 * @return 0 (spawned), 1 (pool full or cell occupied), -1 (error).
 */
int turn_manager_spawn_monster(game_state_t *p_state, int x, int y);

/**
 * @brief Randomly spawn monsters across the new top row (y = 0).
 *
 * For each interior column (x = 1 .. MAP_WIDTH-2):
 *   if (rand() % 100 < MONSTER_SPAWN_PCT) → try turn_manager_spawn_monster().
 *
 * @param p_state  Game state; must not be NULL.
 * @return Number of monsters successfully spawned (>= 0), -1 on error.
 */
int turn_manager_spawn_row(game_state_t *p_state);

/**
 * @brief Count the number of alive monsters in the pool.
 *
 * @param p_state  Game state; must not be NULL.
 * @return Number of alive monsters, or -1 on error.
 */
int turn_manager_alive_count(const game_state_t *p_state);

/**
 * @brief Open a chest at (x, y) and give the player a random reward.
 *
 * Reward is chosen randomly:
 *   rand() % 2 == 0 → HP reward: player.hp += CHEST_HP_REWARD (capped at max_hp)
 *   rand() % 2 == 1 → ATK reward: player.atk += CHEST_ATK_REWARD
 *
 * The chest tile at (x, y) is replaced with TILE_FLOOR.
 *
 * @param p_state  Game state; must not be NULL.
 * @param x        Column of the chest.
 * @param y        Row    of the chest.
 * @return 0 on success, -1 on error.
 */
int turn_manager_open_chest(game_state_t *p_state, int x, int y);

#endif /* TURN_MANAGER_H */
