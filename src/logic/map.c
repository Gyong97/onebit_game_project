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
#include "map.h"

/* ── Internal helpers ─────────────────────────────────────────────────── */

/**
 * @brief Fill one row with border walls and interior floor tiles.
 *
 * x=0 and x=MAP_WIDTH-1 are set to TILE_WALL.
 * x=1 … MAP_WIDTH-2 are set to TILE_FLOOR.
 * Future phases may add random monsters/chests here.
 */
static void map_generate_row(tile_type_t row[MAP_WIDTH])
{
    int x;
    row[0]             = TILE_WALL;
    row[MAP_WIDTH - 1] = TILE_WALL;
    for (x = 1; x < MAP_WIDTH - 1; x++) {
        row[x] = TILE_FLOOR;
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
        map_generate_row(p_map->rows[r]);
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
    map_generate_row(p_map->rows[0]);
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
