/**
 * @file test_playable_db.c
 * @brief TDD Red: playable class database tests.
 *
 * Tests:
 *  1. playable_db_count() == PLAYABLE_TYPE_COUNT (1)
 *  2. playable_db_get(WARRIOR): stats match WARRIOR_INIT_*
 *  3. Warrior name is non-empty
 *  4. Out-of-range type returns -1
 *  5. NULL p_out returns -1
 *  6. player_init_typed(WARRIOR) produces same stats as player_init()
 */
#include <stdio.h>
#include "playable_db.h"
#include "player.h"   /* player_t, player_init, player_init_typed, PLAYER_INIT_* */

#define TEST_ASSERT(cond, msg)                                         \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); \
            return -1;                                                 \
        }                                                              \
    } while (0)

/* ── 1. count ─────────────────────────────────────────────────────────── */
static int test_playable_db_count(void)
{
    TEST_ASSERT(playable_db_count() == PLAYABLE_TYPE_COUNT,
                "playable_db_count must equal PLAYABLE_TYPE_COUNT");
    return 0;
}

/* ── 2. Warrior stats ─────────────────────────────────────────────────── */
static int test_playable_db_get_warrior_stats(void)
{
    playable_def_t def;
    TEST_ASSERT(playable_db_get(PLAYABLE_WARRIOR, &def) == 0,
                "playable_db_get WARRIOR should return 0");
    TEST_ASSERT(def.init_hp     == WARRIOR_INIT_HP,
                "Warrior init_hp must equal WARRIOR_INIT_HP");
    TEST_ASSERT(def.init_max_hp == WARRIOR_INIT_MAXHP,
                "Warrior init_max_hp must equal WARRIOR_INIT_MAXHP");
    TEST_ASSERT(def.init_atk    == WARRIOR_INIT_ATK,
                "Warrior init_atk must equal WARRIOR_INIT_ATK");
    TEST_ASSERT(def.init_def    == WARRIOR_INIT_DEF,
                "Warrior init_def must equal WARRIOR_INIT_DEF");
    TEST_ASSERT(def.type        == PLAYABLE_WARRIOR,
                "Warrior def.type must be PLAYABLE_WARRIOR");
    return 0;
}

/* ── 3. Warrior name non-empty ────────────────────────────────────────── */
static int test_playable_db_warrior_name_nonempty(void)
{
    playable_def_t def;
    TEST_ASSERT(playable_db_get(PLAYABLE_WARRIOR, &def) == 0,
                "playable_db_get WARRIOR should return 0");
    TEST_ASSERT(def.name[0] != '\0',
                "Warrior name must not be empty");
    return 0;
}

/* ── 4. Out-of-range type ─────────────────────────────────────────────── */
static int test_playable_db_get_oob(void)
{
    playable_def_t def;
    TEST_ASSERT(playable_db_get(PLAYABLE_TYPE_COUNT, &def) == -1,
                "playable_db_get type=PLAYABLE_TYPE_COUNT must return -1");
    TEST_ASSERT(playable_db_get((playable_type_t)-1, &def) == -1,
                "playable_db_get type=-1 must return -1");
    return 0;
}

/* ── 5. NULL p_out ────────────────────────────────────────────────────── */
static int test_playable_db_get_null(void)
{
    TEST_ASSERT(playable_db_get(PLAYABLE_WARRIOR, NULL) == -1,
                "playable_db_get(NULL p_out) must return -1");
    return 0;
}

/* ── 6. player_init_typed(WARRIOR) matches player_init() ─────────────── */
static int test_player_init_typed_warrior(void)
{
    player_t p1;
    player_t p2;

    TEST_ASSERT(player_init(&p1) == 0,
                "player_init should return 0");
    TEST_ASSERT(player_init_typed(&p2, PLAYABLE_WARRIOR) == 0,
                "player_init_typed(WARRIOR) should return 0");

    TEST_ASSERT(p2.hp     == p1.hp,     "hp must match player_init");
    TEST_ASSERT(p2.max_hp == p1.max_hp, "max_hp must match player_init");
    TEST_ASSERT(p2.atk    == p1.atk,    "atk must match player_init");
    TEST_ASSERT(p2.def    == p1.def,    "def must match player_init");
    TEST_ASSERT(p2.x      == p1.x,      "x must match player_init");
    TEST_ASSERT(p2.y      == p1.y,      "y must match player_init");
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
        { "test_playable_db_count",                 test_playable_db_count                 },
        { "test_playable_db_get_warrior_stats",     test_playable_db_get_warrior_stats     },
        { "test_playable_db_warrior_name_nonempty", test_playable_db_warrior_name_nonempty },
        { "test_playable_db_get_oob",               test_playable_db_get_oob               },
        { "test_playable_db_get_null",              test_playable_db_get_null              },
        { "test_player_init_typed_warrior",         test_player_init_typed_warrior         },
    };

    printf("=== Playable DB Tests ===\n");
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
