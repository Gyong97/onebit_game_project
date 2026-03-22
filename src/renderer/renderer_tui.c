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
    [TILE_FLOOR]   = '.',
    [TILE_WALL]    = '#',
    [TILE_PLAYER]  = 'P',
    [TILE_MONSTER] = 'M',
    [TILE_CHEST]   = 'C',
    [TILE_COIN]    = '$'
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

    if (p_frame == NULL) {
        return -1;
    }

    /* Draw the map grid row by row */
    for (row = 0; row < MAP_HEIGHT; row++) {
        for (col = 0; col < MAP_WIDTH; col++) {
            tile_type_t tile = p_frame->tiles[row][col];
            putchar(TILE_CHARS[tile]);
        }
        putchar('\n');
    }

    /* Draw the HUD: player stats + depth */
    printf("HP: %d/%d  ATK: %d  DEPTH: %ld\n",
           p_frame->player_hp,
           p_frame->player_max_hp,
           p_frame->player_atk,
           p_frame->scroll_count);

    /* Draw the event message if one is present */
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
