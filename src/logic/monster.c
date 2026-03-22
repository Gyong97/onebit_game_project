/**
 * @file monster.c
 * @brief Monster entity implementation for the OneBit roguelike.
 *
 * Tracking AI (monster_step) rules:
 *   1. Primary axis: the axis with the larger |distance| to player.
 *      Horizontal (x) wins when |dx| == |dy|.
 *   2. If the primary step is blocked (TILE_WALL / TILE_MONSTER / TILE_CHEST),
 *      attempt the secondary axis as a fallback.
 *   3. If both axes are blocked, the monster stays in place.
 *   4. If the target cell is TILE_PLAYER, perform an attack attempt:
 *      monster stays put (Phase 4 applies damage).
 *
 * Map tiles are kept in sync on every successful move.
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#include <stddef.h>   /* NULL  */
#include <stdlib.h>   /* abs() */
#include "monster.h"

/* ── Public API ───────────────────────────────────────────────────────── */

int monster_init(monster_t *p_monster, int x, int y)
{
    if (p_monster == NULL) {
        return -1;
    }
    p_monster->x     = x;
    p_monster->y     = y;
    p_monster->hp    = MONSTER_INIT_HP;
    p_monster->atk   = MONSTER_INIT_ATK;
    p_monster->alive = 1;
    return 0;
}

/**
 * @brief Attempt one move step in the given direction.
 *
 * @param p_monster  Monster performing the step.
 * @param p_map      Map for tile collision and update.
 * @param try_x      Target column.
 * @param try_y      Target row.
 * @param p_moved    Output: set to 1 if the monster moved, 0 otherwise.
 * @return 0 on normal resolution (moved/attacked/blocked),
 *         -1 on map access error.
 */
static int try_step(monster_t *p_monster, player_t *p_player,
                    map_t *p_map,
                    int try_x, int try_y, int *p_moved)
{
    tile_type_t tile;

    *p_moved = 0;

    if (map_get_tile(p_map, try_x, try_y, &tile) != 0) {
        return -1; /* out-of-bounds or map error */
    }

    switch (tile) {
        case TILE_FLOOR:
            /* Move: sync map tiles, update monster position */
            map_set_tile(p_map, p_monster->x, p_monster->y, TILE_FLOOR);
            p_monster->x = try_x;
            p_monster->y = try_y;
            map_set_tile(p_map, try_x, try_y, TILE_MONSTER);
            *p_moved = 1;
            return 0;

        case TILE_PLAYER:
            /* Attack: monster stays in place, deals damage to player */
            p_player->hp -= p_monster->atk;
            *p_moved = 1; /* counts as "acted" */
            return 0;

        default:
            /* TILE_WALL, TILE_MONSTER, TILE_CHEST: blocked */
            return 0;
    }
}

int monster_step(monster_t *p_monster, player_t *p_player,
                 map_t *p_map)
{
    int dx;
    int dy;
    int prim_x;  /* primary step target */
    int prim_y;
    int sec_x;   /* secondary (fallback) step target */
    int sec_y;
    int moved;
    int prefer_horiz; /* 1 if primary axis is horizontal */

    if (p_monster == NULL || p_player == NULL || p_map == NULL) {
        return -1;
    }
    if (!p_monster->alive) {
        return 0;
    }

    dx = p_player->x - p_monster->x;
    dy = p_player->y - p_monster->y;

    if (dx == 0 && dy == 0) {
        return 0; /* player and monster on same tile (shouldn't happen) */
    }

    /*
     * Determine primary axis: horizontal when |dx| >= |dy|.
     * Compute primary and secondary step targets.
     */
    prefer_horiz = (abs(dx) >= abs(dy));

    if (prefer_horiz) {
        /* Primary: horizontal */
        prim_x = p_monster->x + (dx > 0 ? 1 : -1);
        prim_y = p_monster->y;
        /* Secondary: vertical (only if dy != 0) */
        sec_x  = p_monster->x;
        sec_y  = (dy != 0) ? p_monster->y + (dy > 0 ? 1 : -1)
                            : p_monster->y; /* no vertical option */
    } else {
        /* Primary: vertical */
        prim_x = p_monster->x;
        prim_y = p_monster->y + (dy > 0 ? 1 : -1);
        /* Secondary: horizontal (only if dx != 0) */
        sec_x  = (dx != 0) ? p_monster->x + (dx > 0 ? 1 : -1)
                            : p_monster->x; /* no horizontal option */
        sec_y  = p_monster->y;
    }

    /* Try primary axis */
    if (try_step(p_monster, p_player, p_map, prim_x, prim_y, &moved) != 0) {
        return -1;
    }
    if (moved) {
        return 0;
    }

    /* Primary blocked: try secondary axis (only if it differs from current) */
    if (sec_x == p_monster->x && sec_y == p_monster->y) {
        return 0; /* no secondary option (dy==0 or dx==0) */
    }
    if (try_step(p_monster, p_player, p_map, sec_x, sec_y, &moved) != 0) {
        return -1;
    }

    return 0; /* moved or stayed — either way, action complete */
}
