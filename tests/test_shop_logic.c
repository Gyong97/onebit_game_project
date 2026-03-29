/**
 * @file test_shop_logic.c
 * @brief TDD Red: shop module (shop.h/shop.c) unit tests.
 *
 * Tests:
 *  1.  shop_open: active=1, page=BUY, buy_cursor=0, sell_cursor=0
 *  2.  shop_close: active=0
 *  3.  shop_nav_up: cursor at 0 stays at 0 (floor)
 *  4.  shop_nav_up: decrements buy_cursor when page=BUY
 *  5.  shop_nav_down: increments cursor up to count-1
 *  6.  shop_nav_down: stays at count-1 when already at max
 *  7.  shop_switch_page: BUY→SELL, resets sell_cursor to 0
 *  8.  shop_switch_page: SELL→BUY, resets buy_cursor to 0
 *  9.  shop_buy: SHOP_OK, coins deducted, item added
 * 10.  shop_buy: SHOP_INSUFFICIENT when coins < buy_price
 * 11.  shop_buy: SHOP_INVENTORY_FULL when inventory is full
 * 12.  shop_sell: SHOP_OK, item removed from inventory, coins added
 * 13.  shop_sell: SHOP_NOTHING when sell_cursor >= inventory_count
 * 14.  shop_sell: sell_price = buy_price / SHOP_SELL_RATIO (min 1)
 * 15.  shop_sell: equipment indices updated correctly after removal
 * 16.  shop_open/close NULL guard
 * 17.  shop_buy NULL guard
 * 18.  shop_sell NULL guard
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shop.h"
#include "player.h"
#include "item_db.h"

/* ── helpers ──────────────────────────────────────────────────────────── */

static void setup_player(player_t *p)
{
    player_init(p);
}

/* ── 1. shop_open ─────────────────────────────────────────────────────── */
static void test_shop_open_state(void)
{
    shop_state_t shop;
    shop_open(&shop, 3, 5);
    assert(shop.active      == 1);
    assert(shop.page        == SHOP_PAGE_BUY);
    assert(shop.buy_cursor  == 0);
    assert(shop.sell_cursor == 0);
    assert(shop.shop_x      == 3);
    assert(shop.shop_y      == 5);
    printf("[PASS] shop_open: active=1, page=BUY, cursors=0, coords stored\n");
}

/* ── 2. shop_close ────────────────────────────────────────────────────── */
static void test_shop_close(void)
{
    shop_state_t shop;
    shop_open(&shop, 0, 0);
    shop_close(&shop);
    assert(shop.active == 0);
    printf("[PASS] shop_close: active=0\n");
}

/* ── 3. shop_nav_up at floor ──────────────────────────────────────────── */
static void test_shop_nav_up_floor(void)
{
    shop_state_t shop;
    shop_open(&shop, 0, 0);
    shop.buy_cursor = 0;
    shop_nav_up(&shop);
    assert(shop.buy_cursor == 0);
    printf("[PASS] shop_nav_up: cursor at 0 stays at 0\n");
}

/* ── 4. shop_nav_up decrements ────────────────────────────────────────── */
static void test_shop_nav_up_decrement(void)
{
    shop_state_t shop;
    shop_open(&shop, 0, 0);
    shop.buy_cursor = 3;
    shop_nav_up(&shop);
    assert(shop.buy_cursor == 2);
    printf("[PASS] shop_nav_up: decrements buy_cursor from 3 to 2\n");
}

/* ── 5. shop_nav_down increments ──────────────────────────────────────── */
static void test_shop_nav_down_increment(void)
{
    shop_state_t shop;
    shop_open(&shop, 0, 0);
    shop.buy_cursor = 0;
    shop_nav_down(&shop, ITEM_DB_COUNT);
    assert(shop.buy_cursor == 1);
    printf("[PASS] shop_nav_down: increments buy_cursor from 0 to 1\n");
}

/* ── 6. shop_nav_down at max stays ───────────────────────────────────── */
static void test_shop_nav_down_max(void)
{
    shop_state_t shop;
    shop_open(&shop, 0, 0);
    shop.buy_cursor = ITEM_DB_COUNT - 1;
    shop_nav_down(&shop, ITEM_DB_COUNT);
    assert(shop.buy_cursor == ITEM_DB_COUNT - 1);
    printf("[PASS] shop_nav_down: cursor at max stays at max\n");
}

/* ── 7. shop_switch_page BUY→SELL ────────────────────────────────────── */
static void test_shop_switch_buy_to_sell(void)
{
    shop_state_t shop;
    shop_open(&shop, 0, 0);
    shop.buy_cursor  = 3;
    shop.sell_cursor = 1;
    shop_switch_page(&shop);
    assert(shop.page        == SHOP_PAGE_SELL);
    assert(shop.buy_cursor  == 3); /* buy_cursor preserved */
    assert(shop.sell_cursor == 0); /* sell_cursor reset on entering sell page */
    printf("[PASS] shop_switch_page: BUY→SELL, sell_cursor reset\n");
}

