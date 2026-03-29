/**
 * @file input_tui.c
 * @brief TUI (terminal) implementation of the input interface.
 *
 * Switches the terminal to raw / no-echo mode for the duration of one
 * keypress, reads a single character, then restores the original settings.
 * This lets the game loop react immediately to WASD without requiring Enter.
 *
 * Only display/input I/O lives here — no game logic.
 */
#include <stddef.h>     /* NULL           */
#include <stdio.h>      /* getchar        */
#include <termios.h>    /* tcgetattr etc. */
#include <unistd.h>     /* STDIN_FILENO   */
#include "input.h"

int input_get_action(action_t *p_action)
{
    struct termios old_tio;
    struct termios raw_tio;
    int ch;

    if (p_action == NULL) {
        return -1;
    }

    /* Save current terminal settings */
    if (tcgetattr(STDIN_FILENO, &old_tio) != 0) {
        return -1;
    }

    /* Switch to raw mode: one char at a time, no echo */
    raw_tio          = old_tio;
    raw_tio.c_lflag &= (unsigned int)~(ICANON | ECHO);
    raw_tio.c_cc[VMIN]  = 1;
    raw_tio.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw_tio) != 0) {
        return -1;
    }

    ch = getchar();

    /* Restore original settings unconditionally */
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

    switch (ch) {
        case 'w': case 'W': *p_action = ACTION_MOVE_UP;    break;
        case 's': case 'S': *p_action = ACTION_MOVE_DOWN;  break;
        case 'a': case 'A': *p_action = ACTION_MOVE_LEFT;  break;
        case 'd': case 'D': *p_action = ACTION_MOVE_RIGHT; break;
        case 'q': case 'Q': *p_action = ACTION_QUIT;       break;
        case ' ':            *p_action = ACTION_SPACE;      break;
        default:             *p_action = ACTION_NONE;       break;
    }
    return 0;
}
