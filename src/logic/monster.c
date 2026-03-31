/**
 * @file monster.c
 * @brief Monster entity implementation for the OneBit roguelike.
 *
 * AI rules (monster_step):
 *  CHASING (player within perception_range):
 *    - BFS from monster to player, follow shortest connected path 1 step.
 *    - Flying monsters treat TILE_WALL as passable in BFS and movement.
 *    - No path found → stay put.
 *  IDLE (player outside perception_range):
 *    - Pick a random adjacent passable tile and step to it.
 *    - Flying monsters also count TILE_WALL as passable in idle movement.
 *    - Player tile is never selected during idle (no opportunistic attack).
 *
 * tile_under tracks the tile originally beneath the monster so it can be
 * restored correctly when the monster moves (important for bats on walls).
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#include <stddef.h>   /* NULL   */
#include <stdlib.h>   /* rand() */
#include <string.h>   /* memset */
#include "monster.h"
#include "monster_db.h"

/* ── BFS constants ────────────────────────────────────────────────────── */

/* Max cells the BFS queue can hold (every map cell at most once). */
#define BFS_CELLS (VIEWPORT_H * MAP_WIDTH)

/* Direction order: right, down, left, up.
 * Right-first provides consistent tie-breaking. */
static const int DX[4] = { 1,  0, -1,  0};
static const int DY[4] = { 0,  1,  0, -1};

/* ── Internal helpers ─────────────────────────────────────────────────── */

/**
 * @brief Return 1 if a tile can be traversed (not entered as final cell)
 *        during BFS path-finding.
 *
 * TILE_PLAYER is the target and is handled separately; it is NOT passable
 * for traversal.  TILE_WALL is passable only for flying monsters.
 */
static int bfs_traversable(tile_type_t t, int can_fly)
{
    switch (t) {
        case TILE_FLOOR:
        case TILE_COIN:
            return 1;
        case TILE_WALL:
            return can_fly;
        default:
            return 0; /* TILE_MONSTER, TILE_CHEST, TILE_SHOP, … */
    }
}

/**
 * @brief BFS shortest-path from (from_x, from_y) to (to_x, to_y).
 *
 * Fills (*p_nx, *p_ny) with the first step the monster should take.
 * When the player is adjacent the first step IS the player cell (caller
 * must handle the attack via try_step).
 *
 * @param p_map    Read-only map for tile queries.
 * @param can_fly  1 if TILE_WALL counts as traversable.
 * @return 0 on success (path found), -1 if no path exists.
 */
static int bfs_next_step(const map_t *p_map,
                          int from_x, int from_y,
                          int to_x,   int to_y,
                          int can_fly,
                          int *p_nx,  int *p_ny)
{
    int visited [VIEWPORT_H][MAP_WIDTH];
    int parent_x[VIEWPORT_H][MAP_WIDTH];
    int parent_y[VIEWPORT_H][MAP_WIDTH];
    int queue_x[BFS_CELLS];
    int queue_y[BFS_CELLS];
    int head = 0, tail = 0;
    int d, x, y, nx, ny;
    tile_type_t tile;

    memset(visited,  0,  sizeof(visited));
    memset(parent_x, -1, sizeof(parent_x));
    memset(parent_y, -1, sizeof(parent_y));

    visited[from_y][from_x] = 1;
    queue_x[tail] = from_x;
    queue_y[tail] = from_y;
    tail++;

    while (head < tail) {
        x = queue_x[head];
        y = queue_y[head];
        head++;

        for (d = 0; d < 4; d++) {
            nx = x + DX[d];
            ny = y + DY[d];

            if (nx <= 0 || nx >= MAP_WIDTH - 1) continue;
            if (ny < 0  || ny >= VIEWPORT_H)    continue;
            if (visited[ny][nx])                 continue;

            /* Target reached: record parent and backtrack */
            if (nx == to_x && ny == to_y) {
                parent_x[ny][nx] = x;
                parent_y[ny][nx] = y;

                /* Walk parent chain until direct child of (from_x, from_y) */
                {
                    int cx = nx, cy = ny;
                    while (parent_x[cy][cx] != from_x
                           || parent_y[cy][cx] != from_y) {
                        int ppx = parent_x[cy][cx];
                        int ppy = parent_y[cy][cx];
                        cx = ppx;
                        cy = ppy;
                    }
                    *p_nx = cx;
                    *p_ny = cy;
                }
                return 0;
            }

            /* Non-target: enqueue if traversable */
            if (map_get_tile(p_map, nx, ny, &tile) != 0) continue;
            if (!bfs_traversable(tile, can_fly)) {
                visited[ny][nx] = 1; /* mark to avoid redundant checks */
                continue;
            }

            visited[ny][nx]  = 1;
            parent_x[ny][nx] = x;
            parent_y[ny][nx] = y;
            queue_x[tail]    = nx;
            queue_y[tail]    = ny;
            tail++;
        }
    }

    return -1; /* no path */
}

/**
 * @brief Attempt one physical move step to (try_x, try_y).
 *
 * Handles:
 *  - TILE_FLOOR / TILE_COIN : normal move; tile_under set to TILE_FLOOR.
 *  - TILE_WALL (flying only): move onto wall; tile_under set to TILE_WALL.
 *  - TILE_PLAYER             : attack (monster stays, player takes damage).
 *  - Everything else         : blocked, *p_moved = 0.
 *
 * On move, restores tile_under at the old position (not necessarily TILE_FLOOR
 * — could be TILE_WALL for a bat that was standing on a wall).
 *
 * @param can_fly  1 if TILE_WALL is a valid landing tile.
 * @param p_moved  Output: 1 if the monster moved or attacked, 0 if blocked.
 * @return 0 on success, -1 on map access error.
 */
