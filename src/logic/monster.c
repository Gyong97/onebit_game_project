/**
 * @file monster.c
 * @brief Stub — TDD red phase. Functions exist but do not fulfil the spec.
 * Tests compile and link, but assertions fail until the real implementation
 * replaces this file.
 */
#include "monster.h"

int monster_init(monster_t *p_monster, int x, int y)
{
    (void)p_monster;
    (void)x;
    (void)y;
    return 0; /* does not set any fields */
}

int monster_step(monster_t *p_monster, const player_t *p_player,
                 map_t *p_map)
{
    (void)p_monster;
    (void)p_player;
    (void)p_map;
    return 0; /* does not move */
}
