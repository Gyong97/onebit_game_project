/**
 * @file renderer_tui.c
 * @brief TUI (terminal) implementation of the renderer interface.
 *
 * Draws the game state to stdout using plain ASCII characters and
 * ANSI escape codes for screen clearing. No ncurses dependency required.
 *
 * Separation of concerns: this file contains ONLY display logic.
 * It must never call game-logic functions or modify any game state.
 */
#include <stdio.h>
#include "renderer.h"

/* ── Tile → ASCII character lookup ───────────────────────────────────── */
static const char TILE_CHARS[] = {
    [TILE_FLOOR]      = '.',
    [TILE_WALL]       = '#',
    [TILE_PLAYER]     = 'P',
    [TILE_MONSTER]    = 'M',
    [TILE_CHEST]      = 'C',
    [TILE_COIN]       = '$',
    [TILE_SHOP]       = 'S',
    [TILE_CHEST_OPEN] = 'c',
    [TILE_SHOP_OPEN]  = 's'
};

/* ── Public API implementation ────────────────────────────────────────── */

int renderer_init(void)
{
    /* Basic TUI needs no special initialisation. */
    return 0;
}

int renderer_draw(const render_frame_t *p_frame)
{
    int row;
    int col;
    int i;

    if (p_frame == NULL) {
        return -1;
    }

    /* ── Map grid ─────────────────────────────────────────────────── */
    for (row = 0; row < MAP_HEIGHT; row++) {
        for (col = 0; col < MAP_WIDTH; col++) {
            tile_type_t tile = p_frame->tiles[row][col];
            putchar(TILE_CHARS[tile]);
        }
        putchar('\n');
    }

    /* ── Player HUD ───────────────────────────────────────────────── */
    printf("HP: %d/%d  ATK: %d  DEF: %d  COINS: %d  DEPTH: %ld  BEST: %ld\n",
           p_frame->player_hp,
           p_frame->player_max_hp,
           p_frame->player_atk,
           p_frame->player_def,
           p_frame->player_coins,
           p_frame->scroll_count,
           p_frame->best_depth);

    /* Level / XP bar */
    printf("LV: %d  XP: %d/%d",
           p_frame->player_level,
           p_frame->player_xp,
           p_frame->player_xp_to_next);
    if (p_frame->show_levelup) {
        printf("  *** LEVEL UP! ***");
    }
    putchar('\n');

    /* Equipment slots */
    printf("[W]%-6s  [H]%-6s  [B]%-6s  BAG:%d\n",
           p_frame->equip_weapon[0] != '\0' ? p_frame->equip_weapon : "--",
           p_frame->equip_head[0]   != '\0' ? p_frame->equip_head   : "--",
           p_frame->equip_body[0]   != '\0' ? p_frame->equip_body   : "--",
           p_frame->inventory_count);

    /* ── Monster info panel ───────────────────────────────────────── */
    for (i = 0; i < UI_MONSTER_PANEL_MAX; i++) {
        const ui_monster_entry_t *e = &p_frame->monster_panel[i];
        if (!e->active) continue;
        if (e->dir[0] != '\0') {
            printf("[%s] %-8s  HP: %d/%d\n",
                   e->dir, e->name, e->hp, e->max_hp);
        } else {
            printf("[~] %-8s  HP: %d/%d\n",
                   e->name, e->hp, e->max_hp);
        }
    }

    /* ── Chest loot panel ─────────────────────────────────────────── */
    if (p_frame->chest_loot_count > 0) {
        printf("Chest: ");
        for (i = 0; i < p_frame->chest_loot_count; i++) {
            if (i > 0) printf(", ");
            printf("%s", p_frame->chest_loot[i]);
        }
        putchar('\n');
    }

    /* ── Shop panel ───────────────────────────────────────────────── */
    if (p_frame->in_shop) {
        if (p_frame->shop_page == 0) { /* SHOP_PAGE_BUY */
            printf("=== SHOP: BUY ===  [W/S]=select  [A/D]=page  [SPC]=buy  [Q]=exit\n");
            for (i = 0; i < p_frame->shop_buy_count; i++) {
                printf("%s %-12s  %2d coins\n",
                       (i == p_frame->shop_buy_cursor) ? ">" : " ",
                       p_frame->shop_buy_list[i].name,
                       p_frame->shop_buy_list[i].price);
            }
        } else {
            printf("=== SHOP: SELL ===  [W/S]=select  [A/D]=page  [SPC]=sell  [Q]=exit\n");
            if (p_frame->shop_sell_count == 0) {
                printf("  (no items)\n");
            } else {
                for (i = 0; i < p_frame->shop_sell_count; i++) {
                    printf("%s %-12s  sell %d coins\n",
                           (i == p_frame->shop_sell_cursor) ? ">" : " ",
                           p_frame->shop_sell_list[i].name,
                           p_frame->shop_sell_list[i].sell_price);
                }
            }
        }
        printf("Coins: %d\n", p_frame->player_coins);
    }

    /* ── Event message ────────────────────────────────────────────── */
    if (p_frame->message[0] != '\0') {
        printf("> %s\n", p_frame->message);
    }

    fflush(stdout);
    return 0;
}

int renderer_clear(void)
{
    /* ANSI escape: clear screen (\033[2J) then move cursor to home (\033[H) */
    printf("\033[2J\033[H");
    fflush(stdout);
    return 0;
}

void renderer_destroy(void)
{
    /* No resources were allocated by this backend; nothing to release. */
}
