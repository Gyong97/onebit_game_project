/**
 * @file map.c
 * @brief Scroll-map implementation for the OneBit roguelike.
 *
 * Manages a fixed 10-row viewport over an infinite upward map.
 * Scrolling shifts all rows down by one and procedurally generates
 * a new top row — the old bottom row is simply discarded (overwritten).
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#include <string.h>  /* memcpy */
#include <stdlib.h>  /* rand()  */
#include "map.h"

/* ── Internal helpers ─────────────────────────────────────────────────── */

/**
 * @brief Fill one row with border walls and plain floor (used by map_init).
 *
 * x=0 and x=MAP_WIDTH-1 → TILE_WALL. Interior → TILE_FLOOR (no random).
 * The starting map is kept obstacle-free so the player always has a safe
 * initial play area.
 */
static void map_generate_base_row(tile_type_t row[MAP_WIDTH])
{
    int x;
    row[0]             = TILE_WALL;
    row[MAP_WIDTH - 1] = TILE_WALL;
    for (x = 1; x < MAP_WIDTH - 1; x++) {
        row[x] = TILE_FLOOR;
    }
}

/**
 * @brief Fill one row with border walls and procedurally generated interior.
 *
 * Used by map_scroll() for newly generated rows only.
 * Each interior cell has an OBSTACLE_SPAWN_PCT % chance of being TILE_WALL.
 * Entity and collectible spawning (monsters, chests, coins) is handled
 * separately by turn_manager_spawn_row() after map_scroll() returns.
 */
static void map_generate_procedural_row(tile_type_t row[MAP_WIDTH])
{
    int x;
    row[0]             = TILE_WALL;
    row[MAP_WIDTH - 1] = TILE_WALL;
    for (x = 1; x < MAP_WIDTH - 1; x++) {
        row[x] = (rand() % 100 < OBSTACLE_SPAWN_PCT) ? TILE_WALL : TILE_FLOOR;
    }
}

/* ── Public API ───────────────────────────────────────────────────────── */

int map_init(map_t *p_map)
{
    int r;

    if (p_map == NULL) {
        return -1;
    }

    for (r = 0; r < VIEWPORT_H; r++) {
        map_generate_base_row(p_map->rows[r]);
    }
    p_map->scroll_count = 0;
    return 0;
}

int map_scroll(map_t *p_map)
{
    int r;

    if (p_map == NULL) {
        return -1;
    }

    /*
     * Shift existing rows down by one:
     *   old rows[VIEWPORT_H-2] → rows[VIEWPORT_H-1]  (old bottom discarded)
     *   old rows[0]            → rows[1]
     * Then generate a fresh top row at rows[0].
     */
    for (r = VIEWPORT_H - 1; r > 0; r--) {
        memcpy(p_map->rows[r], p_map->rows[r - 1],
               sizeof(tile_type_t) * MAP_WIDTH);
    }
    map_generate_procedural_row(p_map->rows[0]);
    p_map->scroll_count++;
    return 0;
}

int map_get_tile(const map_t *p_map, int x, int y, tile_type_t *p_tile)
{
    if (p_map == NULL || p_tile == NULL) {
        return -1;
    }
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= VIEWPORT_H) {
        return -1;
    }
    *p_tile = p_map->rows[y][x];
    return 0;
}

int map_set_tile(map_t *p_map, int x, int y, tile_type_t tile)
{
    if (p_map == NULL) {
        return -1;
    }
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= VIEWPORT_H) {
        return -1;
    }
    p_map->rows[y][x] = tile;
    return 0;
}
