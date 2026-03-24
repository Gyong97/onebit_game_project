/**
 * @file main.c
 * @brief Entry point and game loop for the OneBit roguelike.
 *
 * Controls: W=up  S=down  A=left  D=right  Q=quit
 */
#include <stdio.h>
#include <string.h>
#include "renderer.h"
#include "input.h"
#include "turn_manager.h"
#include "player.h"   /* EQUIP_SLOT_* */

/* ── Helpers ──────────────────────────────────────────────────────────── */

/**
 * @brief Copy game_state_t into a render_frame_t snapshot for the renderer.
 */
static void build_frame(const game_state_t *p_state, render_frame_t *p_frame)
{
    int             r;
    int             c;
    const player_t *pl = &p_state->player;
    int             slot;
    const char     *name;

    for (r = 0; r < MAP_HEIGHT; r++) {
        for (c = 0; c < MAP_WIDTH; c++) {
            p_frame->tiles[r][c] = p_state->map.rows[r][c];
        }
    }

    p_frame->player_hp       = pl->hp;
    p_frame->player_max_hp   = pl->max_hp;
    p_frame->player_atk      = pl->atk;
    p_frame->player_def      = pl->def;
    p_frame->player_coins    = pl->coins;
    p_frame->inventory_count = pl->inventory_count;
    p_frame->scroll_count    = p_state->map.scroll_count;
    p_frame->message[0]      = '\0';

    slot = pl->equipment[EQUIP_SLOT_WEAPON];
    name = (slot != -1) ? pl->inventory[slot].name : "";
    strncpy(p_frame->equip_weapon, name, EQUIP_NAME_MAX - 1);
    p_frame->equip_weapon[EQUIP_NAME_MAX - 1] = '\0';

    slot = pl->equipment[EQUIP_SLOT_HEAD];
    name = (slot != -1) ? pl->inventory[slot].name : "";
    strncpy(p_frame->equip_head, name, EQUIP_NAME_MAX - 1);
    p_frame->equip_head[EQUIP_NAME_MAX - 1] = '\0';

    slot = pl->equipment[EQUIP_SLOT_BODY];
    name = (slot != -1) ? pl->inventory[slot].name : "";
    strncpy(p_frame->equip_body, name, EQUIP_NAME_MAX - 1);
    p_frame->equip_body[EQUIP_NAME_MAX - 1] = '\0';
}

/**
 * @brief Peek at the tile the player is about to step into.
 *
 * Used before turn_manager_player_act() so we can compose an event message
 * without duplicating move logic.
 */
static tile_type_t peek_target(const game_state_t *p_state, action_t action)
{
    int         tx   = p_state->player.x;
    int         ty   = p_state->player.y;
    tile_type_t tile = TILE_FLOOR;

    switch (action) {
        case ACTION_MOVE_UP:    ty--; break;
        case ACTION_MOVE_DOWN:  ty++; break;
        case ACTION_MOVE_LEFT:  tx--; break;
        case ACTION_MOVE_RIGHT: tx++; break;
        default: break;
    }
    map_get_tile(&p_state->map, tx, ty, &tile);
    return tile;
}

/* ── Entry point ──────────────────────────────────────────────────────── */

int main(void)
{
    game_state_t   state;
    render_frame_t frame;
    action_t       action;
    tile_type_t    target;
    int            turn_result;

    if (renderer_init() != 0) {
        fprintf(stderr, "error: renderer init failed\n");
        return 1;
    }

    if (turn_manager_init(&state) != 0) {
        fprintf(stderr, "error: turn_manager init failed\n");
        renderer_destroy();
        return 1;
    }

    /* ── Game loop ──────────────────────────────────────────────────── */
    for (;;) {
        renderer_clear();
        build_frame(&state, &frame);
        renderer_draw(&frame);

        if (input_get_action(&action) != 0) {
            break;
        }
        if (action == ACTION_QUIT) {
            break;
        }
        if (action == ACTION_NONE) {
            continue;
        }

        /* Peek target tile so we can show a contextual message */
        target = peek_target(&state, action);

        turn_result = turn_manager_player_act(&state, action);

        if (turn_result == TURN_GAME_OVER) {
            snprintf(frame.message, MSG_BUF_SIZE,
                     "YOU DIED at depth %ld!", state.map.scroll_count);
            renderer_clear();
            build_frame(&state, &frame);
            /* Restore message that build_frame cleared */
            snprintf(frame.message, MSG_BUF_SIZE,
                     "YOU DIED at depth %ld!", state.map.scroll_count);
            renderer_draw(&frame);
            printf("\n*** GAME OVER ***\n");
            break;
        }

        /* Set next-frame event message based on what the player did */
        if (turn_result == 1) {
            snprintf(frame.message, MSG_BUF_SIZE, "Blocked.");
        } else {
            switch (target) {
                case TILE_MONSTER:
                    snprintf(frame.message, MSG_BUF_SIZE, "You strike a monster!");
                    break;
                case TILE_CHEST:
                    snprintf(frame.message, MSG_BUF_SIZE,
                             "You open a chest! [BAG: %d]",
                             state.player.inventory_count);
                    break;
                case TILE_COIN:
                    snprintf(frame.message, MSG_BUF_SIZE,
                             "Coin! (%d total)", state.player.coins);
                    break;
                default:
                    break;
            }
        }

        /* Persist message into next frame */
        strncpy(frame.message,
                frame.message[0] != '\0' ? frame.message : "",
                MSG_BUF_SIZE - 1);
    }

    printf("\nFinal DEPTH: %ld  COINS: %d\n",
           state.map.scroll_count, state.player.coins);
    renderer_destroy();
    return 0;
}
