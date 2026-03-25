/**
 * @file player.h
 * @brief Player entity interface for the OneBit roguelike.
 *
 * Defines the player data structure and movement API.
 * All game-logic rules (wall collision, viewport scroll trigger,
 * no-backtrack enforcement) live in player.c; this header is
 * purely the contract.
 *
 * Movement return codes:
 *   0                  — player successfully moved (or scroll occurred at top)
 *   1                  — move was blocked (wall, viewport boundary)
 *   PLAYER_MOVE_ATTACK — player stepped into a monster (attack triggered)
 *   PLAYER_MOVE_CHEST  — player stepped into a chest (open triggered)
 *  -1                  — error (NULL pointer argument)
 */
#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"    /* map_t, MAP_WIDTH, VIEWPORT_H */
#include "input.h"  /* action_t */
#include "item.h"   /* item_t, INVENTORY_MAX */

/* ── Initial player stats (per game spec) ────────────────────────────── */
#define PLAYER_INIT_HP    100
#define PLAYER_INIT_MAXHP 100
#define PLAYER_INIT_ATK    10
#define PLAYER_INIT_DEF     0   /* base defense (increases via armor equip) */

/* ── Leveling constants (per game spec §4) ───────────────────────────── */
#define PLAYER_INIT_LEVEL     1   /* starting level                            */
#define PLAYER_INIT_XP        0   /* starting experience points                */
#define LEVELUP_XP_FACTOR    50   /* XP needed to reach next level = level*50  */
#define LEVELUP_MAXHP_BONUS  10   /* max HP increase per level-up              */
#define LEVELUP_ATK_BONUS     2   /* ATK increase per level-up                 */
#define LEVELUP_DEF_BONUS     1   /* DEF increase per level-up                 */
#define XP_BASE_REWARD       10   /* base XP granted for killing a monster     */

/* ── Equipment slot indices ───────────────────────────────────────────── */
#define EQUIP_SLOT_WEAPON 0  /* ITEM_WEAPON goes here */
#define EQUIP_SLOT_HEAD   1  /* ITEM_HELMET goes here */
#define EQUIP_SLOT_BODY   2  /* ITEM_ARMOR  goes here */
#define EQUIP_SLOT_COUNT  3  /* total number of equipment slots */

/* Extended movement return codes (values > 1) */
#define PLAYER_MOVE_ATTACK 2  /* player bumped into a monster — attack triggered */
#define PLAYER_MOVE_CHEST  3  /* player bumped into a chest  — open triggered    */
#define PLAYER_MOVE_COIN   4  /* player stepped onto a coin  — collect triggered */

/* 5-buffer scroll threshold: scroll fires when player.y < SCROLL_BUFFER */
#define SCROLL_BUFFER 5

/* Starting position: horizontally centred, second row from bottom */
#define PLAYER_INIT_X  4
#define PLAYER_INIT_Y  (VIEWPORT_H - 2)

/**
 * @brief All data owned by the player entity.
 */
typedef struct {
    int x;       /* column in viewport [0, MAP_WIDTH)  */
    int y;       /* row    in viewport [0, VIEWPORT_H) */
    int hp;      /* current hit points                 */
    int max_hp;  /* maximum hit points                 */
    int atk;     /* attack power                       */
    int def;     /* defense power (reduces incoming damage) */
    int coins;   /* total coins collected this run     */
    int level;   /* current player level (starts at 1) */
    int xp;      /* experience points toward next level */
    int inventory_count;                  /* items currently in bag [0, INVENTORY_MAX] */
    item_t inventory[INVENTORY_MAX];      /* item bag — indices 0..inventory_count-1   */
    int equipment[EQUIP_SLOT_COUNT];      /* inventory indices of equipped items; -1 = empty */
} player_t;

/* ── Player API ───────────────────────────────────────────────────────── */

/**
 * @brief Add experience points and trigger level-up(s) if the threshold is met.
 *
 * Level-up threshold: level * LEVELUP_XP_FACTOR.
 * On each level-up: level++, max_hp += LEVELUP_MAXHP_BONUS, hp restored to
 * max_hp, atk += LEVELUP_ATK_BONUS, def += LEVELUP_DEF_BONUS.
 * Leftover XP carries over and may trigger additional level-ups.
 *
 * @param p_player  Player receiving XP; must not be NULL.
 * @param amount    XP to add (must be >= 0).
 * @return 0 on success, -1 if p_player is NULL.
 */
int player_gain_xp(player_t *p_player, int amount);

/**
 * @brief Initialise a player with default stats and starting position.
 *
 * @param p_player  Output player to initialise; must not be NULL.
 * @return 0 on success, -1 if p_player is NULL.
 */
int player_init(player_t *p_player);

/**
 * @brief Attempt to move the player one cell in the given direction.
 *
 * Rules applied in order:
 *  1. Moving up from y=0  → map_scroll(p_map), player stays at y=0 (return 0).
 *  2. Moving down from y=VIEWPORT_H-1 → blocked, no backtracking (return 1).
 *  3. Target tile = TILE_WALL → blocked (return 1).
 *  4. Target tile = TILE_FLOOR → player moves to (new_x, new_y) (return 0).
 *
 * @param p_player  Player to move; must not be NULL.
 * @param action    Directional action (ACTION_MOVE_*).
 * @param p_map     Current map; must not be NULL.
 * @return 0 (moved/scrolled), 1 (blocked), -1 (error).
 */
int player_move(player_t *p_player, action_t action, map_t *p_map);

/**
 * @brief Add an item to the player's inventory.
 *
 * Copies *p_item into the next free inventory slot and increments
 * inventory_count. Does nothing if the inventory is already full.
 *
 * @param p_player  Player receiving the item; must not be NULL.
 * @param p_item    Item to add; must not be NULL.
 * @return 0 on success, 1 if inventory is full, -1 if any arg is NULL.
 */
int player_add_item(player_t *p_player, const item_t *p_item);

/**
 * @brief Equip the inventory item at inv_idx into the appropriate slot.
 *
 * Slot is determined by item type:
 *   ITEM_WEAPON → EQUIP_SLOT_WEAPON (adds atk_bonus to player.atk)
 *   ITEM_HELMET → EQUIP_SLOT_HEAD   (adds def_bonus to player.def)
 *   ITEM_ARMOR  → EQUIP_SLOT_BODY   (adds def_bonus to player.def)
 *
 * If the slot is already occupied the old item's bonus is removed first.
 * ITEM_POTION and ITEM_NONE cannot be equipped.
 *
 * @param p_player  Player equipping the item; must not be NULL.
 * @param inv_idx   Index into p_player->inventory [0, inventory_count).
 * @return 0 on success, -1 on error (NULL, invalid index, un-equippable type).
 */
int player_equip(player_t *p_player, int inv_idx);

/**
 * @brief Unequip the item in the given slot, reversing its stat bonus.
 *
 * @param p_player  Player; must not be NULL.
 * @param slot      One of EQUIP_SLOT_WEAPON / HEAD / BODY.
 * @return 0 on success, -1 on error (NULL, invalid slot, slot already empty).
 */
int player_unequip(player_t *p_player, int slot);

#endif /* PLAYER_H */
