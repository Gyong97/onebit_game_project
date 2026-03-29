/**
 * @file frame_builder.h
 * @brief Frame builder — the UI bridge between game state and renderer.
 *
 * This module translates the complete game_state_t into a render_frame_t
 * snapshot.  It is the single point of contact between game logic and the
 * renderer, enabling the renderer to be swapped (CLI → GUI) without any
 * changes to logic code.
 *
 * Usage:
 *   render_frame_t frame;
 *   frame_builder_build(&frame, &state, &save);
 *   renderer_draw(&frame);
 *
 * No stdio output: this module only fills data structures.
 */
#ifndef FRAME_BUILDER_H
#define FRAME_BUILDER_H

#include "renderer.h"      /* render_frame_t */
#include "turn_manager.h"  /* game_state_t   */
#include "save_manager.h"  /* save_data_t    */

/**
 * @brief Build a complete render frame from current game state.
 *
 * Populates all fields of p_frame:
 *  - Map tiles (viewport slice from MAP_BUFFER_H offset)
 *  - Player HUD: hp, atk, def, coins, level, xp, xp_to_next, show_levelup
 *  - Equipment slot names
 *  - Monster info panel: adjacent + last-attacked monsters with direction
 *  - Chest loot panel: items from last chest open (when ttl > 0)
 *  - Shop panel: buy/sell lists when shop is active
 *  - Event message (copied unchanged from p_frame — caller sets it)
 *
 * @param p_frame   Output frame; must not be NULL.  Caller must zero it
 *                  (or set message) before calling; this function does NOT
 *                  clear the message field.
 * @param p_state   Current game state; must not be NULL.
 * @param p_save    Persistent save data (best_depth etc.); must not be NULL.
 */
void frame_builder_build(render_frame_t     *p_frame,
                         const game_state_t *p_state,
                         const save_data_t  *p_save);

#endif /* FRAME_BUILDER_H */