/* ── 8. shop_switch_page SELL→BUY ────────────────────────────────────── */
static void test_shop_switch_sell_to_buy(void)
{
    shop_state_t shop;
    shop_open(&shop, 0, 0);
    shop_switch_page(&shop);  /* now SELL */
    shop.sell_cursor = 2;
    shop.buy_cursor  = 4;
    shop_switch_page(&shop);  /* back to BUY */
    assert(shop.page        == SHOP_PAGE_BUY);
    assert(shop.sell_cursor == 2); /* sell_cursor preserved */
    assert(shop.buy_cursor  == 0); /* buy_cursor reset on entering buy page */
    printf("[PASS] shop_switch_page: SELL→BUY, buy_cursor reset\n");
}

/* ── 9. shop_buy success ──────────────────────────────────────────────── */
static void test_shop_buy_success(void)
{
    shop_state_t shop;
    player_t     player;
    item_t       first_item;
    int          expected_price;
    int          coins_before;
    int          inv_before;

    setup_player(&player);
    item_db_get(0, &first_item);
    expected_price = first_item.buy_price;

    player.coins = expected_price + 10;
    coins_before = player.coins;
    inv_before   = player.inventory_count;

    shop_open(&shop, 0, 0);
    shop.buy_cursor = 0;

    assert(shop_buy(&shop, &player) == SHOP_OK);
    assert(player.coins == coins_before - expected_price);
    assert(player.inventory_count == inv_before + 1);
    assert(player.inventory[inv_before].type == first_item.type);
    printf("[PASS] shop_buy: coins deducted, item added (%s)\n",
           first_item.name);
}

/* ── 10. shop_buy insufficient coins ─────────────────────────────────── */
static void test_shop_buy_insufficient(void)
{
    shop_state_t shop;
    player_t     player;
    item_t       first_item;
    int          inv_before;

    setup_player(&player);
    item_db_get(0, &first_item);
    player.coins = first_item.buy_price - 1;
    inv_before   = player.inventory_count;

    shop_open(&shop, 0, 0);
    shop.buy_cursor = 0;

    assert(shop_buy(&shop, &player) == SHOP_INSUFFICIENT);
    assert(player.inventory_count == inv_before);
    printf("[PASS] shop_buy: SHOP_INSUFFICIENT when coins < price\n");
}

/* ── 11. shop_buy inventory full ─────────────────────────────────────── */
static void test_shop_buy_inventory_full(void)
{
    shop_state_t shop;
    player_t     player;
    item_t       dummy;
    int          i;

    setup_player(&player);
    item_db_get(0, &dummy);
    dummy.buy_price = 1;

    /* Fill inventory to max */
    for (i = 0; i < INVENTORY_MAX; i++) {
        player_add_item(&player, &dummy);
    }
    player.coins = 999;

    shop_open(&shop, 0, 0);
    shop.buy_cursor = 0;

    assert(shop_buy(&shop, &player) == SHOP_INVENTORY_FULL);
    printf("[PASS] shop_buy: SHOP_INVENTORY_FULL when bag is full\n");
}

/* ── 12. shop_sell success ───────────────────────────────────────────── */
static void test_shop_sell_success(void)
{
    shop_state_t shop;
    player_t     player;
    item_t       item;
    int          coins_before;
    int          inv_before;
    int          expected_sell;

    setup_player(&player);
    item_db_get(6, &item);  /* Health Pot, buy_price=5 */
    player_add_item(&player, &item);

    coins_before  = player.coins;
    inv_before    = player.inventory_count;
    expected_sell = item.buy_price / SHOP_SELL_RATIO;
    if (expected_sell < 1) expected_sell = 1;

    shop_open(&shop, 0, 0);
    shop_switch_page(&shop);  /* SELL page */
    shop.sell_cursor = 0;

    assert(shop_sell(&shop, &player) == SHOP_OK);
    assert(player.inventory_count == inv_before - 1);
    assert(player.coins == coins_before + expected_sell);
    printf("[PASS] shop_sell: item removed, coins += %d\n", expected_sell);
}

/* ── 13. shop_sell cursor out of range ───────────────────────────────── */
static void test_shop_sell_nothing(void)
{
    shop_state_t shop;
    player_t     player;

    setup_player(&player);
    /* empty inventory */

    shop_open(&shop, 0, 0);
    shop_switch_page(&shop);
    shop.sell_cursor = 0;

    assert(shop_sell(&shop, &player) == SHOP_NOTHING);
    printf("[PASS] shop_sell: SHOP_NOTHING when inventory empty\n");
}

