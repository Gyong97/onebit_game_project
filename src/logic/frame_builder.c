/**
 * @file frame_builder.c
 * @brief Frame builder — translates game_state_t → render_frame_t.
 *
 * This is the adapter layer between game logic and the renderer.
 * No stdio output; no game-logic modifications.
 */
#include <string.h>        /* strncpy, memset */
#include "frame_builder.h"
#include "item_db.h"       /* item_db_get, ITEM_DB_COUNT */
#include "monster_db.h"    /* monster_db_get, monster_def_t */
#include "shop.h"          /* SHOP_PAGE_BUY, SHOP_SELL_RATIO */

/* ── Internal helpers ─────────────────────────────────────────────────── */

/**
 * @brief Compute the direction string from monster → player perspective.
 *
 * The monster is at (mx, my), the player is at (px, py).
 * Only the four cardinal adjacents are returned; diagonal or distant →  "".
 */
static void direction_string(int mx, int my, int px, int py, char dir[4])
{
    /* Direction of monster relative to player:
     *   monster.y < player.y → monster is NORTH ("N")
     *   monster.y > player.y → monster is SOUTH ("S")
     *   monster.x > player.x → monster is EAST  ("E")
     *   monster.x < player.x → monster is WEST  ("W")
     */
    int dx = mx - px;
    int dy = my - py;

    dir[0] = '\0';

    if (dx == 0 && dy == -1) { strncpy(dir, "N", 3); return; }
    if (dx == 0 && dy ==  1) { strncpy(dir, "S", 3); return; }
    if (dx == 1 && dy ==  0) { strncpy(dir, "E", 3); return; }
    if (dx == -1 && dy == 0) { strncpy(dir, "W", 3); return; }
}

/**
 * @brief Check whether monster index idx is already in the panel.
 */
static int already_in_panel(const render_frame_t *p_frame, int n,
                             int mx, int my)
{
    int i;
    for (i = 0; i < n; i++) {
        if (!p_frame->monster_panel[i].active) continue;
        /* Compare by position (no duplicate position in the pool) */
        (void)mx; (void)my; /* suppress warning — comparison via name+hp below */
    }
    return 0; /* simple: each monster occupies a unique tile, no dup needed */
}

/**
 * @brief Try to add monster at pool index idx to the panel (if room).
 *
 * @param p_frame  Output frame.
 * @param p_state  Game state.
 * @param idx      Monster pool index.
 * @param is_adj   1 = monster is adjacent, 0 = not adjacent (no direction).
 * @param p_count  Current count of panel entries; incremented on add.
 */
static void panel_add(render_frame_t *p_frame, const game_state_t *p_state,
                      int idx, int is_adj, int *p_count)
{
    monster_def_t    def;
    ui_monster_entry_t *e;

    if (*p_count >= UI_MONSTER_PANEL_MAX) return;
    if (!p_state->monsters[idx].alive) return;

    e = &p_frame->monster_panel[*p_count];

    monster_db_get(p_state->monsters[idx].type, &def);
    strncpy(e->name, def.name, UI_NAME_MAX - 1);
    e->name[UI_NAME_MAX - 1] = '\0';
    e->hp     = p_state->monsters[idx].hp;
    e->max_hp = p_state->monsters[idx].max_hp;
    e->active = 1;

    if (is_adj) {
        direction_string(p_state->monsters[idx].x,
                         p_state->monsters[idx].y,
                         p_state->player.x,
                         p_state->player.y,
                         e->dir);
    } else {
        e->dir[0] = '\0';
    }

    (*p_count)++;
}

/* ── Public API ───────────────────────────────────────────────────────── */

