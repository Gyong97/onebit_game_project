/**
 * @file player.c
 * @brief Player entity implementation for the OneBit roguelike.
 *
 * Handles initialisation and movement including:
 *   - WASD directional movement within the viewport
 *   - Wall tile collision (blocked → return 1)
 *   - No-backtrack enforcement at the viewport bottom (blocked → return 1)
 *   - Scroll trigger when moving up from y=0 (map_scroll + return 0)
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#include <stddef.h>  /* NULL */
#include "player.h"

/* ── Public API ───────────────────────────────────────────────────────── */

int player_init(player_t *p_player)
{
    if (p_player == NULL) {
        return -1;
    }
    p_player->hp     = PLAYER_INIT_HP;
    p_player->max_hp = PLAYER_INIT_MAXHP;
    p_player->atk    = PLAYER_INIT_ATK;
    p_player->x      = PLAYER_INIT_X;
    p_player->y      = PLAYER_INIT_Y;
    return 0;
}

int player_move(player_t *p_player, action_t action, map_t *p_map)
{
    int new_x;
    int new_y;
    tile_type_t tile;

    if (p_player == NULL || p_map == NULL) {
        return -1;
    }

    new_x = p_player->x;
    new_y = p_player->y;

    switch (action) {
        case ACTION_MOVE_UP:    new_y--; break;
        case ACTION_MOVE_DOWN:  new_y++; break;
        case ACTION_MOVE_LEFT:  new_x--; break;
        case ACTION_MOVE_RIGHT: new_x++; break;
        default:
            return 1; /* unrecognised action: no movement */
    }

    /*
     * Rule 1: moving up past the top of the viewport triggers a map scroll.
     * Player stays at y=0 on the freshly generated row.
     */
    if (new_y < 0) {
        return map_scroll(p_map); /* 0 on success, -1 on error */
    }

    /*
     * Rule 2: moving down past the bottom of the viewport is forbidden.
     * Those rows have already scrolled off and no longer exist.
     */
    if (new_y >= VIEWPORT_H) {
        return 1; /* blocked */
    }

    /* Horizontal out-of-bounds guard (walls cover these, but be safe) */
    if (new_x < 0 || new_x >= MAP_WIDTH) {
        return 1; /* blocked */
    }

    /* Rule 3 & 4: inspect the target tile */
    if (map_get_tile(p_map, new_x, new_y, &tile) != 0) {
        return -1; /* map access error */
    }

    switch (tile) {
        case TILE_FLOOR:
            p_player->x = new_x;
            p_player->y = new_y;
            return 0; /* moved successfully */

        case TILE_WALL:
            return 1; /* blocked */

        /*
         * TILE_MONSTER → attack (Phase 3)
         * TILE_CHEST   → interact (Phase 4)
         * Until those phases are implemented, treat as blocked.
         */
        default:
            return 1;
    }
}
