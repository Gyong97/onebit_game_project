/**
 * @file shop.h
 * @brief Shop state and interaction interface for the OneBit roguelike.
 *
 * The shop module manages the in-shop UI state: open/close, page navigation
 * (buy/sell), cursor movement, and the buy/sell transactions.
 *
 * Adding a new shop behaviour:
 *   - Open: shop_open() sets active=1, resets cursors.
 *   - Close: shop_close() sets active=0 (tile was already set to TILE_SHOP_OPEN
 *     by turn_manager_enter_shop when the shop was first entered).
 *   - Buy:  shop_buy() looks up cursor item from item_db, deducts buy_price.
 *   - Sell: shop_sell() calls player_remove_item() and awards coins.
 *
 * shop_buy/sell return codes:
 *   SHOP_OK              — transaction succeeded
 *   SHOP_INSUFFICIENT    — not enough coins (buy)
 *   SHOP_INVENTORY_FULL  — inventory is full (buy)
 *   SHOP_NOTHING         — no item at cursor (sell with empty inv or oob cursor)
 *  -1                    — NULL argument error
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#ifndef SHOP_H
#define SHOP_H

#include "player.h"   /* player_t, player_add_item, player_remove_item */
#include "item_db.h"  /* ITEM_DB_COUNT, item_db_get */

/* ── Shop page identifiers ────────────────────────────────────────────── */
#define SHOP_PAGE_BUY   0   /* buy page: browse item_db */
#define SHOP_PAGE_SELL  1   /* sell page: browse player inventory */

/* ── Sell price ratio ─────────────────────────────────────────────────── */
#define SHOP_SELL_RATIO 2   /* sell_price = item.buy_price / SHOP_SELL_RATIO (min 1) */

/* ── Transaction return codes ─────────────────────────────────────────── */
#define SHOP_OK              0   /* transaction succeeded */
#define SHOP_INSUFFICIENT    1   /* not enough coins */
#define SHOP_INVENTORY_FULL  2   /* inventory is full (buy only) */
#define SHOP_NOTHING         3   /* nothing at cursor to act on */

/**
 * @brief In-shop UI state.
 *
 * Two independent cursors are maintained so the player's position on each
 * page is preserved when switching pages with left/right.
 */
typedef struct {
    int active;       /* 1 = shop UI is open, 0 = closed */
    int page;         /* SHOP_PAGE_BUY or SHOP_PAGE_SELL */
    int buy_cursor;   /* selected item index in item_db [0, ITEM_DB_COUNT) */
    int sell_cursor;  /* selected item index in inventory [0, inventory_count) */
    int shop_x;       /* tile column of the shop (for reference) */
    int shop_y;       /* tile row    of the shop (for reference) */
} shop_state_t;

/* ── Shop API ─────────────────────────────────────────────────────────── */

/**
 * @brief Open the shop UI at the given tile coordinates.
 *
 * Sets active=1, page=BUY, buy_cursor=0, sell_cursor=0.
 *
 * @param p_shop  Shop state to initialise; must not be NULL.
 * @param x       Tile column of the shop.
 * @param y       Tile row    of the shop.
 * @return 0 on success, -1 if p_shop is NULL.
 */
int shop_open(shop_state_t *p_shop, int x, int y);

/**
 * @brief Close the shop UI.
 *
 * Sets active=0.  The tile was already set to TILE_SHOP_OPEN by
 * turn_manager_enter_shop() when the shop was first entered.
 *
 * @param p_shop  Shop state; must not be NULL.
 * @return 0 on success, -1 if p_shop is NULL.
 */
int shop_close(shop_state_t *p_shop);

/**
 * @brief Move the cursor up by one (minimum 0).
 *
 * Operates on buy_cursor when page=BUY, sell_cursor when page=SELL.
 *
 * @param p_shop  Shop state; must not be NULL.
 * @return 0 on success, -1 if p_shop is NULL.
 */
int shop_nav_up(shop_state_t *p_shop);

/**
 * @brief Move the cursor down by one (maximum count-1).
 *
 * Operates on buy_cursor when page=BUY, sell_cursor when page=SELL.
 *
 * @param p_shop  Shop state; must not be NULL.
 * @param count   Upper bound: ITEM_DB_COUNT for buy, inventory_count for sell.
 * @return 0 on success, -1 if p_shop is NULL.
 */
int shop_nav_down(shop_state_t *p_shop, int count);

/**
 * @brief Toggle between buy and sell pages.
 *
 * Resets the cursor on the page being entered to 0; preserves the other.
 *
 * @param p_shop  Shop state; must not be NULL.
 * @return 0 on success, -1 if p_shop is NULL.
 */
int shop_switch_page(shop_state_t *p_shop);

/**
 * @brief Buy the item currently selected by buy_cursor.
 *
 * Looks up item at buy_cursor from item_db, deducts buy_price from
 * player.coins, and adds the item to inventory.
 *
 * @param p_shop    Shop state; must not be NULL.
 * @param p_player  Player making the purchase; must not be NULL.
 * @return SHOP_OK, SHOP_INSUFFICIENT, SHOP_INVENTORY_FULL, or -1 on error.
 */
int shop_buy(shop_state_t *p_shop, player_t *p_player);

/**
 * @brief Sell the item currently selected by sell_cursor.
 *
 * Awards player.coins += item.buy_price / SHOP_SELL_RATIO (minimum 1).
 * Calls player_remove_item() which handles unequipping and index remapping.
 *
 * @param p_shop    Shop state; must not be NULL.
 * @param p_player  Player making the sale; must not be NULL.
 * @return SHOP_OK, SHOP_NOTHING, or -1 on error.
 */
int shop_sell(shop_state_t *p_shop, player_t *p_player);

#endif /* SHOP_H */
