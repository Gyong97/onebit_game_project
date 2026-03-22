/**
 * @file main.c
 * @brief Phase 3 debug game loop for the OneBit roguelike.
 *
 * Temporary verification harness: renders the map each turn and prints
 * a debug log showing player/monster coordinates and distance so Phase 3
 * tracking AI can be confirmed visually.
 *
 * Controls: W/A/S/D to move, Q to quit.
 */
#include <stdio.h>
#include <string.h>
#include "renderer.h"
#include "input.h"
#include "turn_manager.h"

/* ── Helpers ──────────────────────────────────────────────────────────── */

static int iabs(int v) { return v < 0 ? -v : v; }

/**
 * @brief Copy game_state_t into a render_frame_t for the renderer.
 *
 * The map tiles already contain TILE_PLAYER / TILE_MONSTER in sync,
 * so a plain memcpy of each row is sufficient.
 */
static void build_frame(const game_state_t *p_state, render_frame_t *p_frame)
{
    int r;
    int c;

    for (r = 0; r < MAP_HEIGHT; r++) {
        for (c = 0; c < MAP_WIDTH; c++) {
            p_frame->tiles[r][c] = p_state->map.rows[r][c];
        }
    }
    p_frame->player_hp    = p_state->player.hp;
    p_frame->player_max_hp = p_state->player.max_hp;
    p_frame->player_atk   = p_state->player.atk;
    p_frame->scroll_count  = p_state->map.scroll_count;
    p_frame->message[0]   = '\0';
}

/**
 * @brief Print debug coordinates and Manhattan distances.
 *
 * Called every frame so the user can watch monster positions close in.
 */
static void print_debug_log(const game_state_t *p_state)
{
    int i;
    int dist;

    printf("----------------------------------------\n");
    printf("[Player ] pos=(%d,%d)  HP=%d/%d  ATK=%d  DEPTH=%ld\n",
           p_state->player.x,
           p_state->player.y,
           p_state->player.hp,
           p_state->player.max_hp,
           p_state->player.atk,
           p_state->map.scroll_count);

    for (i = 0; i < MONSTER_MAX_COUNT; i++) {
        if (!p_state->monsters[i].alive) {
            continue;
        }
        dist = iabs(p_state->player.x - p_state->monsters[i].x)
             + iabs(p_state->player.y - p_state->monsters[i].y);
        printf("[Monster%d] pos=(%d,%d)  HP=%d  dist=%d\n",
               i,
               p_state->monsters[i].x,
               p_state->monsters[i].y,
               p_state->monsters[i].hp,
               dist);
    }
    printf("----------------------------------------\n");
    printf("Move: WASD  |  Quit: Q\n");
}

/* ── Entry point ──────────────────────────────────────────────────────── */

int main(void)
{
    game_state_t   state;
    render_frame_t frame;
    action_t       action;
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

    /* Pre-spawn two monsters at fixed positions to verify tracking AI */
    turn_manager_spawn_monster(&state, 4, 2); /* directly above player */
    turn_manager_spawn_monster(&state, 7, 5); /* diagonal upper-right  */

    /* ── Game loop ──────────────────────────────────────────────────── */
    for (;;) {
        renderer_clear();
        build_frame(&state, &frame);
        renderer_draw(&frame);
        print_debug_log(&state);

        if (input_get_action(&action) != 0) {
            break;
        }

        if (action == ACTION_QUIT) {
            break;
        }

        if (action == ACTION_NONE) {
            continue; /* unrecognised key — redraw without consuming a turn */
        }

        turn_result = turn_manager_player_act(&state, action);
        if (turn_result == TURN_GAME_OVER) {
            snprintf(frame.message, MSG_BUF_SIZE, "YOU DIED.");
            renderer_clear();
            build_frame(&state, &frame);
            renderer_draw(&frame);
            print_debug_log(&state);
            printf("\n*** GAME OVER ***\n");
            break;
        }
        if (turn_result == 1) {
            /* Player was blocked; show a brief note on next redraw */
            snprintf(frame.message, MSG_BUF_SIZE, "Blocked.");
        }
    }

    printf("\nGame ended. Final DEPTH: %ld\n", state.map.scroll_count);
    renderer_destroy();
    return 0;
}
