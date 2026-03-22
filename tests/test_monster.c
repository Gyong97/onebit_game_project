/**
 * @file test_monster.c
 * @brief TDD tests for the monster module (Plan 3).
 *
 * Written BEFORE the real implementation — all tests should FAIL when
 * linked against stub monster.c, and pass after implementation.
 *
 * Return convention: 0 = pass, -1 = fail.
 */
#include <stdio.h>
#include "monster.h"

/* ── Minimal test framework ───────────────────────────────────────────── */
#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

/* ── Helper: absolute value without stdlib ────────────────────────────── */
static int iabs(int v) { return v < 0 ? -v : v; }

/* ── monster_init tests ───────────────────────────────────────────────── */

static int test_monster_init_hp(void)
{
    monster_t m;
    TEST_ASSERT(monster_init(&m, 3, 5) == 0, "monster_init should return 0");
    TEST_ASSERT(m.hp == MONSTER_INIT_HP,
                "initial hp must equal MONSTER_INIT_HP");
    return 0;
}

static int test_monster_init_atk(void)
{
    monster_t m;
    TEST_ASSERT(monster_init(&m, 3, 5) == 0, "monster_init should return 0");
    TEST_ASSERT(m.atk == MONSTER_INIT_ATK,
                "initial atk must equal MONSTER_INIT_ATK");
    return 0;
}

static int test_monster_init_position(void)
{
    monster_t m;
    TEST_ASSERT(monster_init(&m, 3, 5) == 0, "monster_init should return 0");
    TEST_ASSERT(m.x == 3, "monster x must be set to the given value");
    TEST_ASSERT(m.y == 5, "monster y must be set to the given value");
    return 0;
}

static int test_monster_init_alive(void)
{
    monster_t m;
    TEST_ASSERT(monster_init(&m, 3, 5) == 0, "monster_init should return 0");
    TEST_ASSERT(m.alive != 0, "monster must be alive after monster_init");
    return 0;
}

static int test_monster_init_null(void)
{
    TEST_ASSERT(monster_init(NULL, 0, 0) == -1,
                "monster_init(NULL, ...) must return -1");
    return 0;
}

/* ── monster_step tests ───────────────────────────────────────────────── */

/*
 * Helper: build a map with walls at borders and floor inside,
 * place monster and player tiles.
 */
static int setup_map_with_entities(map_t *p_map,
                                   monster_t *p_monster, int mx, int my,
                                   player_t  *p_player,  int px, int py)
{
    if (map_init(p_map) != 0)            return -1;
    if (monster_init(p_monster, mx, my) != 0) return -1;
    if (player_init(p_player)  != 0)     return -1;

    p_player->x = px;
    p_player->y = py;

    map_set_tile(p_map, mx, my, TILE_MONSTER);
    map_set_tile(p_map, px, py, TILE_PLAYER);
    return 0;
}

