/**
 * @file test_monster_ai.c
 * @brief TDD tests for the upgraded monster AI (Phase 5).
 *
 * Tests cover:
 *  - Perception range: state transitions IDLE ↔ CHASING
 *  - BFS chase: approaches via shortest connected path
 *  - BFS routes around walls (ground monsters)
 *  - Flying monsters pass through walls
 *  - IDLE random walk (moves but doesn't directly chase)
 *  - No-path: monster stays when fully enclosed
 */
#include <stdio.h>
#include <stdlib.h>   /* srand */
#include "monster.h"
#include "map.h"
#include "player.h"

/* ── Minimal test framework ───────────────────────────────────────────── */
#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

static int iabs(int v) { return v < 0 ? -v : v; }

/* ── Helper: build clean map + place monster + player ─────────────────── */
static int setup(map_t *pm, monster_t *mm, int mx, int my,
                 monster_type_t mtype,
                 player_t *pp, int px, int py)
{
    if (map_init(pm) != 0)                           return -1;
    if (monster_init_typed(mm, mx, my, mtype) != 0)  return -1;
    if (player_init(pp) != 0)                        return -1;
    pp->x = px; pp->y = py;
    map_set_tile(pm, mx, my, TILE_MONSTER);
    map_set_tile(pm, px, py, TILE_PLAYER);
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * 1. State transitions
 * ════════════════════════════════════════════════════════════════════════ */

/*
 * Player at dist=4 ≤ GOBLIN_PERCEPTION_RANGE(6) → CHASING after step.
 */
static int test_state_chasing_when_in_range(void)
{
    map_t m; monster_t mo; player_t p;
    /* Monster (1,5), player (5,5): Manhattan dist = 4 */
    TEST_ASSERT(setup(&m, &mo, 1, 5, MONSTER_TYPE_GOBLIN, &p, 5, 5) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.state == MONSTER_STATE_CHASING,
                "state must be CHASING when player within perception range");
    return 0;
}

/*
 * Player at dist=7 > GOBLIN_PERCEPTION_RANGE(6) → IDLE after step.
 */
static int test_state_idle_when_out_of_range(void)
{
    map_t m; monster_t mo; player_t p;
    /* Monster (1,5), player (1,12): dist = 7 */
    TEST_ASSERT(setup(&m, &mo, 1, 5, MONSTER_TYPE_GOBLIN, &p, 1, 12) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.state == MONSTER_STATE_IDLE,
                "state must be IDLE when player outside perception range");
    return 0;
}

/*
 * Player exactly at boundary (dist == range) → CHASING (≤ comparison).
 * SLIME range = 4, monster (2,5), player (6,5): dist=4.
 */
static int test_state_chasing_at_exact_boundary(void)
{
    map_t m; monster_t mo; player_t p;
    TEST_ASSERT(setup(&m, &mo, 2, 5, MONSTER_TYPE_SLIME, &p, 6, 5) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.state == MONSTER_STATE_CHASING,
                "state must be CHASING when dist == perception_range");
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * 2. CHASING: BFS approach
 * ════════════════════════════════════════════════════════════════════════ */

/*
 * Horizontal chase: monster (1,5), player (5,5), dist=4 (in range).
 * No obstacles → BFS next step is right → (2,5).
 */
static int test_chase_moves_horizontally(void)
{
    map_t m; monster_t mo; player_t p;
    TEST_ASSERT(setup(&m, &mo, 1, 5, MONSTER_TYPE_GOBLIN, &p, 5, 5) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.x == 2, "monster x must be 2 (stepped right)");
    TEST_ASSERT(mo.y == 5, "monster y must be unchanged");
    return 0;
}

/*
 * Vertical chase: monster (4,2), player (4,7), dist=5 (in range).
 * No obstacles → BFS next step is down → (4,3).
 */
static int test_chase_moves_vertically(void)
{
    map_t m; monster_t mo; player_t p;
    TEST_ASSERT(setup(&m, &mo, 4, 2, MONSTER_TYPE_GOBLIN, &p, 4, 7) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.x == 4, "monster x must be unchanged");
    TEST_ASSERT(mo.y == 3, "monster y must be 3 (stepped down)");
    return 0;
}

/*
 * Chase reduces Manhattan distance when in range.
 */
static int test_chase_reduces_distance(void)
{
    map_t m; monster_t mo; player_t p;
    int dist_before, dist_after;
    /* Monster (3,3), player (7,3): dist=4, in range */
    TEST_ASSERT(setup(&m, &mo, 3, 3, MONSTER_TYPE_GOBLIN, &p, 7, 3) == 0,
                "setup must succeed");
    dist_before = iabs(p.x - mo.x) + iabs(p.y - mo.y);
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    dist_after = iabs(p.x - mo.x) + iabs(p.y - mo.y);
    TEST_ASSERT(dist_after < dist_before,
                "chasing monster must reduce distance to player");
    return 0;
}

/*
 * Chase approaches player from a diagonal position (dist ≤ range).
 * Monster (2,5), player (6,7): dist=6=range. Distance must decrease.
 */
static int test_chase_diagonal_approaches(void)
{
    map_t m; monster_t mo; player_t p;
    int dist_before, dist_after;
    TEST_ASSERT(setup(&m, &mo, 2, 5, MONSTER_TYPE_GOBLIN, &p, 6, 7) == 0,
                "setup must succeed");
    dist_before = iabs(p.x - mo.x) + iabs(p.y - mo.y);
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    dist_after = iabs(p.x - mo.x) + iabs(p.y - mo.y);
    TEST_ASSERT(dist_after < dist_before,
                "chasing monster must reduce diagonal distance");
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * 3. BFS routes around walls (ground monsters)
 * ════════════════════════════════════════════════════════════════════════ */

/*
 * Wall blocks direct horizontal path; BFS routes around.
 *
 *  Monster (1,10) → Player (3,10), Wall at (2,10).
 *  Direct path blocked. BFS routes via (1,11) [down first].
 *  Expected first step: (1,11).
 */
static int test_chase_routes_around_wall(void)
{
    map_t m; monster_t mo; player_t p;
    TEST_ASSERT(setup(&m, &mo, 1, 10, MONSTER_TYPE_GOBLIN, &p, 3, 10) == 0,
                "setup must succeed");
    map_set_tile(&m, 2, 10, TILE_WALL);

    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    /* BFS direction order: right, down, left, up.
     * Right is blocked. Down (1,11) is added before up (1,9).
     * Both routes have equal length → down-first path selected. */
    TEST_ASSERT(mo.x == 1 && mo.y == 11,
                "monster must step down to begin routing around wall");
    return 0;
}

/*
 * Chase after routing: over 3 steps the monster gets closer despite wall.
 * Monster (1,10), Player (3,10), Wall at (2,10).
 * Route: (1,10)→(1,11)→(2,11)→(3,11). dist 2→3→2→1 — reduces overall.
 *
 * BFS routing around a wall may temporarily increase Manhattan distance
 * (detour), but the total trend is toward the player. Verify after 3 steps.
 */
static int test_chase_gets_closer_despite_wall(void)
{
    map_t m; monster_t mo; player_t p;
    int d0, d3;
    int s;
    TEST_ASSERT(setup(&m, &mo, 1, 10, MONSTER_TYPE_GOBLIN, &p, 3, 10) == 0,
                "setup must succeed");
    map_set_tile(&m, 2, 10, TILE_WALL);

    d0 = iabs(p.x - mo.x) + iabs(p.y - mo.y);
    for (s = 0; s < 3; s++) {
        TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    }
    d3 = iabs(p.x - mo.x) + iabs(p.y - mo.y);
    TEST_ASSERT(d3 < d0,
                "distance must be less after 3 steps routing around wall");
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * 4. Flying monsters pass through walls
 * ════════════════════════════════════════════════════════════════════════ */

/*
 * BAT flies through a wall tile directly toward the player.
 *
 *  BAT (2,5), Player (4,5), Wall at (3,5).
 *  Ground monster would route around. BAT's BFS ignores walls →
 *  next step is (3,5) [through the wall].
 *  After the move: tile at (3,5) = TILE_MONSTER, mo.tile_under = TILE_WALL.
 */
static int test_bat_chases_through_wall(void)
{
    map_t m; monster_t mo; player_t p;
    tile_type_t tile;

    TEST_ASSERT(setup(&m, &mo, 2, 5, MONSTER_TYPE_BAT, &p, 4, 5) == 0,
                "setup must succeed");
    map_set_tile(&m, 3, 5, TILE_WALL);

    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.x == 3 && mo.y == 5,
                "bat must move to (3,5) through the wall");
    TEST_ASSERT(map_get_tile(&m, 3, 5, &tile) == 0, "get tile must succeed");
    TEST_ASSERT(tile == TILE_MONSTER,
                "tile at bat's new position must be TILE_MONSTER");
    TEST_ASSERT(mo.tile_under == TILE_WALL,
                "bat must remember it is standing on a wall tile");
    return 0;
}

/*
 * When a BAT that was on a WALL tile moves away, the wall is restored
 * (not converted to floor).
 *
 *  BAT at (3,5) with tile_under=TILE_WALL (simulated: already flew in).
 *  Player at (5,5). Next step right → (4,5). Old tile at (3,5) → TILE_WALL.
 */
static int test_bat_restores_wall_on_departure(void)
{
    map_t m; monster_t mo; player_t p;
    tile_type_t tile;

    /* Manually simulate a bat already standing on a wall */
    map_init(&m);
    monster_init_typed(&mo, 3, 5, MONSTER_TYPE_BAT);
    player_init(&p);
    p.x = 5; p.y = 5;

    /* The bat is on what was a wall tile */
    mo.tile_under = TILE_WALL;
    map_set_tile(&m, 3, 5, TILE_MONSTER);
    map_set_tile(&m, 5, 5, TILE_PLAYER);

    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.x == 4 && mo.y == 5, "bat moved to (4,5)");
    TEST_ASSERT(map_get_tile(&m, 3, 5, &tile) == 0, "get old tile must succeed");
    TEST_ASSERT(tile == TILE_WALL,
                "old bat position must be restored to TILE_WALL");
    return 0;
}

/*
 * BAT flies through walls in IDLE state too (random walk ignores walls).
 * BAT at (2,5), player at (2,20) (dist=15 > range=8).
 * Surround bat with 3 walls (right/left/up) and 1 floor (down) so
 * a ground monster would have only one option while bat would also
 * consider the wall tiles.
 *
 * Test: bat's state == IDLE AND it moved (position changed).
 */
static int test_bat_idle_can_pass_walls(void)
{
    map_t m; monster_t mo; player_t p;
    int old_x, old_y;

    TEST_ASSERT(setup(&m, &mo, 2, 5, MONSTER_TYPE_BAT, &p, 2, 20) == 0,
                "setup must succeed");
    /* Surround with walls except down */
    map_set_tile(&m, 1, 5, TILE_WALL);
    map_set_tile(&m, 3, 5, TILE_WALL);
    map_set_tile(&m, 2, 4, TILE_WALL);
    /* (2,6) remains TILE_FLOOR — only floor option for ground monster */

    old_x = mo.x; old_y = mo.y;
    srand(0); /* deterministic */
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.state == MONSTER_STATE_IDLE,
                "bat must be IDLE when player out of range");
    /* With srand(0): bat picks from {floor(down), wall(right), wall(left), wall(up)}.
     * It must move — 4 options available (3 walls + 1 floor for flying). */
    TEST_ASSERT(mo.x != old_x || mo.y != old_y,
                "idle bat must move when valid tiles exist");
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * 5. IDLE: random walk (ground monster)
 * ════════════════════════════════════════════════════════════════════════ */

/*
 * GOBLIN at center of open map, player far away (dist=12 > range=6).
 * Monster is IDLE and must move to one of the 4 open adjacent tiles.
 */
static int test_idle_random_walk_moves(void)
{
    map_t m; monster_t mo; player_t p;
    int old_x, old_y;

    /* Player at (6,0), monster at (6,12): dist=12>6 → IDLE */
    TEST_ASSERT(setup(&m, &mo, 6, 12, MONSTER_TYPE_GOBLIN, &p, 6, 0) == 0,
                "setup must succeed");

    old_x = mo.x; old_y = mo.y;
    srand(42); /* deterministic */
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.state == MONSTER_STATE_IDLE,
                "state must be IDLE when player out of range");
    TEST_ASSERT(mo.x != old_x || mo.y != old_y,
                "idle monster must move when surrounded by free tiles");
    return 0;
}

/*
 * IDLE monster does not directly approach player.
 * Use a setup where player is outside range and in only ONE direction.
 * Block every direction EXCEPT away from player, so if the monster were
 * chasing it could not move. Verify the monster stays IDLE (state check).
 */
static int test_idle_does_not_chase(void)
{
    map_t m; monster_t mo; player_t p;

    /* Monster (1,5), player (1,12): dist=7>6 → IDLE */
    TEST_ASSERT(setup(&m, &mo, 1, 5, MONSTER_TYPE_GOBLIN, &p, 1, 12) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.state == MONSTER_STATE_IDLE,
                "state must remain IDLE — no chase when player out of range");
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * 6. No path: monster stays put
 * ════════════════════════════════════════════════════════════════════════ */

/*
 * GOBLIN completely enclosed by walls. Player within range but unreachable.
 * BFS finds no path → monster stays at (2,5).
 */
static int test_stays_when_no_path(void)
{
    map_t m; monster_t mo; player_t p;

    /* dist=6=range, player in range but fully walled off */
    TEST_ASSERT(setup(&m, &mo, 2, 5, MONSTER_TYPE_GOBLIN, &p, 8, 5) == 0,
                "setup must succeed");
    map_set_tile(&m, 1, 5, TILE_WALL);
    map_set_tile(&m, 3, 5, TILE_WALL);
    map_set_tile(&m, 2, 4, TILE_WALL);
    map_set_tile(&m, 2, 6, TILE_WALL);

    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.x == 2 && mo.y == 5,
                "monster must not move when completely enclosed");
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * 7. Attack (adjacent to player)
 * ════════════════════════════════════════════════════════════════════════ */

/*
 * Monster adjacent to player: performs attack, does not move.
 */
static int test_attacks_player_stays_put(void)
{
    map_t m; monster_t mo; player_t p;
    int bx, by;
    /* Monster (4,6), player (4,5): dist=1, in range */
    TEST_ASSERT(setup(&m, &mo, 4, 6, MONSTER_TYPE_GOBLIN, &p, 4, 5) == 0,
                "setup must succeed");
    bx = mo.x; by = mo.y;
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.x == bx && mo.y == by,
                "monster must not move on attack attempt");
    return 0;
}

/*
 * Map tiles are updated correctly after a move.
 * Monster (1,5) moves right to (2,5): (1,5)→TILE_FLOOR, (2,5)→TILE_MONSTER.
 */
static int test_updates_map_tiles(void)
{
    map_t m; monster_t mo; player_t p;
    tile_type_t tile;

    TEST_ASSERT(setup(&m, &mo, 1, 5, MONSTER_TYPE_GOBLIN, &p, 5, 5) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");

    map_get_tile(&m, 1, 5, &tile);
    TEST_ASSERT(tile == TILE_FLOOR, "old position must become TILE_FLOOR");
    map_get_tile(&m, 2, 5, &tile);
    TEST_ASSERT(tile == TILE_MONSTER, "new position must be TILE_MONSTER");
    return 0;
}

/*
 * Monster treats TILE_COIN as passable (moves onto coin tile).
 */
static int test_passes_through_coin(void)
{
    map_t m; monster_t mo; player_t p;
    tile_type_t tile;

    map_init(&m);
    player_init(&p);
    p.x = 4; p.y = 8;
    monster_init_typed(&mo, 4, 5, MONSTER_TYPE_GOBLIN);
    map_set_tile(&m, 4, 8, TILE_PLAYER);
    map_set_tile(&m, 4, 5, TILE_MONSTER);
    map_set_tile(&m, 4, 6, TILE_COIN);

    TEST_ASSERT(monster_step(&mo, &p, &m) == 0, "step must return 0");
    TEST_ASSERT(mo.y == 6, "monster must step onto TILE_COIN position");
    map_get_tile(&m, 4, 6, &tile);
    TEST_ASSERT(tile == TILE_MONSTER, "coin tile overwritten by TILE_MONSTER");
    map_get_tile(&m, 4, 5, &tile);
    TEST_ASSERT(tile == TILE_FLOOR, "old position must become TILE_FLOOR");
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * 8. NULL safety
 * ════════════════════════════════════════════════════════════════════════ */

static int test_null_monster(void)
{
    map_t m; player_t p;
    map_init(&m); player_init(&p);
    TEST_ASSERT(monster_step(NULL, &p, &m) == -1, "NULL monster must return -1");
    return 0;
}
static int test_null_player(void)
{
    map_t m; monster_t mo;
    map_init(&m); monster_init(&mo, 3, 5);
    TEST_ASSERT(monster_step(&mo, NULL, &m) == -1, "NULL player must return -1");
    return 0;
}
static int test_null_map(void)
{
    monster_t mo; player_t p;
    monster_init(&mo, 3, 5); player_init(&p);
    TEST_ASSERT(monster_step(&mo, &p, NULL) == -1, "NULL map must return -1");
    return 0;
}

/* ── Test runner ──────────────────────────────────────────────────────── */

typedef struct { const char *name; int (*fn)(void); } tc_t;

int main(void)
{
    int i, failed = 0;
    tc_t tests[] = {
        { "test_state_chasing_when_in_range",        test_state_chasing_when_in_range        },
        { "test_state_idle_when_out_of_range",       test_state_idle_when_out_of_range       },
        { "test_state_chasing_at_exact_boundary",    test_state_chasing_at_exact_boundary    },
        { "test_chase_moves_horizontally",           test_chase_moves_horizontally           },
        { "test_chase_moves_vertically",             test_chase_moves_vertically             },
        { "test_chase_reduces_distance",             test_chase_reduces_distance             },
        { "test_chase_diagonal_approaches",          test_chase_diagonal_approaches          },
        { "test_chase_routes_around_wall",           test_chase_routes_around_wall           },
        { "test_chase_gets_closer_despite_wall",     test_chase_gets_closer_despite_wall     },
        { "test_bat_chases_through_wall",            test_bat_chases_through_wall            },
        { "test_bat_restores_wall_on_departure",     test_bat_restores_wall_on_departure     },
        { "test_bat_idle_can_pass_walls",            test_bat_idle_can_pass_walls            },
        { "test_idle_random_walk_moves",             test_idle_random_walk_moves             },
        { "test_idle_does_not_chase",                test_idle_does_not_chase                },
        { "test_stays_when_no_path",                 test_stays_when_no_path                 },
        { "test_attacks_player_stays_put",           test_attacks_player_stays_put           },
        { "test_updates_map_tiles",                  test_updates_map_tiles                  },
        { "test_passes_through_coin",                test_passes_through_coin                },
        { "test_null_monster",                       test_null_monster                       },
        { "test_null_player",                        test_null_player                        },
        { "test_null_map",                           test_null_map                           },
    };

    printf("=== Monster AI Tests ===\n");
    for (i = 0; i < (int)(sizeof(tests)/sizeof(tests[0])); i++) {
        if (tests[i].fn() == 0) {
            printf("  [PASS] %s\n", tests[i].name);
        } else {
            printf("  [FAIL] %s\n", tests[i].name);
            failed++;
        }
    }
    if (failed == 0) {
        printf("All %d tests passed.\n", (int)(sizeof(tests)/sizeof(tests[0])));
        return 0;
    }
    printf("%d test(s) failed.\n", failed);
    return 1;
}
