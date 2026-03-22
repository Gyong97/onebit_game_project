/**
 * @file map.c
 * @brief Stub implementation — functions exist but do NOT fulfil the spec.
 *
 * TDD red phase: tests/test_map.c compiles and links against this file,
 * but all assertions will fail because the logic is not yet implemented.
 * Replace with the real implementation in the green phase.
 */
#include "map.h"

int map_init(map_t *p_map)
{
    (void)p_map;
    return 0; /* does not set up any tiles — tests will fail */
}

int map_scroll(map_t *p_map)
{
    (void)p_map;
    return 0; /* does not shift rows or generate new row */
}

int map_get_tile(const map_t *p_map, int x, int y, tile_type_t *p_tile)
{
    (void)p_map;
    (void)x;
    (void)y;
    (void)p_tile;
    return 0; /* does not write *p_tile — tests will fail */
}

int map_set_tile(map_t *p_map, int x, int y, tile_type_t tile)
{
    (void)p_map;
    (void)x;
    (void)y;
    (void)tile;
    return 0; /* does not write anything */
}
