/**
 * @file turn_manager.c
 * @brief Stub — TDD red phase. Functions exist but do not fulfil the spec.
 * Tests compile and link, but assertions fail until the real implementation
 * replaces this file.
 */
#include "turn_manager.h"

int turn_manager_init(game_state_t *p_state)
{
    (void)p_state;
    return 0;
}

int turn_manager_player_act(game_state_t *p_state, action_t action)
{
    (void)p_state;
    (void)action;
    return 0;
}

int turn_manager_monsters_act(game_state_t *p_state)
{
    (void)p_state;
    return 0;
}

int turn_manager_shift_monsters(game_state_t *p_state)
{
    (void)p_state;
    return 0;
}

int turn_manager_spawn_monster(game_state_t *p_state, int x, int y)
{
    (void)p_state;
    (void)x;
    (void)y;
    return 0;
}

int turn_manager_spawn_row(game_state_t *p_state)
{
    (void)p_state;
    return 0;
}

int turn_manager_alive_count(const game_state_t *p_state)
{
    (void)p_state;
    return 0;
}
