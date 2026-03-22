/**
 * @file turn_manager.c
 * @brief Turn orchestrator implementation for the OneBit roguelike.
 *
 * Owns the complete game-turn pipeline:
 *   player_act → [scroll detected] shift+spawn → monsters_act
 *
 * All entity-to-map tile synchronisation for player happens in player.c.
 * Monster tile synchronisation happens in monster.c (monster_step) and
 * here for spawn/shift/removal operations.
 *
 * Uses rand() for procedural monster spawning; seed is set in
 * turn_manager_init() via srand(time(NULL)).
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#include <stddef.h>   /* NULL   */
#include <stdlib.h>   /* rand(), srand() */
#include <time.h>     /* time() */
#include <string.h>   /* memset */
#include "turn_manager.h"

/* ── Internal helpers ─────────────────────────────────────────────────── */

/**
 * @brief Find the first free monster slot (alive == 0).
 * @return Index [0, MONSTER_MAX_COUNT) or -1 if pool is full.
 */
static int find_free_slot(const game_state_t *p_state)
{
    int i;
    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (!p_state->monsters[i].alive) {
            return i;
        }
    }
    return -1; /* pool full */
}

/* ── Public API ───────────────────────────────────────────────────────── */

int turn_manager_init(game_state_t *p_state)
{
    int i;

    if (p_state == NULL) {
        return -1;
    }

    if (map_init(&p_state->map) != 0)    return -1;
    if (player_init(&p_state->player) != 0) return -1;

    /* Place player tile on the map */
    map_set_tile(&p_state->map,
                 p_state->player.x,
                 p_state->player.y,
                 TILE_PLAYER);

    /* Clear all monster pool slots */
    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        memset(&p_state->monsters[i], 0, sizeof(monster_t));
        /* alive == 0 from memset */
    }

    srand((unsigned int)time(NULL));
    return 0;
}

int turn_manager_player_act(game_state_t *p_state, action_t action)
{
    long prev_scroll;
    int  move_result;

    if (p_state == NULL) {
        return -1;
    }

    prev_scroll = p_state->map.scroll_count;
    move_result = player_move(&p_state->player, action, &p_state->map);

    if (move_result == -1) {
        return -1; /* internal error */
    }
    if (move_result == 1) {
        return 1;  /* player blocked — no monster turn */
    }

    /* Player acted (moved or scrolled) */
    if (p_state->map.scroll_count > prev_scroll) {
        /* Scroll detected: shift all monsters down, spawn new ones */
        if (turn_manager_shift_monsters(p_state) != 0) return -1;
        if (turn_manager_spawn_row(p_state) < 0)      return -1;
    }

    /* Trigger monster turn */
    if (turn_manager_monsters_act(p_state) != 0) {
        return -1;
    }

    return 0;
}

int turn_manager_monsters_act(game_state_t *p_state)
{
    int i;

    if (p_state == NULL) {
        return -1;
    }

    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (!p_state->monsters[i].alive) {
            continue;
        }
        if (monster_step(&p_state->monsters[i],
                         &p_state->player,
                         &p_state->map) == -1) {
            return -1;
        }
    }
    return 0;
}

int turn_manager_shift_monsters(game_state_t *p_state)
{
    int i;

    if (p_state == NULL) {
        return -1;
    }

    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (!p_state->monsters[i].alive) {
            continue;
        }

        /* Clear tile at current position */
        map_set_tile(&p_state->map,
                     p_state->monsters[i].x,
                     p_state->monsters[i].y,
                     TILE_FLOOR);

        /* Shift monster down one row */
        p_state->monsters[i].y++;

        if (p_state->monsters[i].y >= VIEWPORT_H) {
            /* Monster fell off the bottom of the viewport — remove it */
            p_state->monsters[i].alive = 0;
        } else {
            /* Place tile at new position */
            map_set_tile(&p_state->map,
                         p_state->monsters[i].x,
                         p_state->monsters[i].y,
                         TILE_MONSTER);
        }
    }
    return 0;
}

int turn_manager_spawn_monster(game_state_t *p_state, int x, int y)
{
    int slot;
    tile_type_t tile;

    if (p_state == NULL) {
        return -1;
    }

    /* Validate target position */
    if (map_get_tile(&p_state->map, x, y, &tile) != 0) {
        return -1; /* out-of-bounds */
    }

    /* Only spawn on empty floor */
    if (tile != TILE_FLOOR) {
        return 1; /* cell occupied */
    }

    slot = find_free_slot(p_state);
    if (slot == -1) {
        return 1; /* monster pool is full */
    }

    monster_init(&p_state->monsters[slot], x, y);
    map_set_tile(&p_state->map, x, y, TILE_MONSTER);
    return 0;
}

int turn_manager_spawn_row(game_state_t *p_state)
{
    int x;
    int count;

    if (p_state == NULL) {
        return -1;
    }

    count = 0;
    for (x = 1; x < MAP_WIDTH - 1; x++) {
        if (rand() % 100 < MONSTER_SPAWN_PCT) {
            if (turn_manager_spawn_monster(p_state, x, 0) == 0) {
                count++;
            }
        }
    }
    return count;
}

int turn_manager_open_chest(game_state_t *p_state, int x, int y)
{
    if (p_state == NULL) {
        return -1;
    }
    /* Stub: Phase 4 Green implements reward and tile removal */
    (void)x;
    (void)y;
    return 0;
}

int turn_manager_alive_count(const game_state_t *p_state)
{
    int i;
    int count;

    if (p_state == NULL) {
        return -1;
    }

    count = 0;
    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (p_state->monsters[i].alive) {
            count++;
        }
    }
    return count;
}
