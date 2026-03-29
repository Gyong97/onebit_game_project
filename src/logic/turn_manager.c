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
#include <math.h>     /* pow()  — depth-scaled monster stats */
#include "turn_manager.h"
#include "item_db.h"    /* item_db_get(), ITEM_DB_COUNT */
#include "shop.h"       /* shop_open() */

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

/**
 * @brief Compute the target cell for the given action from the player's pos.
 */
static void compute_target(const player_t *p_player, action_t action,
                            int *p_tx, int *p_ty)
{
    *p_tx = p_player->x;
    *p_ty = p_player->y;
    switch (action) {
        case ACTION_MOVE_UP:    (*p_ty)--; break;
        case ACTION_MOVE_DOWN:  (*p_ty)++; break;
        case ACTION_MOVE_LEFT:  (*p_tx)--; break;
        case ACTION_MOVE_RIGHT: (*p_tx)++; break;
        default: break;
    }
}

/**
 * @brief Apply player attack to the monster occupying (tx, ty).
 *
 * Reduces monster.hp by player.atk. If hp drops to 0 or below, the monster
 * is killed (alive = 0), its tile is cleared, and the player receives XP.
 * XP reward: XP_BASE_REWARD + scroll_count/10 (bonus scales with depth).
 */
static void apply_player_attack(game_state_t *p_state, int tx, int ty)
{
    int i;

    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (!p_state->monsters[i].alive) {
            continue;
        }
        if (p_state->monsters[i].x == tx && p_state->monsters[i].y == ty) {
            p_state->monsters[i].hp -= p_state->player.atk;
            if (p_state->monsters[i].hp <= 0) {
                p_state->monsters[i].alive = 0;
                map_set_tile(&p_state->map, tx, ty, TILE_FLOOR);
                /* Grant XP: base reward + depth bonus */
                {
                    int xp = XP_BASE_REWARD
                             + (int)(p_state->map.scroll_count / 10);
                    player_gain_xp(&p_state->player, xp);
                }
            }
            break;
        }
    }
}