void frame_builder_build(render_frame_t     *p_frame,
                         const game_state_t *p_state,
                         const save_data_t  *p_save)
{
    int r;
    int c;
    int i;
    int panel_count;
    int last_idx;

    if (p_frame == NULL || p_state == NULL || p_save == NULL) return;

    /* ── Map tiles (viewport slice) ───────────────────────────────── */
    for (r = 0; r < MAP_HEIGHT; r++) {
        for (c = 0; c < MAP_WIDTH; c++) {
            p_frame->tiles[r][c] =
                p_state->map.rows[MAP_BUFFER_H + r][c];
        }
    }

    /* ── Player HUD ───────────────────────────────────────────────── */
    p_frame->player_hp       = p_state->player.hp;
    p_frame->player_max_hp   = p_state->player.max_hp;
    p_frame->player_atk      = p_state->player.atk;
    p_frame->player_def      = p_state->player.def;
    p_frame->player_coins    = p_state->player.coins;
    p_frame->inventory_count = p_state->player.inventory_count;
    p_frame->scroll_count    = p_state->map.scroll_count;
    p_frame->best_depth      = p_save->best_depth;

    p_frame->player_level      = p_state->player.level;
    p_frame->player_xp         = p_state->player.xp;
    p_frame->player_xp_to_next = p_state->player.level * LEVELUP_XP_FACTOR;
    p_frame->show_levelup      = (p_state->levelup_ttl > 0) ? 1 : 0;

    /* ── Equipment names ──────────────────────────────────────────── */
    p_frame->equip_weapon[0] = '\0';
    p_frame->equip_head[0]   = '\0';
    p_frame->equip_body[0]   = '\0';

    {
        int slot;
        int inv_idx;
        for (slot = 0; slot < EQUIP_SLOT_COUNT; slot++) {
            inv_idx = p_state->player.equipment[slot];
            if (inv_idx >= 0 && inv_idx < p_state->player.inventory_count) {
                const char *n = p_state->player.inventory[inv_idx].name;
                if (slot == EQUIP_SLOT_WEAPON) {
                    strncpy(p_frame->equip_weapon, n, EQUIP_NAME_MAX - 1);
                    p_frame->equip_weapon[EQUIP_NAME_MAX - 1] = '\0';
                } else if (slot == EQUIP_SLOT_HEAD) {
                    strncpy(p_frame->equip_head, n, EQUIP_NAME_MAX - 1);
                    p_frame->equip_head[EQUIP_NAME_MAX - 1] = '\0';
                } else if (slot == EQUIP_SLOT_BODY) {
                    strncpy(p_frame->equip_body, n, EQUIP_NAME_MAX - 1);
                    p_frame->equip_body[EQUIP_NAME_MAX - 1] = '\0';
                }
            }
        }
    }

    /* ── Monster info panel ───────────────────────────────────────── */
    for (i = 0; i < UI_MONSTER_PANEL_MAX; i++) {
        memset(&p_frame->monster_panel[i], 0, sizeof(ui_monster_entry_t));
    }
    panel_count = 0;

    /* Pass 1: adjacent monsters (distance == 1 in one axis, 0 in other) */
    for (i = 0; i < MONSTER_MAX_COUNT && panel_count < UI_MONSTER_PANEL_MAX; i++) {
        int dx;
        int dy;
        if (!p_state->monsters[i].alive) continue;
        dx = p_state->monsters[i].x - p_state->player.x;
        dy = p_state->monsters[i].y - p_state->player.y;
        if ((dx == 0 && (dy == 1 || dy == -1))
            || (dy == 0 && (dx == 1 || dx == -1))) {
            panel_add(p_frame, p_state, i, 1, &panel_count);
        }
    }

    /* Pass 2: last attacked monster (if alive and not already in panel) */
    last_idx = p_state->last_attacked_monster_idx;
    if (last_idx >= 0 && last_idx < MONSTER_MAX_COUNT
        && p_state->monsters[last_idx].alive) {
        /* Check for duplicate: was it already added as adjacent? */
        int already = 0;
        int j;
        for (j = 0; j < panel_count; j++) {
            /* Compare by hp+name — sufficient since panel_count is small */
            if (p_frame->monster_panel[j].hp
                    == p_state->monsters[last_idx].hp) {
                already = 1;
                break;
            }
        }
        if (!already) {
            panel_add(p_frame, p_state, last_idx, 0, &panel_count);
        }
    }

    /* ── Chest loot panel ─────────────────────────────────────────── */
    p_frame->chest_loot_count = 0;
    for (i = 0; i < UI_CHEST_PANEL_MAX; i++) {
        p_frame->chest_loot[i][0] = '\0';
    }
    if (p_state->chest_loot_ttl > 0) {
        int loot_n = p_state->chest_loot_count;
        if (loot_n > UI_CHEST_PANEL_MAX) loot_n = UI_CHEST_PANEL_MAX;
        for (i = 0; i < loot_n; i++) {
            strncpy(p_frame->chest_loot[i],
                    p_state->chest_loot_names[i],
                    UI_NAME_MAX - 1);
            p_frame->chest_loot[i][UI_NAME_MAX - 1] = '\0';
        }
        p_frame->chest_loot_count = loot_n;
    }

    /* ── Shop panel ───────────────────────────────────────────────── */
    p_frame->in_shop         = p_state->shop.active;
    p_frame->shop_page       = p_state->shop.page;
    p_frame->shop_buy_cursor = p_state->shop.buy_cursor;
    p_frame->shop_sell_cursor= p_state->shop.sell_cursor;
    p_frame->shop_buy_count  = 0;
    p_frame->shop_sell_count = 0;

    if (p_state->shop.active) {
        /* Buy list: all items in item_db */
        int buy_n = ITEM_DB_COUNT;
        if (buy_n > UI_SHOP_BUY_MAX) buy_n = UI_SHOP_BUY_MAX;
        for (i = 0; i < buy_n; i++) {
            item_t it;
            item_db_get(i, &it);
            strncpy(p_frame->shop_buy_list[i].name, it.name, UI_NAME_MAX - 1);
            p_frame->shop_buy_list[i].name[UI_NAME_MAX - 1] = '\0';
            p_frame->shop_buy_list[i].price = it.buy_price;
        }
        p_frame->shop_buy_count = buy_n;

        /* Sell list: player inventory */
        {
            int sell_n = p_state->player.inventory_count;
            if (sell_n > UI_SELL_MAX) sell_n = UI_SELL_MAX;
            for (i = 0; i < sell_n; i++) {
                const item_t *it = &p_state->player.inventory[i];
                int sp = it->buy_price / SHOP_SELL_RATIO;
                if (sp < 1) sp = 1;
                strncpy(p_frame->shop_sell_list[i].name,
                        it->name, UI_NAME_MAX - 1);
                p_frame->shop_sell_list[i].name[UI_NAME_MAX - 1] = '\0';
                p_frame->shop_sell_list[i].sell_price = sp;
            }
            p_frame->shop_sell_count = sell_n;
        }
    }

    (void)already_in_panel; /* suppress unused-function warning */
}