/* Monster to the left of player (same row): should step right (closer) */
static int test_monster_step_moves_horizontally_toward_player(void)
{
    map_t    map;
    monster_t m;
    player_t  p;

    /* Monster at (1,5), player at (5,5): dx=+4, dy=0 → step right */
    TEST_ASSERT(setup_map_with_entities(&map, &m, 1, 5, &p, 5, 5) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step should return 0");
    TEST_ASSERT(m.x == 2, "monster x must increase by 1 (step right)");
    TEST_ASSERT(m.y == 5, "monster y must not change");
    return 0;
}

/* Monster above player (same column): should step down (closer) */
static int test_monster_step_moves_vertically_toward_player(void)
{
    map_t    map;
    monster_t m;
    player_t  p;

    /* Monster at (4,2), player at (4,7): dx=0, dy=+5 → step down */
    TEST_ASSERT(setup_map_with_entities(&map, &m, 4, 2, &p, 4, 7) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step should return 0");
    TEST_ASSERT(m.x == 4, "monster x must not change");
    TEST_ASSERT(m.y == 3, "monster y must increase by 1 (step down)");
    return 0;
}

/* Diagonal with |dx| > |dy|: prefers horizontal step */
static int test_monster_step_prefers_horizontal_on_wide_diagonal(void)
{
    map_t    map;
    monster_t m;
    player_t  p;

    /* Monster at (2,5), player at (6,7): dx=+4, dy=+2 → prefer horizontal */
    TEST_ASSERT(setup_map_with_entities(&map, &m, 2, 5, &p, 6, 7) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step should return 0");
    TEST_ASSERT(m.x == 3, "monster x must increase by 1 (horizontal preferred)");
    TEST_ASSERT(m.y == 5, "monster y must not change");
    return 0;
}

/* Diagonal with |dy| > |dx|: prefers vertical step */
static int test_monster_step_prefers_vertical_on_tall_diagonal(void)
{
    map_t    map;
    monster_t m;
    player_t  p;

    /* Monster at (3,1), player at (5,8): dx=+2, dy=+7 → prefer vertical */
    TEST_ASSERT(setup_map_with_entities(&map, &m, 3, 1, &p, 5, 8) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step should return 0");
    TEST_ASSERT(m.x == 3, "monster x must not change");
    TEST_ASSERT(m.y == 2, "monster y must increase by 1 (vertical preferred)");
    return 0;
}

/* Equal |dx|==|dy|: tie broken by horizontal preference */
static int test_monster_step_ties_broken_by_horizontal(void)
{
    map_t    map;
    monster_t m;
    player_t  p;

    /* Monster at (2,3), player at (5,6): dx=+3, dy=+3 → horizontal wins tie */
    TEST_ASSERT(setup_map_with_entities(&map, &m, 2, 3, &p, 5, 6) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step should return 0");
    TEST_ASSERT(m.x == 3, "monster x must increase by 1 (horizontal wins tie)");
    TEST_ASSERT(m.y == 3, "monster y must not change");
    return 0;
}

/*
 * Primary axis blocked by wall, fallback to secondary axis.
 * Monster at (1,5), player at (5,5): primary = right (dx=+4, dy=0).
 * Place a wall at (2,5) to block horizontal.
 * Fallback: vertical, but dy=0 → no vertical option → monster stays.
 * Then use player at (5,4) instead: dx=+4, dy=-1, |dx|>|dy| → prefer right.
 * Block (2,5) with TILE_WALL. Fallback: dy=-1 → try (1,4).
 * (1,4) is TILE_FLOOR → monster steps up to (1,4).
 */
static int test_monster_step_fallback_to_secondary_axis(void)
{
    map_t    map;
    monster_t m;
    player_t  p;

    /* Monster at (1,5), player at (5,4) */
    TEST_ASSERT(setup_map_with_entities(&map, &m, 1, 5, &p, 5, 4) == 0,
                "setup must succeed");
    /* Block primary direction (right: x=2,y=5) */
    map_set_tile(&map, 2, 5, TILE_WALL);

    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step with fallback should return 0");
    /* Fallback: vertical, dy=-1 → step up to (1,4) */
    TEST_ASSERT(m.x == 1, "monster x must not change (fallback used vertical)");
    TEST_ASSERT(m.y == 4, "monster y must decrease by 1 (fallback vertical step)");
    return 0;
}

/*
 * Both axes blocked: monster stays in place.
 * Monster at (1,5), player at (5,5): prefer right (dx=+4, dy=0).
 * Block (2,5) with TILE_WALL. dy=0 → no vertical fallback → stay.
 */
static int test_monster_step_stays_when_both_axes_blocked(void)
{
    map_t    map;
    monster_t m;
    player_t  p;

    TEST_ASSERT(setup_map_with_entities(&map, &m, 1, 5, &p, 5, 5) == 0,
                "setup must succeed");
    map_set_tile(&map, 2, 5, TILE_WALL); /* block horizontal */
    /* dy=0, no vertical option */

    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step when fully blocked should return 0");
    TEST_ASSERT(m.x == 1, "monster x must not change when both axes blocked");
    TEST_ASSERT(m.y == 5, "monster y must not change when both axes blocked");
    return 0;
}

/* Monster adjacent to player: attack attempt, monster stays */
static int test_monster_step_attacks_player_stays_put(void)
{
    map_t    map;
    monster_t m;
    player_t  p;
    int before_x;
    int before_y;

    /* Monster at (4,6), player at (4,5): dy=-1 → try (4,5) = TILE_PLAYER */
    TEST_ASSERT(setup_map_with_entities(&map, &m, 4, 6, &p, 4, 5) == 0,
                "setup must succeed");
    before_x = m.x;
    before_y = m.y;

    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step attack attempt should return 0");
    TEST_ASSERT(m.x == before_x,
                "monster x must not change on attack attempt");
    TEST_ASSERT(m.y == before_y,
                "monster y must not change on attack attempt");
    return 0;
}

/* monster_step updates map tiles correctly on a move */
static int test_monster_step_updates_map_tiles(void)
{
    map_t    map;
    monster_t m;
    player_t  p;
    tile_type_t tile;

    /* Monster at (1,5) → moves right to (2,5) */
    TEST_ASSERT(setup_map_with_entities(&map, &m, 1, 5, &p, 5, 5) == 0,
                "setup must succeed");
    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step should return 0");

    /* Old position must be floor */
    TEST_ASSERT(map_get_tile(&map, 1, 5, &tile) == 0, "get old tile must succeed");
    TEST_ASSERT(tile == TILE_FLOOR, "old monster position must become TILE_FLOOR");

    /* New position must be monster */
    TEST_ASSERT(map_get_tile(&map, 2, 5, &tile) == 0, "get new tile must succeed");
    TEST_ASSERT(tile == TILE_MONSTER, "new monster position must be TILE_MONSTER");
    return 0;
}

/* monster_step reduces Manhattan distance by 1 on a clean move */
static int test_monster_step_reduces_distance(void)
{
    map_t    map;
    monster_t m;
    player_t  p;
    int dist_before;
    int dist_after;

    /* Monster at (3,3), player at (7,3): dx=+4, dy=0 → step right */
    TEST_ASSERT(setup_map_with_entities(&map, &m, 3, 3, &p, 7, 3) == 0,
                "setup must succeed");
    dist_before = iabs(p.x - m.x) + iabs(p.y - m.y); /* 4 */

    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step should return 0");
    dist_after = iabs(p.x - m.x) + iabs(p.y - m.y); /* should be 3 */

    TEST_ASSERT(dist_after < dist_before,
                "monster must be closer to player after step");
    return 0;
}

/* NULL argument checks */
static int test_monster_step_null_monster(void)
{
    map_t    map;
    player_t p;
    map_init(&map);
    player_init(&p);
    TEST_ASSERT(monster_step(NULL, &p, &map) == -1,
                "monster_step(NULL, ...) must return -1");
    return 0;
}

/*
 * Monster must treat TILE_COIN as passable (move onto it, overwriting the
 * coin with TILE_MONSTER; spec: monsters can pass through coin cells).
 *
 * Monster at (4,5), player at (4,8). Coin at (4,6) in monster's path.
 * dy=3, dx=0 → vertical primary → tries (4,6)=TILE_COIN → must move.
 */
static int test_monster_step_passes_through_coin(void)
{
    map_t     map;
    monster_t m;
    player_t  p;
    tile_type_t tile;

    map_init(&map);
    player_init(&p);
    p.x = 4; p.y = 8;
    monster_init(&m, 4, 5);
    map_set_tile(&map, 4, 8, TILE_PLAYER);
    map_set_tile(&map, 4, 5, TILE_MONSTER);
    map_set_tile(&map, 4, 6, TILE_COIN); /* coin in monster's direct path */

    TEST_ASSERT(monster_step(&m, &p, &map) == 0,
                "monster_step must return 0 when stepping onto TILE_COIN");
    TEST_ASSERT(m.y == 6,
                "monster must move to TILE_COIN position");
    TEST_ASSERT(map_get_tile(&map, 4, 6, &tile) == 0, "get new tile must succeed");
    TEST_ASSERT(tile == TILE_MONSTER,
                "monster tile must be at new position (TILE_COIN overwritten)");
    TEST_ASSERT(map_get_tile(&map, 4, 5, &tile) == 0, "get old tile must succeed");
    TEST_ASSERT(tile == TILE_FLOOR,
                "old monster position must become TILE_FLOOR");
    return 0;
}

static int test_monster_step_null_player(void)
{
    map_t    map;
    monster_t m;
    map_init(&map);
    monster_init(&m, 3, 5);
    TEST_ASSERT(monster_step(&m, NULL, &map) == -1,
                "monster_step(..., NULL, ...) must return -1");
    return 0;
}

static int test_monster_step_null_map(void)
{
    monster_t m;
    player_t  p;
    monster_init(&m, 3, 5);
    player_init(&p);
    TEST_ASSERT(monster_step(&m, &p, NULL) == -1,
                "monster_step(..., NULL) must return -1");
    return 0;
}

/* ── Test runner ──────────────────────────────────────────────────────── */

typedef struct {
    const char *name;
    int (*fn)(void);
} test_case_t;

int main(void)
{
    int i;
    int failed = 0;

    test_case_t tests[] = {
        { "test_monster_init_hp",                              test_monster_init_hp                              },
        { "test_monster_init_atk",                             test_monster_init_atk                             },
        { "test_monster_init_position",                        test_monster_init_position                        },
        { "test_monster_init_alive",                           test_monster_init_alive                           },
        { "test_monster_init_null",                            test_monster_init_null                            },
        { "test_monster_step_moves_horizontally_toward_player",test_monster_step_moves_horizontally_toward_player},
        { "test_monster_step_moves_vertically_toward_player",  test_monster_step_moves_vertically_toward_player  },
        { "test_monster_step_prefers_horizontal_on_wide_diagonal", test_monster_step_prefers_horizontal_on_wide_diagonal },
        { "test_monster_step_prefers_vertical_on_tall_diagonal",   test_monster_step_prefers_vertical_on_tall_diagonal   },
        { "test_monster_step_ties_broken_by_horizontal",       test_monster_step_ties_broken_by_horizontal       },
        { "test_monster_step_fallback_to_secondary_axis",      test_monster_step_fallback_to_secondary_axis      },
        { "test_monster_step_stays_when_both_axes_blocked",    test_monster_step_stays_when_both_axes_blocked    },
        { "test_monster_step_attacks_player_stays_put",        test_monster_step_attacks_player_stays_put        },
        { "test_monster_step_updates_map_tiles",               test_monster_step_updates_map_tiles               },
        { "test_monster_step_reduces_distance",                test_monster_step_reduces_distance                },
        { "test_monster_step_null_monster",                    test_monster_step_null_monster                    },
        { "test_monster_step_passes_through_coin",             test_monster_step_passes_through_coin             },
        { "test_monster_step_null_player",                     test_monster_step_null_player                     },
        { "test_monster_step_null_map",                        test_monster_step_null_map                        },
    };

    printf("=== Monster Tests ===\n");
    for (i = 0; i < (int)(sizeof(tests) / sizeof(tests[0])); i++) {
        if (tests[i].fn() == 0) {
            printf("  [PASS] %s\n", tests[i].name);
        } else {
            printf("  [FAIL] %s\n", tests[i].name);
            failed++;
        }
    }

    if (failed == 0) {
        printf("All %d tests passed.\n", (int)(sizeof(tests) / sizeof(tests[0])));
        return 0;
    }
    printf("%d test(s) failed.\n", failed);
    return 1;
}
