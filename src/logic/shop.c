/**
 * @file shop.c
 * @brief Shop interaction logic for the OneBit roguelike.
 *
 * Handles in-shop state (open/close), page and cursor navigation, and
 * the buy/sell transactions.  All inventory mutations go through
 * player_add_item() and player_remove_item() in player.c.
 *
 * No stdio output: pure game-logic module per architecture constraints.
 */
#include <stddef.h>   /* NULL */
#include "shop.h"

/* ── Public API ───────────────────────────────────────────────────────── */

int shop_open(shop_state_t *p_shop, int x, int y)
{
    if (p_shop == NULL) {
        return -1;
    }
    p_shop->active      = 1;
    p_shop->page        = SHOP_PAGE_BUY;
    p_shop->buy_cursor  = 0;
    p_shop->sell_cursor = 0;
    p_shop->shop_x      = x;
    p_shop->shop_y      = y;
    return 0;
}

int shop_close(shop_state_t *p_shop)
{
    if (p_shop == NULL) {
        return -1;
    }
    p_shop->active = 0;
    return 0;
}

int shop_nav_up(shop_state_t *p_shop)
{
    if (p_shop == NULL) {
        return -1;
    }
    if (p_shop->page == SHOP_PAGE_BUY) {
        if (p_shop->buy_cursor > 0) {
            p_shop->buy_cursor--;
        }
    } else {
        if (p_shop->sell_cursor > 0) {
            p_shop->sell_cursor--;
        }
    }
    return 0;
}

int shop_nav_down(shop_state_t *p_shop, int count)
{
    if (p_shop == NULL) {
        return -1;
    }
    if (count <= 0) {
        return 0;
    }
    if (p_shop->page == SHOP_PAGE_BUY) {
        if (p_shop->buy_cursor < count - 1) {
            p_shop->buy_cursor++;
        }
    } else {
        if (p_shop->sell_cursor < count - 1) {
            p_shop->sell_cursor++;
        }
    }
    return 0;
}

int shop_switch_page(shop_state_t *p_shop)
{
    if (p_shop == NULL) {
        return -1;
    }
    if (p_shop->page == SHOP_PAGE_BUY) {
        p_shop->page        = SHOP_PAGE_SELL;
        p_shop->sell_cursor = 0;  /* reset cursor on the newly entered page */
    } else {
        p_shop->page       = SHOP_PAGE_BUY;
        p_shop->buy_cursor = 0;
    }
    return 0;
}

int shop_buy(shop_state_t *p_shop, player_t *p_player)
{
    item_t item;

    if (p_shop == NULL || p_player == NULL) {
        return -1;
    }
    if (p_shop->buy_cursor < 0 || p_shop->buy_cursor >= ITEM_DB_COUNT) {
        return SHOP_NOTHING;
    }
    if (item_db_get(p_shop->buy_cursor, &item) != 0) {
        return SHOP_NOTHING;
    }
    if (p_player->coins < item.buy_price) {
        return SHOP_INSUFFICIENT;
    }
    if (p_player->inventory_count >= INVENTORY_MAX) {
        return SHOP_INVENTORY_FULL;
    }

    p_player->coins -= item.buy_price;
    player_add_item(p_player, &item);
    return SHOP_OK;
}

int shop_sell(shop_state_t *p_shop, player_t *p_player)
{
    int sell_price;

    if (p_shop == NULL || p_player == NULL) {
        return -1;
    }
    if (p_shop->sell_cursor < 0
        || p_shop->sell_cursor >= p_player->inventory_count) {
        return SHOP_NOTHING;
    }

    sell_price = p_player->inventory[p_shop->sell_cursor].buy_price
                 / SHOP_SELL_RATIO;
    if (sell_price < 1) {
        sell_price = 1;  /* minimum 1 coin */
    }

    p_player->coins += sell_price;
    player_remove_item(p_player, p_shop->sell_cursor);
    return SHOP_OK;
}
