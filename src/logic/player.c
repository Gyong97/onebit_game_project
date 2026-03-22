/**
 * @file player.c
 * @brief Stub implementation — functions exist but do NOT fulfil the spec.
 *
 * TDD red phase: tests/test_player.c compiles and links against this file,
 * but all assertions will fail because the logic is not yet implemented.
 * Replace with the real implementation in the green phase.
 */
#include "player.h"

int player_init(player_t *p_player)
{
    (void)p_player;
    return 0; /* does not set any fields — tests will fail */
}

int player_move(player_t *p_player, action_t action, map_t *p_map)
{
    (void)p_player;
    (void)action;
    (void)p_map;
    return 0; /* does not move, scroll, or block — tests will fail */
}
