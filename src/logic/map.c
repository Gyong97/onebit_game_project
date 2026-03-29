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

/* ── Connectivity helpers ─────────────────────────────────────────────── */

/* Total cells in the buffer region */
#define BUF_CELLS (MAP_BUFFER_H * MAP_WIDTH)

/* A tile that is not TILE_WALL counts as passable for connectivity. */
static int tile_passable(tile_type_t t)
{
    return (t != TILE_WALL);
}

/*
 * BFS flood-fill within rows[0..MAP_BUFFER_H-1], seeded from all passable
 * tiles in rows[MAP_BUFFER_H-1] (the buffer-to-visible boundary).
 * Sets visited[y][x]=1 for every reachable passable tile.
 */
static void buffer_bfs(const map_t *p_map,
                       int visited[MAP_BUFFER_H][MAP_WIDTH])
{
    int queue[BUF_CELLS];
    int head = 0, tail = 0;
    int x, y, nx, ny, d;
    static const int dx[4] = { 0,  0, 1, -1};
    static const int dy[4] = {-1,  1, 0,  0};

    memset(visited, 0, MAP_BUFFER_H * MAP_WIDTH * sizeof(int));

    for (x = 1; x < MAP_WIDTH - 1; x++) {
        if (tile_passable(p_map->rows[MAP_BUFFER_H - 1][x])) {
            visited[MAP_BUFFER_H - 1][x] = 1;
            queue[tail++] = (MAP_BUFFER_H - 1) * MAP_WIDTH + x;
        }
    }
    while (head < tail) {
        int pos = queue[head++];
        y = pos / MAP_WIDTH;
        x = pos % MAP_WIDTH;
        for (d = 0; d < 4; d++) {
            nx = x + dx[d]; ny = y + dy[d];
            if (nx <= 0 || nx >= MAP_WIDTH - 1) continue;
            if (ny < 0 || ny >= MAP_BUFFER_H)   continue;
            if (visited[ny][nx]) continue;
            if (!tile_passable(p_map->rows[ny][nx])) continue;
            visited[ny][nx] = 1;
            queue[tail++] = ny * MAP_WIDTH + nx;
        }
    }
}

/*
 * From unvisited passable tile (start_x, start_y), BFS through ALL tiles
 * (crossing walls) until a visited tile is found, then backtrack via parent
 * pointers and convert wall tiles on the path to TILE_FLOOR.
 * Newly passable tiles are marked visited[] so the caller's visited map stays
 * consistent.
 */
static void carve_to_reachable(map_t *p_map,
                                int visited[MAP_BUFFER_H][MAP_WIDTH],
                                int start_x, int start_y)
{
    int queue[BUF_CELLS];
    int parent[MAP_BUFFER_H][MAP_WIDTH]; /* packed pos, -1 = none */
    int seen[MAP_BUFFER_H][MAP_WIDTH];
    int head = 0, tail = 0;
    int x, y, nx, ny, d;
    static const int dx[4] = { 0,  0, 1, -1};
    static const int dy[4] = {-1,  1, 0,  0};

    memset(parent, -1, sizeof(parent));
    memset(seen,    0, sizeof(seen));

    seen[start_y][start_x] = 1;
    queue[tail++] = start_y * MAP_WIDTH + start_x;

    while (head < tail) {
        int pos = queue[head++];
        y = pos / MAP_WIDTH;
        x = pos % MAP_WIDTH;

        for (d = 0; d < 4; d++) {
            nx = x + dx[d]; ny = y + dy[d];
            if (nx <= 0 || nx >= MAP_WIDTH - 1) continue;
            if (ny < 0 || ny >= MAP_BUFFER_H)   continue;
            if (seen[ny][nx]) continue;
            seen[ny][nx]     = 1;
            parent[ny][nx]   = pos; /* remember where we came from */

            if (visited[ny][nx]) {
                /* Path found — backtrack and carve walls to floor */
                int cx = nx, cy = ny;
                while (parent[cy][cx] >= 0) {
                    int pp = parent[cy][cx];
                    int py = pp / MAP_WIDTH;
                    int px = pp % MAP_WIDTH;
                    if (p_map->rows[cy][cx] == TILE_WALL) {
                        p_map->rows[cy][cx] = TILE_FLOOR;
                    }
                    visited[cy][cx] = 1;
                    cx = px; cy = py;
                }
                visited[start_y][start_x] = 1;
                return;
            }
            queue[tail++] = ny * MAP_WIDTH + nx;
        }
    }
}

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
    int has_floor = 0;

    row[0]             = TILE_WALL;
    row[MAP_WIDTH - 1] = TILE_WALL;
    for (x = 1; x < MAP_WIDTH - 1; x++) {
        row[x] = (rand() % 100 < OBSTACLE_SPAWN_PCT) ? TILE_WALL : TILE_FLOOR;
        if (row[x] == TILE_FLOOR) {
            has_floor = 1;
        }
    }
    /* Guarantee at least one passable interior cell (no isolated row). */
    if (!has_floor) {
        row[MAP_WIDTH / 2] = TILE_FLOOR;
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

    /* Ensure buffer connectivity before the new row enters the visible area */
    map_ensure_connectivity(p_map);
    return 0;
}

void map_ensure_connectivity(map_t *p_map)
{
    int visited[MAP_BUFFER_H][MAP_WIDTH];
    int x, y;
    int found;

    if (p_map == NULL) return;

    /*
     * Repeat until every passable buffer tile is reachable:
     *   1. BFS from rows[MAP_BUFFER_H-1] passable tiles.
     *   2. Find first unreachable passable tile.
     *   3. Carve a wall-breaking path to the nearest reachable tile.
     * Each carve connects at least one new tile; the loop terminates because
     * the total number of walls is finite.
     */
    do {
        found = 0;
        buffer_bfs(p_map, visited);
        for (y = 0; y < MAP_BUFFER_H && !found; y++) {
            for (x = 1; x < MAP_WIDTH - 1 && !found; x++) {
                if (tile_passable(p_map->rows[y][x]) && !visited[y][x]) {
                    carve_to_reachable(p_map, visited, x, y);
                    found = 1;
                }
            }
        }
    } while (found);
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
