/**
 * @file main.c
 * @brief Entry point for the OneBit roguelike.
 *
 * Phase 1 skeleton: initialises the renderer and draws one static test frame
 * to verify the build pipeline end-to-end. Full game-loop logic will be added
 * in subsequent phases.
 */
#include <stdio.h>
#include <string.h>
#include "renderer.h"
#include "input.h"

int main(void)
{
    render_frame_t frame;
    int row;
    int col;

    if (renderer_init() != 0) {
        fprintf(stderr, "error: failed to initialise renderer\n");
        return 1;
    }

    /* Build a minimal test frame: walls on the border, floor inside */
    for (row = 0; row < MAP_HEIGHT; row++) {
        for (col = 0; col < MAP_WIDTH; col++) {
            if (row == 0 || row == MAP_HEIGHT - 1 ||
                col == 0 || col == MAP_WIDTH - 1) {
                frame.tiles[row][col] = TILE_WALL;
            } else {
                frame.tiles[row][col] = TILE_FLOOR;
            }
        }
    }
    frame.tiles[1][1]    = TILE_PLAYER;
    frame.player_hp      = 100;
    frame.player_max_hp  = 100;
    frame.player_atk     = 10;
    frame.message[0]     = '\0';

    renderer_draw(&frame);
    renderer_destroy();
    return 0;
}
