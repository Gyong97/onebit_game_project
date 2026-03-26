/**
 * @file test_save_manager.c
 * @brief TDD Red: save manager module tests.
 *
 * Tests:
 *  1. save_manager_load on missing file → 0, zeroed data
 *  2. save_manager_load: NULL guard → -1
 *  3. save_manager_save: NULL guard → -1
 *  4. save_manager_save + load round-trip preserves best_depth/total_coins
 *  5. update_on_death: new depth > best → best_depth updated
 *  6. update_on_death: new depth < best → best_depth unchanged
 *  7. update_on_death: total_coins accumulates run coins
 *  8. update_on_death: NULL p_data → -1
 *  9. update_on_death: NULL p_state → -1
 * 10. update_on_death writes through to disk (load reflects change)
 */
#include <stdio.h>
#include <assert.h>
#include "save_manager.h"
#include "turn_manager.h"

/* Clean up save file before/after each test that uses disk */
static void cleanup(void)
{
    remove(SAVE_FILE_PATH);
}

/* ── 1. load on missing file returns 0 with zeroed data ─────────────── */
static void test_load_missing_file(void)
{
    save_data_t data;
    data.best_depth  = 99;
    data.total_coins = 99;

    cleanup();
    assert(save_manager_load(&data) == 0);
    assert(data.best_depth  == 0);
    assert(data.total_coins == 0);
    printf("[PASS] load missing file: returns 0, zeroed data\n");
}

/* ── 2. load NULL guard ──────────────────────────────────────────────── */
static void test_load_null(void)
{
    assert(save_manager_load(NULL) == -1);
    printf("[PASS] load NULL guard: returns -1\n");
}

/* ── 3. save NULL guard ──────────────────────────────────────────────── */
static void test_save_null(void)
{
    assert(save_manager_save(NULL) == -1);
    printf("[PASS] save NULL guard: returns -1\n");
}

/* ── 4. save + load round-trip ───────────────────────────────────────── */
static void test_save_load_roundtrip(void)
{
    save_data_t written;
    save_data_t read;

    cleanup();
    written.best_depth  = 42;
    written.total_coins = 100;

    assert(save_manager_save(&written) == 0);

    read.best_depth  = 0;
    read.total_coins = 0;
    assert(save_manager_load(&read) == 0);
    assert(read.best_depth  == 42);
    assert(read.total_coins == 100);
    cleanup();
    printf("[PASS] save+load round-trip: best_depth=%ld total_coins=%ld\n",
           read.best_depth, read.total_coins);
}

/* ── 5. update_on_death: new depth > best → best_depth updated ───────── */
static void test_update_better_depth(void)
{
    save_data_t   data;
    game_state_t  state;

    cleanup();
    data.best_depth  = 10;
    data.total_coins = 0;

    turn_manager_init(&state);
    state.map.scroll_count = 30;   /* better than 10 */
    state.player.coins     = 0;

    assert(save_manager_update_on_death(&data, &state) == 0);
    assert(data.best_depth == 30);
    cleanup();
    printf("[PASS] update_on_death: better depth 30 > 10 → best updated\n");
}

/* ── 6. update_on_death: new depth < best → best_depth unchanged ──────── */
static void test_update_worse_depth(void)
{
    save_data_t  data;
    game_state_t state;

    cleanup();
    data.best_depth  = 50;
    data.total_coins = 0;

    turn_manager_init(&state);
    state.map.scroll_count = 10;   /* worse than 50 */
    state.player.coins     = 0;

    assert(save_manager_update_on_death(&data, &state) == 0);
    assert(data.best_depth == 50);
    cleanup();
    printf("[PASS] update_on_death: worse depth 10 < 50 → best unchanged\n");
}

/* ── 7. update_on_death: total_coins accumulates ─────────────────────── */
static void test_update_accumulates_coins(void)
{
    save_data_t  data;
    game_state_t state;

    cleanup();
    data.best_depth  = 0;
    data.total_coins = 20;   /* already 20 from previous run */

    turn_manager_init(&state);
    state.map.scroll_count = 0;
    state.player.coins     = 15;  /* earned 15 this run */

    assert(save_manager_update_on_death(&data, &state) == 0);
    assert(data.total_coins == 35);
    cleanup();
    printf("[PASS] update_on_death: total_coins 20+15=35\n");
}

/* ── 8. update_on_death NULL p_data → -1 ─────────────────────────────── */
static void test_update_null_data(void)
{
    game_state_t state;
    turn_manager_init(&state);
    assert(save_manager_update_on_death(NULL, &state) == -1);
    printf("[PASS] update_on_death NULL p_data: returns -1\n");
}

/* ── 9. update_on_death NULL p_state → -1 ────────────────────────────── */
static void test_update_null_state(void)
{
    save_data_t data = {0, 0};
    assert(save_manager_update_on_death(&data, NULL) == -1);
    printf("[PASS] update_on_death NULL p_state: returns -1\n");
}

/* ── 10. update_on_death writes through to disk ──────────────────────── */
static void test_update_persists_to_disk(void)
{
    save_data_t  data;
    save_data_t  loaded;
    game_state_t state;

    cleanup();
    data.best_depth  = 5;
    data.total_coins = 0;

    turn_manager_init(&state);
    state.map.scroll_count = 25;
    state.player.coins     = 8;

    assert(save_manager_update_on_death(&data, &state) == 0);

    loaded.best_depth  = 0;
    loaded.total_coins = 0;
    assert(save_manager_load(&loaded) == 0);
    assert(loaded.best_depth  == 25);
    assert(loaded.total_coins == 8);
    cleanup();
    printf("[PASS] update_on_death persists to disk (depth=%ld coins=%ld)\n",
           loaded.best_depth, loaded.total_coins);
}

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void)
{
    printf("=== test_save_manager ===\n");
    test_load_missing_file();
    test_load_null();
    test_save_null();
    test_save_load_roundtrip();
    test_update_better_depth();
    test_update_worse_depth();
    test_update_accumulates_coins();
    test_update_null_data();
    test_update_null_state();
    test_update_persists_to_disk();
    printf("=== All save_manager tests passed. ===\n");
    return 0;
}
