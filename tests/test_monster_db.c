/**
 * @file test_monster_db.c
 * @brief TDD Red: monster database tests.
 *
 * Tests:
 *  1. monster_db_count() == MONSTER_TYPE_COUNT (3)
 *  2. monster_db_get(GOBLIN): hp/atk match GOBLIN_INIT_*
 *  3. monster_db_get(SLIME):  hp/atk match SLIME_INIT_*
 *  4. monster_db_get(BAT):    hp/atk match BAT_INIT_*
 *  5. All monster names are non-empty
 *  6. Out-of-range type returns -1
 *  7. NULL p_out returns -1
 *  8. monster_init_typed uses DB values (GOBLIN stats match DB)
 */
#include <stdio.h>
#include <string.h>
#include "monster_db.h"
#include "turn_manager.h"  /* game_state_t, turn_manager_init */

#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

/* ── 1. count ─────────────────────────────────────────────────────────── */
static int test_monster_db_count(void)
{
    TEST_ASSERT(monster_db_count() == MONSTER_TYPE_COUNT,
                "monster_db_count must equal MONSTER_TYPE_COUNT");
    return 0;
}

/* ── 2. Goblin stats ──────────────────────────────────────────────────── */
static int test_monster_db_get_goblin_stats(void)
{
    monster_def_t def;
    TEST_ASSERT(monster_db_get(MONSTER_TYPE_GOBLIN, &def) == 0,
                "monster_db_get GOBLIN should return 0");
    TEST_ASSERT(def.base_hp  == GOBLIN_INIT_HP,
                "Goblin base_hp must equal GOBLIN_INIT_HP");
    TEST_ASSERT(def.base_atk == GOBLIN_INIT_ATK,
                "Goblin base_atk must equal GOBLIN_INIT_ATK");
    TEST_ASSERT(def.type == MONSTER_TYPE_GOBLIN,
                "Goblin def.type must be MONSTER_TYPE_GOBLIN");
    return 0;
}

/* ── 3. Slime stats ───────────────────────────────────────────────────── */
static int test_monster_db_get_slime_stats(void)
{
    monster_def_t def;
    TEST_ASSERT(monster_db_get(MONSTER_TYPE_SLIME, &def) == 0,
                "monster_db_get SLIME should return 0");
    TEST_ASSERT(def.base_hp  == SLIME_INIT_HP,
                "Slime base_hp must equal SLIME_INIT_HP");
    TEST_ASSERT(def.base_atk == SLIME_INIT_ATK,
                "Slime base_atk must equal SLIME_INIT_ATK");
    return 0;
}

/* ── 4. Bat stats ─────────────────────────────────────────────────────── */
static int test_monster_db_get_bat_stats(void)
{
    monster_def_t def;
    TEST_ASSERT(monster_db_get(MONSTER_TYPE_BAT, &def) == 0,
                "monster_db_get BAT should return 0");
    TEST_ASSERT(def.base_hp  == BAT_INIT_HP,
                "Bat base_hp must equal BAT_INIT_HP");
    TEST_ASSERT(def.base_atk == BAT_INIT_ATK,
                "Bat base_atk must equal BAT_INIT_ATK");
    return 0;
}

/* ── 5. Non-empty names ───────────────────────────────────────────────── */
static int test_monster_db_names_nonempty(void)
{
    monster_def_t def;
    int           t;
    for (t = 0; t < MONSTER_TYPE_COUNT; t++) {
        TEST_ASSERT(monster_db_get((monster_type_t)t, &def) == 0,
                    "monster_db_get should succeed for each valid type");
        TEST_ASSERT(def.name[0] != '\0',
                    "monster name must not be empty");
    }
    return 0;
}

/* ── 6. Out-of-range type ─────────────────────────────────────────────── */
static int test_monster_db_get_oob(void)
{
    monster_def_t def;
    TEST_ASSERT(monster_db_get(MONSTER_TYPE_COUNT, &def) == -1,
                "monster_db_get type=MONSTER_TYPE_COUNT must return -1");
    TEST_ASSERT(monster_db_get((monster_type_t)-1, &def) == -1,
                "monster_db_get type=-1 must return -1");
    return 0;
}

/* ── 7. NULL p_out ────────────────────────────────────────────────────── */
static int test_monster_db_get_null(void)
{
    TEST_ASSERT(monster_db_get(MONSTER_TYPE_GOBLIN, NULL) == -1,
                "monster_db_get(NULL p_out) must return -1");
    return 0;
}

/* ── 8. monster_init_typed reflects DB values ─────────────────────────── */
static int test_monster_init_typed_uses_db(void)
{
    game_state_t  state;
    monster_def_t def;

    turn_manager_init(&state);
    monster_db_get(MONSTER_TYPE_GOBLIN, &def);
    turn_manager_spawn_monster_typed(&state, PLAYER_INIT_X, PLAYER_INIT_Y - 2,
                                     MONSTER_TYPE_GOBLIN);

    /* Base stats (no depth scaling at scroll=0) must match DB */
    TEST_ASSERT(state.monsters[0].hp  == def.base_hp,
                "spawned goblin hp must match monster_db base_hp");
    TEST_ASSERT(state.monsters[0].atk == def.base_atk,
                "spawned goblin atk must match monster_db base_atk");
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
        { "test_monster_db_count",              test_monster_db_count              },
        { "test_monster_db_get_goblin_stats",   test_monster_db_get_goblin_stats   },
        { "test_monster_db_get_slime_stats",    test_monster_db_get_slime_stats    },
        { "test_monster_db_get_bat_stats",      test_monster_db_get_bat_stats      },
        { "test_monster_db_names_nonempty",     test_monster_db_names_nonempty     },
        { "test_monster_db_get_oob",            test_monster_db_get_oob            },
        { "test_monster_db_get_null",           test_monster_db_get_null           },
        { "test_monster_init_typed_uses_db",    test_monster_init_typed_uses_db    },
    };

    printf("=== Monster DB Tests ===\n");
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
