/**
 * @file map.h
 * @brief Scroll-map interface for the OneBit roguelike.
 *
 * The logical map is infinite in the upward direction. Only a fixed
 * VIEWPORT_H-row window is kept in memory at any time. When the player
 * advances past the top of the viewport, map_scroll() is called: the
 * bottom-most row is discarded, all rows shift down by one, and a new
 * procedurally-generated row appears at the top.
 *
 * Coordinate convention:
 *   rows[0][x]              — topmost visible row (player advances toward this)
 *   rows[VIEWPORT_H-1][x]  — bottommost visible row (falls off on scroll)
 *   x = 0 and x = MAP_WIDTH-1 are always TILE_WALL (border walls).
 */
#ifndef MAP_H
#define MAP_H

#include "renderer.h"  /* tile_type_t, MAP_WIDTH, MAP_HEIGHT */

/* VIEWPORT_H is the same as MAP_HEIGHT — alias for semantic clarity */
#define VIEWPORT_H MAP_HEIGHT

/* Procedural generation: % chance per interior cell for an internal wall */
#define OBSTACLE_SPAWN_PCT 15

/**
 * @brief In-memory representation of the visible map window.
 */
typedef struct {
    tile_type_t rows[VIEWPORT_H][MAP_WIDTH]; /* rows[0] = top of screen */
    long        scroll_count;                 /* cumulative rows scrolled */
} map_t;

/* ── Map API ──────────────────────────────────────────────────────────── */

/**
 * @brief Initialise the map with a starting viewport.
 *
 * All columns at x=0 and x=MAP_WIDTH-1 become TILE_WALL.
 * All interior columns (1 … MAP_WIDTH-2) become TILE_FLOOR.
 * scroll_count is set to 0.
 *
 * @param p_map  Output map to initialise; must not be NULL.
 * @return 0 on success, -1 if p_map is NULL.
 */
int map_init(map_t *p_map);

/**
 * @brief Advance the map by one row (scroll up).
 *
 * Shifts rows[0..VIEWPORT_H-2] down by one (rows[r] ← rows[r-1]),
 * discards the old bottom row, generates a new top row at rows[0],
 * and increments scroll_count.
 *
 * @param p_map  Map to scroll; must not be NULL.
 * @return 0 on success, -1 if p_map is NULL.
 */
int map_scroll(map_t *p_map);

/**
 * @brief Read the tile at (x, y) in the viewport.
 *
 * @param p_map   Source map; must not be NULL.
 * @param x       Column index [0, MAP_WIDTH).
 * @param y       Row index    [0, VIEWPORT_H).
 * @param p_tile  Output: tile at (x, y); must not be NULL.
 * @return 0 on success, -1 on NULL pointer or out-of-bounds coordinates.
 */
int map_get_tile(const map_t *p_map, int x, int y, tile_type_t *p_tile);

/**
 * @brief Write a tile at (x, y) in the viewport.
 *
 * @param p_map  Target map; must not be NULL.
 * @param x      Column index [0, MAP_WIDTH).
 * @param y      Row index    [0, VIEWPORT_H).
 * @param tile   Tile value to write.
 * @return 0 on success, -1 on NULL pointer or out-of-bounds coordinates.
 */
int map_set_tile(map_t *p_map, int x, int y, tile_type_t tile);

#endif /* MAP_H */
