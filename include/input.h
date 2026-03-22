/**
 * @file input.h
 * @brief Abstract input interface for the OneBit roguelike.
 *
 * This header maps raw keyboard input to high-level game actions.
 * The actual key-reading implementation is backend-specific (e.g. TUI
 * terminal), but callers only ever see action_t values.
 */
#ifndef INPUT_H
#define INPUT_H

/**
 * @brief High-level player actions derived from raw input.
 *
 * WASD maps to the four directional moves; QUIT terminates the game loop.
 */
typedef enum {
    ACTION_NONE       = 0, /* no recognised input (e.g. unknown key) */
    ACTION_MOVE_UP    = 1, /* 'W' — move player one cell up */
    ACTION_MOVE_DOWN  = 2, /* 'S' — move player one cell down */
    ACTION_MOVE_LEFT  = 3, /* 'A' — move player one cell left */
    ACTION_MOVE_RIGHT = 4, /* 'D' — move player one cell right */
    ACTION_QUIT       = 5  /* 'Q' — request game exit */
} action_t;

/* ── Input API ────────────────────────────────────────────────────────── */

/**
 * @brief Block until the player presses a recognised key, then return
 *        the corresponding action via the output parameter.
 *
 * @param p_action  Output: set to the action that matches the key pressed.
 *                  Must not be NULL.
 * @return 0 on success, negative value on failure (e.g. read error).
 */
int input_get_action(action_t *p_action);

#endif /* INPUT_H */