static int try_step(monster_t *p_monster, player_t *p_player,
                    map_t *p_map, int can_fly,
                    int try_x, int try_y, int *p_moved)
{
    tile_type_t tile;

    *p_moved = 0;

    if (map_get_tile(p_map, try_x, try_y, &tile) != 0) {
        return -1;
    }

    switch (tile) {
        case TILE_FLOOR:
        case TILE_COIN:
            /* Restore old tile; record new tile_under as TILE_FLOOR */
            map_set_tile(p_map, p_monster->x, p_monster->y,
                         p_monster->tile_under);
            p_monster->x          = try_x;
            p_monster->y          = try_y;
            p_monster->tile_under = TILE_FLOOR;
            map_set_tile(p_map, try_x, try_y, TILE_MONSTER);
            *p_moved = 1;
            return 0;

        case TILE_WALL:
            if (!can_fly) return 0; /* ground monster: blocked */
            /* Flying: land on wall tile; remember it is a wall underneath */
            map_set_tile(p_map, p_monster->x, p_monster->y,
                         p_monster->tile_under);
            p_monster->x          = try_x;
            p_monster->y          = try_y;
            p_monster->tile_under = TILE_WALL;
            map_set_tile(p_map, try_x, try_y, TILE_MONSTER);
            *p_moved = 1;
            return 0;

        case TILE_PLAYER: {
            /* Attack: damage = atk − def, minimum 0 */
            int damage = p_monster->atk - p_player->def;
            if (damage < 0) damage = 0;
            p_player->hp -= damage;
            *p_moved = 1;
            return 0;
        }

        default:
            return 0; /* TILE_MONSTER, TILE_CHEST, TILE_SHOP, … — blocked */
    }
}

/**
 * @brief IDLE behaviour: move one step to a random adjacent passable tile.
 *
 * Ground: passable = TILE_FLOOR, TILE_COIN.
 * Flying: passable = TILE_FLOOR, TILE_COIN, TILE_WALL.
 * TILE_PLAYER is excluded (idle monsters do not initiate attacks).
 */
static void random_walk(monster_t *p_monster, player_t *p_player,
                         map_t *p_map)
{
    int choices_x[4];
    int choices_y[4];
    int count = 0;
    int d, nx, ny, chosen, moved;
    tile_type_t tile;

    for (d = 0; d < 4; d++) {
        nx = p_monster->x + DX[d];
        ny = p_monster->y + DY[d];

        if (nx <= 0 || nx >= MAP_WIDTH - 1) continue;
        if (ny < 0  || ny >= VIEWPORT_H)    continue;
        if (map_get_tile(p_map, nx, ny, &tile) != 0) continue;
        if (tile == TILE_PLAYER) continue; /* idle: no attack */

        if (tile == TILE_FLOOR || tile == TILE_COIN
            || (p_monster->can_fly && tile == TILE_WALL)) {
            choices_x[count] = nx;
            choices_y[count] = ny;
            count++;
        }
    }

    if (count == 0) return;

    chosen = rand() % count;
    try_step(p_monster, p_player, p_map, p_monster->can_fly,
             choices_x[chosen], choices_y[chosen], &moved);
}

/* ── Public API ───────────────────────────────────────────────────────── */

int monster_init_typed(monster_t *p_monster, int x, int y,
                       monster_type_t type)
{
    monster_def_t def;

    if (p_monster == NULL) return -1;

    if (monster_db_get(type, &def) != 0) {
        monster_db_get(MONSTER_TYPE_GOBLIN, &def);
        type = MONSTER_TYPE_GOBLIN;
    }

    p_monster->x                = x;
    p_monster->y                = y;
    p_monster->hp               = def.base_hp;
    p_monster->max_hp           = def.base_hp;
    p_monster->atk              = def.base_atk;
    p_monster->alive            = 1;
    p_monster->type             = type;
    p_monster->state            = MONSTER_STATE_IDLE;
    p_monster->can_fly          = def.can_fly;
    p_monster->perception_range = def.perception_range;
    p_monster->tile_under       = TILE_FLOOR;
    return 0;
}

int monster_init(monster_t *p_monster, int x, int y)
{
    return monster_init_typed(p_monster, x, y, MONSTER_TYPE_GOBLIN);
}

int monster_step(monster_t *p_monster, player_t *p_player, map_t *p_map)
{
    int dx, dy, dist;
    int nx, ny, moved;

    if (p_monster == NULL || p_player == NULL || p_map == NULL) return -1;
    if (!p_monster->alive) return 0;

    dx   = p_player->x - p_monster->x;
    dy   = p_player->y - p_monster->y;
    dist = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);

    if (dist == 0) return 0; /* coincident — shouldn't happen */

    if (dist <= p_monster->perception_range) {
        /* ── CHASING: BFS shortest path ─────────────────────────────── */
        p_monster->state = MONSTER_STATE_CHASING;

        if (bfs_next_step(p_map,
                          p_monster->x, p_monster->y,
                          p_player->x,  p_player->y,
                          p_monster->can_fly,
                          &nx, &ny) == 0) {
            if (try_step(p_monster, p_player, p_map,
                         p_monster->can_fly, nx, ny, &moved) != 0) {
                return -1;
            }
        }
        /* No path found → stay put */

    } else {
        /* ── IDLE: random walk ───────────────────────────────────────── */
        p_monster->state = MONSTER_STATE_IDLE;
        random_walk(p_monster, p_player, p_map);
    }

    return 0;
}
