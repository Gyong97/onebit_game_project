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
#include <stddef.h>    /* NULL */
#include "player.h"
#include "playable_db.h"  /* playable_db_get, playable_def_t */

/**
 * @brief Determine the equipment slot for an item type.
 * @return Slot index, or -1 if the item type is not equippable.
 */
static int slot_for_type(item_type_t type)
{
    switch (type) {
        case ITEM_WEAPON: return EQUIP_SLOT_WEAPON;
        case ITEM_HELMET: return EQUIP_SLOT_HEAD;
        case ITEM_ARMOR:  return EQUIP_SLOT_BODY;
        default:          return -1; /* ITEM_NONE, ITEM_POTION */
    }
}

/**
 * @brief Apply the stat bonus of an inventory item (add or subtract).
 */
static void apply_item_bonus(player_t *p_player, int inv_idx, int sign)
{
    const item_t *it = &p_player->inventory[inv_idx];
    p_player->atk += sign * it->atk_bonus;
    p_player->def += sign * it->def_bonus;
}

int player_equip(player_t *p_player, int inv_idx)
{
    int slot;

    if (p_player == NULL) {
        return -1;
    }
    if (inv_idx < 0 || inv_idx >= p_player->inventory_count) {
        return -1; /* invalid inventory index */
    }

    slot = slot_for_type(p_player->inventory[inv_idx].type);
    if (slot == -1) {
        return -1; /* un-equippable item type */
    }

    /* If slot already occupied, remove old bonus first */
    if (p_player->equipment[slot] != -1) {
        apply_item_bonus(p_player, p_player->equipment[slot], -1);
    }

    /* Apply new item's bonus and update slot */
    apply_item_bonus(p_player, inv_idx, +1);
    p_player->equipment[slot] = inv_idx;
    return 0;
}

int player_unequip(player_t *p_player, int slot)
{
    if (p_player == NULL) {
        return -1;
    }
    if (slot < 0 || slot >= EQUIP_SLOT_COUNT) {
        return -1; /* invalid slot */
    }
    if (p_player->equipment[slot] == -1) {
        return -1; /* slot already empty */
    }

    apply_item_bonus(p_player, p_player->equipment[slot], -1);
    p_player->equipment[slot] = -1;
    return 0;
}

int player_add_item(player_t *p_player, const item_t *p_item)
{
    if (p_player == NULL || p_item == NULL) {
        return -1;
    }
    if (p_player->inventory_count >= INVENTORY_MAX) {
        return 1; /* inventory full */
    }
    p_player->inventory[p_player->inventory_count] = *p_item;
    p_player->inventory_count++;
    return 0;
}

/* ── Public API ───────────────────────────────────────────────────────── */

int player_init_typed(player_t *p_player, playable_type_t class_type)
{
    playable_def_t def;
    int            i;

    if (p_player == NULL) {
        return -1;
    }
    if (playable_db_get(class_type, &def) != 0) {
        return -1; /* invalid class */
    }

    p_player->hp              = def.init_hp;
    p_player->max_hp          = def.init_max_hp;
    p_player->atk             = def.init_atk;
    p_player->def             = def.init_def;
    p_player->x               = PLAYER_INIT_X;
    p_player->y               = PLAYER_INIT_Y;
    p_player->coins           = 0;
    p_player->level           = PLAYER_INIT_LEVEL;
    p_player->xp              = PLAYER_INIT_XP;
    p_player->inventory_count = 0;
    for (i = 0; i < EQUIP_SLOT_COUNT; i++) {
        p_player->equipment[i] = -1;
    }
    return 0;
}

int player_init(player_t *p_player)
{
    return player_init_typed(p_player, PLAYABLE_WARRIOR);
}

int player_gain_xp(player_t *p_player, int amount)
{
    if (p_player == NULL) {
        return -1;
    }

    p_player->xp += amount;

    /* Process level-ups: threshold for current level = level * LEVELUP_XP_FACTOR */
    while (p_player->xp >= p_player->level * LEVELUP_XP_FACTOR) {
        p_player->xp     -= p_player->level * LEVELUP_XP_FACTOR;
        p_player->level++;
        p_player->max_hp += LEVELUP_MAXHP_BONUS;
        p_player->hp      = p_player->max_hp; /* full heal */
        p_player->atk    += LEVELUP_ATK_BONUS;
        p_player->def    += LEVELUP_DEF_BONUS;
    }

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
     *   - Clear player tile before scroll (row shift would copy TILE_PLAYER).
     *   - After scroll rows[0] is fresh; re-place player at y=0.
     *   - Player x/y unchanged (y stays 0).
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
     */
    if (new_y >= VIEWPORT_H) {
        return 1; /* blocked */
    }

    /* Horizontal out-of-bounds guard */
    if (new_x < 0 || new_x >= MAP_WIDTH) {
        return 1; /* blocked */
    }

    /* Inspect the target tile */
    if (map_get_tile(p_map, new_x, new_y, &tile) != 0) {
        return -1; /* map access error */
    }

    switch (tile) {
        case TILE_FLOOR:
        case TILE_COIN:
            /*
             * Move to floor or coin tile.
             * For TILE_COIN the tile is overwritten by TILE_PLAYER and the
             * coin counter is incremented.
             */
            if (tile == TILE_COIN) {
                p_player->coins++;
            }
            map_set_tile(p_map, old_x, old_y, TILE_FLOOR);
            p_player->x = new_x;
            p_player->y = new_y;
            map_set_tile(p_map, new_x, new_y, TILE_PLAYER);

            /*
             * 5-Buffer scroll rule (ACTION_MOVE_UP only):
             * If the player has entered the buffer zone (y < SCROLL_BUFFER),
             * fire a scroll immediately.  The scroll shifts all rows down by
             * one, so player.y is incremented to compensate, keeping the
             * player at the buffer boundary in viewport coordinates.
             */
            if (action == ACTION_MOVE_UP && p_player->y < MAP_BUFFER_H + SCROLL_BUFFER) {
                map_set_tile(p_map, p_player->x, p_player->y, TILE_FLOOR);
                if (map_scroll(p_map) != 0) {
                    return -1;
                }
                p_player->y++;
                map_set_tile(p_map, p_player->x, p_player->y, TILE_PLAYER);
            }
            return 0;

        case TILE_WALL:
            return 1; /* blocked */

        case TILE_MONSTER:
            /*
             * Attack: player stays in place, turn_manager handles HP damage.
             */
            return PLAYER_MOVE_ATTACK;

        case TILE_CHEST:
            /*
             * Chest interaction: player stays, turn_manager opens it.
             */
            return PLAYER_MOVE_CHEST;

        case TILE_SHOP:
            /*
             * Shop interaction: player stays, turn_manager handles purchase.
             */
            return PLAYER_MOVE_SHOP;

        case TILE_CHEST_OPEN:
        case TILE_SHOP_OPEN:
            /* Already opened/visited — still a fixed object, so block movement. */
            return 1;

        default:
            return 1; /* blocked */
    }
}