int turn_manager_player_act(game_state_t *p_state, action_t action)
{
    long prev_scroll;
    int  move_result;
    int  tx;
    int  ty;

    if (p_state == NULL) {
        return -1;
    }

    /* Pre-compute the target tile so we can act on it after player_move */
    compute_target(&p_state->player, action, &tx, &ty);

    prev_scroll = p_state->map.scroll_count;
    move_result = player_move(&p_state->player, action, &p_state->map);

    if (move_result == -1) {
        return -1; /* internal error */
    }
    if (move_result == 1) {
        return 1;  /* player blocked — no monster turn */
    }

    /* Handle special player actions before monster turn */
    if (move_result == PLAYER_MOVE_ATTACK) {
        apply_player_attack(p_state, tx, ty);
        /* fall through to monster turn */
    } else if (move_result == PLAYER_MOVE_CHEST) {
        if (turn_manager_open_chest(p_state, tx, ty) != 0) {
            return -1;
        }
        /* fall through to monster turn */
    } else if (move_result == PLAYER_MOVE_SHOP) {
        if (turn_manager_enter_shop(p_state, tx, ty) < 0) {
            return -1;
        }
        return TURN_SHOP_OPEN;  /* monster turn is skipped; main loop runs shop UI */
    }

    /* Player moved or acted: handle scroll then run monster turn */
    if (p_state->map.scroll_count > prev_scroll) {
        if (turn_manager_shift_monsters(p_state) != 0) return -1;
        if (turn_manager_spawn_row(p_state) < 0)       return -1;
    }

    if (turn_manager_monsters_act(p_state) != 0) {
        return -1;
    }

    /* Check game-over condition after monsters have acted */
    if (p_state->player.hp <= 0) {
        return TURN_GAME_OVER;
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

int turn_manager_spawn_monster_typed(game_state_t *p_state, int x, int y,
                                     monster_type_t type)
{
    int         slot;
    int         depth;
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

    monster_init_typed(&p_state->monsters[slot], x, y, type);

    /*
     * Depth-based stat scaling (per game spec §4):
     *   depth = scroll_count / 10
     *   hp  = (int)(base_hp  * 1.1^depth)
     *   atk = (int)(base_atk * 1.1^depth)
     * Scale is applied to the type's own base stats already set by
     * monster_init_typed(), so each species scales from its own baseline.
     */
    depth = (int)(p_state->map.scroll_count / 10);
    if (depth > 0) {
        double scale = pow(1.1, depth);
        p_state->monsters[slot].hp  = (int)(p_state->monsters[slot].hp  * scale);
        p_state->monsters[slot].atk = (int)(p_state->monsters[slot].atk * scale);
    }

    map_set_tile(&p_state->map, x, y, TILE_MONSTER);
    return 0;
}

int turn_manager_spawn_monster(game_state_t *p_state, int x, int y)
{
    monster_type_t type;

    if (p_state == NULL) {
        return -1;
    }

    type = (monster_type_t)(rand() % MONSTER_TYPE_COUNT);
    return turn_manager_spawn_monster_typed(p_state, x, y, type);
}

int turn_manager_spawn_row(game_state_t *p_state)
{
    int x;
    int count;
    int shop_col; /* column reserved for shop this row (-1 = none) */

    if (p_state == NULL) {
        return -1;
    }

    /*
     * Shop spawn: every SHOP_SPAWN_INTERVAL scrolls (confirmed placement).
     * Reserve the centre interior column and force TILE_SHOP there.
     * The reserved column is skipped in the main procedural loop so it
     * cannot be overwritten by monster/chest/coin rolls.
     */
    shop_col = -1;
    if (p_state->map.scroll_count > 0
        && p_state->map.scroll_count % SHOP_SPAWN_INTERVAL == 0) {
        shop_col = MAP_WIDTH / 2; /* centre interior column */
        map_set_tile(&p_state->map, shop_col, 0, TILE_SHOP);
    }

    count = 0;
    for (x = 1; x < MAP_WIDTH - 1; x++) {
        if (x == shop_col) {
            continue; /* reserved for shop */
        }
        if (rand() % 100 < MONSTER_SPAWN_PCT) {
            if (turn_manager_spawn_monster(p_state, x, 0) == 0) {
                count++;
                continue; /* cell occupied by monster — skip chest roll */
            }
        }
        /* Chest spawn roll (only on cells that remain TILE_FLOOR) */
        if (rand() % 100 < CHEST_SPAWN_PCT) {
            tile_type_t tile;
            if (map_get_tile(&p_state->map, x, 0, &tile) == 0
                && tile == TILE_FLOOR) {
                map_set_tile(&p_state->map, x, 0, TILE_CHEST);
                continue;
            }
        }
        /* Coin spawn roll (only on cells that remain TILE_FLOOR) */
        if (rand() % 100 < COIN_SPAWN_PCT) {
            tile_type_t tile;
            if (map_get_tile(&p_state->map, x, 0, &tile) == 0
                && tile == TILE_FLOOR) {
                map_set_tile(&p_state->map, x, 0, TILE_COIN);
            }
        }
    }
    return count;
}

int turn_manager_enter_shop(game_state_t *p_state, int x, int y)
{
    if (p_state == NULL) {
        return -1;
    }

    /* Mark shop as visited (one-time; blocks re-entry via TILE_SHOP_OPEN) */
    map_set_tile(&p_state->map, x, y, TILE_SHOP_OPEN);

    /* Open the shop UI state */
    shop_open(&p_state->shop, x, y);

    return TURN_SHOP_OPEN;
}

int turn_manager_open_chest(game_state_t *p_state, int x, int y)
{
    item_t item;
    int    id;

    if (p_state == NULL) {
        return -1;
    }

    /* Pick a random item from the database and give it to the player */
    id = rand() % ITEM_DB_COUNT;
    item_db_get(id, &item);
    player_add_item(&p_state->player, &item);

    /* Mark chest as opened (stays on map in open state, no re-interaction) */
    map_set_tile(&p_state->map, x, y, TILE_CHEST_OPEN);
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
