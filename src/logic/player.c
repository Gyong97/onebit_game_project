/**
 * @file player.c
 * @brief Player entity implementation for the OneBit roguelike.
 *
 * Handles initialisation and movement including:
 *   - WASD directional movement within the viewport
 *   - Map tile synchronisation: TILE_PLAYER is kept in sync with player pos
 *   - Wall and entity tile collision (TILE_WALL → blocked)
 *   - TILE_MONSTER collision → attack attempt (Phase 4 handles damage);
 *     counted as a player action so monster turn proceeds
 *   - No-backtrack enforcement at the viewport bottom (blocked → return 1)
 *   - Scroll trigger when moving up from y=0 (map_scroll + tile sync)
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
    int old_x;
    int old_y;
    int new_x;
    int new_y;
    tile_type_t tile;

    if (p_player == NULL || p_map == NULL) {
        return -1;
    }

    old_x = p_player->x;
    old_y = p_player->y;
    new_x = old_x;
    new_y = old_y;

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
     *   - Clear player tile at current position before scroll (the row shift
     *     would otherwise carry TILE_PLAYER down to rows[1]).
     *   - After scroll a fresh row[0] has been generated; re-place player.
     *   - Player x/y are unchanged (y stays 0).
     */
    if (new_y < 0) {
        map_set_tile(p_map, old_x, old_y, TILE_FLOOR);
        if (map_scroll(p_map) != 0) {
            return -1;
        }
        map_set_tile(p_map, p_player->x, 0, TILE_PLAYER);
        return 0;
    }

    /*
     * Rule 2: moving down past the bottom of the viewport is forbidden.
     * Those rows have already scrolled off and no longer exist.
     */
    if (new_y >= VIEWPORT_H) {
        return 1; /* blocked */
    }

    /* Horizontal out-of-bounds guard (border walls cover this in practice) */
    if (new_x < 0 || new_x >= MAP_WIDTH) {
        return 1; /* blocked */
    }

    /* Inspect the target tile */
    if (map_get_tile(p_map, new_x, new_y, &tile) != 0) {
        return -1; /* map access error */
    }

    switch (tile) {
        case TILE_FLOOR:
            /* Standard move: sync map tiles and update player position */
            map_set_tile(p_map, old_x, old_y, TILE_FLOOR);
            p_player->x = new_x;
            p_player->y = new_y;
            map_set_tile(p_map, new_x, new_y, TILE_PLAYER);
            return 0;

        case TILE_WALL:
            return 1; /* blocked */

        case TILE_MONSTER:
            /*
             * Attack: player stays in place, turn_manager handles HP damage.
             * Return PLAYER_MOVE_ATTACK so the caller knows to apply damage.
             */
            return PLAYER_MOVE_ATTACK;

        case TILE_CHEST:
            /*
             * Chest interaction: player stays in place, turn_manager opens it.
             * Return PLAYER_MOVE_CHEST so the caller knows to call open_chest.
             */
            return PLAYER_MOVE_CHEST;

        default:
            return 1; /* blocked */
    }
}