/* ── 14. sell price = buy_price / SHOP_SELL_RATIO (min 1) ───────────── */
static void test_shop_sell_price_calculation(void)
{
    shop_state_t shop;
    player_t     player;
    item_t       item;
    int          coins_before;

    setup_player(&player);
    /* Elixir buy_price=9, sell = 9/2 = 4 */
    item_db_get(7, &item);
    player_add_item(&player, &item);
    coins_before = player.coins;

    shop_open(&shop, 0, 0);
    shop_switch_page(&shop);
    shop.sell_cursor = 0;

    shop_sell(&shop, &player);
    assert(player.coins == coins_before + (item.buy_price / SHOP_SELL_RATIO));
    printf("[PASS] shop_sell: sell price = %d / %d = %d\n",
           item.buy_price, SHOP_SELL_RATIO,
           item.buy_price / SHOP_SELL_RATIO);
}

/* ── 15. shop_sell: equipment indices updated after removal ──────────── */
static void test_shop_sell_updates_equipment_indices(void)
{
    shop_state_t shop;
    player_t     player;
    item_t       sword;   /* inv[0] */
    item_t       armor;   /* inv[1] */
    item_t       helmet;  /* inv[2] */

    setup_player(&player);
    item_db_get(0, &sword);    /* Short Sword */
    item_db_get(2, &armor);    /* Chain Mail */
    item_db_get(4, &helmet);   /* Iron Helm */

    player_add_item(&player, &sword);
    player_add_item(&player, &armor);
    player_add_item(&player, &helmet);

    /* Equip all three */
    player_equip(&player, 0);  /* sword  → EQUIP_SLOT_WEAPON, equipment[0]=0 */
    player_equip(&player, 1);  /* armor  → EQUIP_SLOT_BODY,   equipment[2]=1 */
    player_equip(&player, 2);  /* helmet → EQUIP_SLOT_HEAD,   equipment[1]=2 */

    /* Sell the armor at inv[1] */
    shop_open(&shop, 0, 0);
    shop_switch_page(&shop);   /* SELL page */
    shop.sell_cursor = 1;
    shop_sell(&shop, &player);

    /* After sell:
     *   inventory: [sword(0), helmet(1)]
     *   equipment[EQUIP_SLOT_WEAPON] should still be 0 (sword)
     *   equipment[EQUIP_SLOT_HEAD]   should now be 1  (helmet shifted)
     *   equipment[EQUIP_SLOT_BODY]   should be -1 (armor was sold)
     */
    assert(player.inventory_count == 2);
    assert(player.equipment[EQUIP_SLOT_WEAPON] == 0);
    assert(player.equipment[EQUIP_SLOT_HEAD]   == 1);
    assert(player.equipment[EQUIP_SLOT_BODY]   == -1);
    printf("[PASS] shop_sell: equipment indices remapped after removal\n");
}

/* ── 16. NULL guards ─────────────────────────────────────────────────── */
static void test_shop_null_guards(void)
{
    player_t player;
    setup_player(&player);

    assert(shop_open(NULL, 0, 0)  == -1);
    assert(shop_close(NULL)        == -1);
    assert(shop_nav_up(NULL)       == -1);
    assert(shop_nav_down(NULL, 1)  == -1);
    assert(shop_switch_page(NULL)  == -1);
    assert(shop_buy(NULL, &player) == -1);
    assert(shop_sell(NULL, &player)== -1);
    printf("[PASS] shop_*: NULL pointer guards all return -1\n");
}

/* ── 17/18. shop_buy/sell player NULL ────────────────────────────────── */
static void test_shop_buy_sell_null_player(void)
{
    shop_state_t shop;
    shop_open(&shop, 0, 0);

    assert(shop_buy(&shop, NULL)  == -1);
    assert(shop_sell(&shop, NULL) == -1);
    printf("[PASS] shop_buy/sell: NULL player returns -1\n");
}

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void)
{
    printf("=== test_shop_logic ===\n");
    test_shop_open_state();
    test_shop_close();
    test_shop_nav_up_floor();
    test_shop_nav_up_decrement();
    test_shop_nav_down_increment();
    test_shop_nav_down_max();
    test_shop_switch_buy_to_sell();
    test_shop_switch_sell_to_buy();
    test_shop_buy_success();
    test_shop_buy_insufficient();
    test_shop_buy_inventory_full();
    test_shop_sell_success();
    test_shop_sell_nothing();
    test_shop_sell_price_calculation();
    test_shop_sell_updates_equipment_indices();
    test_shop_null_guards();
    test_shop_buy_sell_null_player();
    printf("=== All shop_logic tests passed. ===\n");
    return 0;
}
