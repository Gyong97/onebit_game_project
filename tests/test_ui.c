/**
 * @file test_ui.c
 * @brief TDD Red: Phase 4-4 UI layer tests.
 *
 * Tests:
 *  1.  monster_t has max_hp field — set to base hp at init
 *  2.  monster max_hp set to scaled hp after turn_manager_spawn_monster_typed
 *  3.  game_state_t has chest_loot_names / chest_loot_ttl fields
 *  4.  turn_manager_open_chest populates chest_loot and sets ttl=CHEST_LOOT_TTL
 *  5.  chest_loot_ttl decrements each player action
 *  6.  chest_loot cleared when ttl reaches 0
 *  7.  game_state_t has levelup_ttl; set after player gains enough XP to level
 *  8.  levelup_ttl decrements each player action
 *  9.  game_state_t has last_attacked_monster_idx — set on attack
 * 10.  render_frame_t has player_level / player_xp / player_xp_to_next fields
 * 11.  render_frame_t has monster_panel entries
 * 12.  render_frame_t has chest_loot / chest_loot_count fields
 * 13.  render_frame_t has in_shop / shop panel fields
 * 14.  frame_builder_build: player_level/xp/xp_to_next correct
 * 15.  frame_builder_build: monster_panel populated for N-adjacent monster
 * 16.  frame_builder_build: monster_panel direction E/S/W correct
 * 17.  frame_builder_build: non-adjacent monster NOT in panel
 * 18.  frame_builder_build: chest_loot copied when ttl > 0
 * 19.  frame_builder_build: chest_loot_count=0 when ttl=0
 * 20.  frame_builder_build: show_levelup set when levelup_ttl > 0
 * 21.  frame_builder_build: shop panel populated when shop active
 * 22.  renderer_draw: new frame fields do not crash
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "monster.h"
#include "turn_manager.h"
#include "renderer.h"
#include "frame_builder.h"
#include "save_manager.h"
#include "item_db.h"

/* ── helpers ──────────────────────────────────────────────────────────── */

static void init_state(game_state_t *s) { turn_manager_init(s); }

/* ── 1. monster_t.max_hp set at init ─────────────────────────────────── */
static void test_monster_max_hp_init(void)
{
    monster_t m;
    monster_init(&m, 2, 5);
    assert(m.max_hp == GOBLIN_INIT_HP);
    printf("[PASS] monster_init: max_hp=%d\n", m.max_hp);
}

/* ── 2. monster max_hp = scaled hp after spawn ───────────────────────── */
static void test_monster_max_hp_scaled(void)
{
    game_state_t state;
    init_state(&state);
    state.map.scroll_count = 20; /* depth=2, scale=1.21 */
    /* clear y=0 to ensure TILE_FLOOR */
    map_set_tile(&state.map, 3, 0, TILE_FLOOR);
    turn_manager_spawn_monster_typed(&state, 3, 0, MONSTER_TYPE_GOBLIN);

    /* find the spawned monster */
    {
        int i;
        for (i = 0; i < MONSTER_MAX_COUNT; i++) {
            if (state.monsters[i].alive && state.monsters[i].x == 3
                && state.monsters[i].y == 0) {
                assert(state.monsters[i].max_hp == state.monsters[i].hp);
                assert(state.monsters[i].max_hp > GOBLIN_INIT_HP);
                printf("[PASS] monster max_hp=%d equals scaled hp\n",
                       state.monsters[i].max_hp);
                return;
            }
        }
    }
    printf("[FAIL] monster not found after spawn\n");
    assert(0);
}

/* ── 3. game_state_t has chest_loot fields ───────────────────────────── */
static void test_game_state_chest_loot_fields(void)
{
    game_state_t state;
    init_state(&state);
    assert(state.chest_loot_ttl == 0);
    assert(state.chest_loot_count == 0);
    printf("[PASS] game_state_t: chest_loot_ttl=0, chest_loot_count=0 after init\n");
}

/* ── 4. open_chest populates chest_loot ──────────────────────────────── */
static void test_open_chest_populates_chest_loot(void)
{
    game_state_t state;
    init_state(&state);
    map_set_tile(&state.map, 3, 15, TILE_CHEST);
    turn_manager_open_chest(&state, 3, 15);

    assert(state.chest_loot_count > 0);
    assert(state.chest_loot_ttl == CHEST_LOOT_TTL);
    assert(state.chest_loot_names[0][0] != '\0');
    printf("[PASS] open_chest: chest_loot_count=%d ttl=%d name='%s'\n",
           state.chest_loot_count, state.chest_loot_ttl,
           state.chest_loot_names[0]);
}

/* ── 5. chest_loot_ttl decrements each action ────────────────────────── */
static void test_chest_loot_ttl_decrements(void)
{
    game_state_t state;
    init_state(&state);

    /* Open chest to set TTL */
    map_set_tile(&state.map, 3, 20, TILE_CHEST);
    map_set_tile(&state.map, state.player.x, state.player.y - 1, TILE_CHEST);
    state.player.y = 20; /* move player near chest */
    map_set_tile(&state.map, state.player.x, state.player.y, TILE_PLAYER);
    map_set_tile(&state.map, state.player.x, state.player.y - 1, TILE_CHEST);

    turn_manager_open_chest(&state, state.player.x, state.player.y - 1);
    assert(state.chest_loot_ttl == CHEST_LOOT_TTL);

    /* Take a step (down is blocked at bottom, use a valid move) */
    /* Instead, manually test the decrement via player act */
    {
        int ttl_before = state.chest_loot_ttl;
        /* Move player sideways */
        map_set_tile(&state.map, state.player.x + 1, state.player.y, TILE_FLOOR);
        turn_manager_player_act(&state, ACTION_MOVE_RIGHT);
        assert(state.chest_loot_ttl == ttl_before - 1);
        printf("[PASS] chest_loot_ttl decrements: %d -> %d\n",
               ttl_before, state.chest_loot_ttl);
    }
}

/* ── 6. chest_loot cleared when ttl=0 ───────────────────────────────── */
static void test_chest_loot_cleared_at_zero(void)
{
    game_state_t state;
    int          i;

    init_state(&state);

    /* Manually set a loot notification */
    state.chest_loot_count = 1;
    state.chest_loot_ttl   = 1;
    strncpy(state.chest_loot_names[0], "Short Sword",
            CHEST_LOOT_NAME_MAX - 1);

    /* One action should decrement to 0 and clear */
    map_set_tile(&state.map, state.player.x + 1, state.player.y, TILE_FLOOR);
    turn_manager_player_act(&state, ACTION_MOVE_RIGHT);

    assert(state.chest_loot_ttl   == 0);
    assert(state.chest_loot_count == 0);
    for (i = 0; i < CHEST_LOOT_MAX; i++) {
        assert(state.chest_loot_names[i][0] == '\0');
    }
    printf("[PASS] chest_loot cleared when ttl reaches 0\n");
}

/* ── 7. game_state_t has levelup_ttl; set after level-up ────────────── */
static void test_levelup_ttl_set(void)
{
    game_state_t state;
    init_state(&state);

    assert(state.levelup_ttl == 0);

    /* Give enough XP to level up: level*LEVELUP_XP_FACTOR = 1*50 */
    state.player.xp = LEVELUP_XP_FACTOR - 1; /* 49 — one short */
    /*
     * Use BAT (10 HP): player ATK=10 kills it in one hit,
     * granting XP_BASE_REWARD (10) → total xp 59 > 50 → level up.
     */
    {
        int mx = state.player.x;
        int my = state.player.y - 1;
        map_set_tile(&state.map, mx, my, TILE_FLOOR);
        turn_manager_spawn_monster_typed(&state, mx, my, MONSTER_TYPE_BAT);
        turn_manager_player_act(&state, ACTION_MOVE_UP);
        assert(state.levelup_ttl > 0);
        printf("[PASS] levelup_ttl=%d after level-up\n", state.levelup_ttl);
    }
}

/* ── 8. levelup_ttl decrements ───────────────────────────────────────── */
static void test_levelup_ttl_decrements(void)
{
    game_state_t state;
    init_state(&state);

    /* Manually set levelup_ttl */
    state.levelup_ttl = 2;
    map_set_tile(&state.map, state.player.x + 1, state.player.y, TILE_FLOOR);
    turn_manager_player_act(&state, ACTION_MOVE_RIGHT);
    assert(state.levelup_ttl == 1);
    printf("[PASS] levelup_ttl decrements: 2 -> 1\n");
}

/* ── 9. last_attacked_monster_idx set on attack ──────────────────────── */
static void test_last_attacked_idx_set(void)
{
    game_state_t state;
    int          mx;
    int          my;

    init_state(&state);
    assert(state.last_attacked_monster_idx == -1);

    mx = state.player.x;
    my = state.player.y - 1;
    map_set_tile(&state.map, mx, my, TILE_FLOOR);
    turn_manager_spawn_monster_typed(&state, mx, my, MONSTER_TYPE_GOBLIN);

    turn_manager_player_act(&state, ACTION_MOVE_UP);
    /* After attacking, last_attacked_monster_idx should be ≥ 0 */
    assert(state.last_attacked_monster_idx >= 0);
    printf("[PASS] last_attacked_monster_idx=%d after attack\n",
           state.last_attacked_monster_idx);
}

/* ── 10. render_frame_t has player_level/xp/xp_to_next ──────────────── */
static void test_frame_player_fields_exist(void)
{
    render_frame_t f;
    memset(&f, 0, sizeof(f));
    f.player_level    = 3;
    f.player_xp       = 25;
    f.player_xp_to_next = 150;
    f.show_levelup    = 1;
    assert(f.player_level == 3);
    assert(f.player_xp == 25);
    assert(f.player_xp_to_next == 150);
    assert(f.show_levelup == 1);
    printf("[PASS] render_frame_t: player_level/xp/xp_to_next fields exist\n");
}

/* ── 11. render_frame_t has monster_panel ─────────────────────────────── */
static void test_frame_monster_panel_exists(void)
{
    render_frame_t f;
    memset(&f, 0, sizeof(f));
    f.monster_panel[0].hp     = 10;
    f.monster_panel[0].max_hp = 20;
    f.monster_panel[0].active = 1;
    strncpy(f.monster_panel[0].dir, "N", 3);
    assert(f.monster_panel[0].active == 1);
    assert(f.monster_panel[0].hp == 10);
    printf("[PASS] render_frame_t: monster_panel[%d] fields exist\n",
           UI_MONSTER_PANEL_MAX);
}

/* ── 12. render_frame_t has chest_loot fields ────────────────────────── */
static void test_frame_chest_loot_exists(void)
{
    render_frame_t f;
    memset(&f, 0, sizeof(f));
    strncpy(f.chest_loot[0], "Elixir", UI_NAME_MAX - 1);
    f.chest_loot_count = 1;
    assert(f.chest_loot_count == 1);
    assert(f.chest_loot[0][0] == 'E');
    printf("[PASS] render_frame_t: chest_loot[%d] fields exist\n",
           UI_CHEST_PANEL_MAX);
}

/* ── 13. render_frame_t has in_shop / shop panel ─────────────────────── */
static void test_frame_shop_fields_exist(void)
{
    render_frame_t f;
    memset(&f, 0, sizeof(f));
    f.in_shop          = 1;
    f.shop_page        = 0;
    f.shop_buy_count   = 2;
    f.shop_buy_cursor  = 0;
    strncpy(f.shop_buy_list[0].name, "Short Sword", UI_NAME_MAX - 1);
    f.shop_buy_list[0].price = 8;
    assert(f.in_shop == 1);
    assert(f.shop_buy_list[0].price == 8);
    printf("[PASS] render_frame_t: shop panel fields exist\n");
}

/* ── 14. frame_builder_build: player_level/xp correct ───────────────── */
static void test_frame_builder_player_level_xp(void)
{
    game_state_t   state;
    save_data_t    save;
    render_frame_t frame;

    init_state(&state);
    memset(&save, 0, sizeof(save));
    memset(&frame, 0, sizeof(frame));

    state.player.level = 3;
    state.player.xp    = 27;

    frame_builder_build(&frame, &state, &save);

    assert(frame.player_level    == 3);
    assert(frame.player_xp       == 27);
    assert(frame.player_xp_to_next == 3 * LEVELUP_XP_FACTOR);
    printf("[PASS] frame_builder: player_level=%d xp=%d xp_to_next=%d\n",
           frame.player_level, frame.player_xp, frame.player_xp_to_next);
}

/* ── 15. frame_builder_build: monster N of player → dir="N" ──────────── */
static void test_frame_builder_monster_panel_north(void)
{
    game_state_t   state;
    save_data_t    save;
    render_frame_t frame;
    int            mx;
    int            my;
    int            i;
    int            found;

    init_state(&state);
    memset(&save, 0, sizeof(save));
    memset(&frame, 0, sizeof(frame));

    /* Place a monster directly above the player */
    mx = state.player.x;
    my = state.player.y - 1;
    map_set_tile(&state.map, mx, my, TILE_FLOOR);
    turn_manager_spawn_monster_typed(&state, mx, my, MONSTER_TYPE_GOBLIN);

    frame_builder_build(&frame, &state, &save);

    found = 0;
    for (i = 0; i < UI_MONSTER_PANEL_MAX; i++) {
        if (frame.monster_panel[i].active) {
            found = 1;
            assert(strcmp(frame.monster_panel[i].dir, "N") == 0);
            assert(frame.monster_panel[i].hp > 0);
            assert(frame.monster_panel[i].max_hp >= frame.monster_panel[i].hp);
            printf("[PASS] frame_builder: adjacent monster dir='%s' hp=%d/%d\n",
                   frame.monster_panel[i].dir,
                   frame.monster_panel[i].hp,
                   frame.monster_panel[i].max_hp);
            break;
        }
    }
    assert(found);
}

/* ── 16. frame_builder: S/E/W directions ─────────────────────────────── */
static void test_frame_builder_monster_panel_directions(void)
{
    game_state_t   state;
    save_data_t    save;
    render_frame_t frame;
    int            i;

    /* ── SOUTH ── */
    init_state(&state);
    memset(&save, 0, sizeof(save));
    memset(&frame, 0, sizeof(frame));
    map_set_tile(&state.map, state.player.x, state.player.y + 1, TILE_FLOOR);
    turn_manager_spawn_monster_typed(&state, state.player.x,
                                     state.player.y + 1,
                                     MONSTER_TYPE_GOBLIN);
    frame_builder_build(&frame, &state, &save);
    for (i = 0; i < UI_MONSTER_PANEL_MAX; i++) {
        if (frame.monster_panel[i].active) {
            assert(strcmp(frame.monster_panel[i].dir, "S") == 0);
            printf("[PASS] frame_builder: monster S dir correct\n");
            break;
        }
    }

    /* ── EAST ── */
    init_state(&state);
    memset(&frame, 0, sizeof(frame));
    map_set_tile(&state.map, state.player.x + 1, state.player.y, TILE_FLOOR);
    turn_manager_spawn_monster_typed(&state, state.player.x + 1,
                                     state.player.y,
                                     MONSTER_TYPE_GOBLIN);
    frame_builder_build(&frame, &state, &save);
    for (i = 0; i < UI_MONSTER_PANEL_MAX; i++) {
        if (frame.monster_panel[i].active) {
            assert(strcmp(frame.monster_panel[i].dir, "E") == 0);
            printf("[PASS] frame_builder: monster E dir correct\n");
            break;
        }
    }

    /* ── WEST ── */
    init_state(&state);
    memset(&frame, 0, sizeof(frame));
    map_set_tile(&state.map, state.player.x - 1, state.player.y, TILE_FLOOR);
    turn_manager_spawn_monster_typed(&state, state.player.x - 1,
                                     state.player.y,
                                     MONSTER_TYPE_GOBLIN);
    frame_builder_build(&frame, &state, &save);
    for (i = 0; i < UI_MONSTER_PANEL_MAX; i++) {
        if (frame.monster_panel[i].active) {
            assert(strcmp(frame.monster_panel[i].dir, "W") == 0);
            printf("[PASS] frame_builder: monster W dir correct\n");
            break;
        }
    }
}

/* ── 17. frame_builder: non-adjacent monster not in panel ────────────── */
static void test_frame_builder_nonadjacent_not_in_panel(void)
{
    game_state_t   state;
    save_data_t    save;
    render_frame_t frame;
    int            i;

    init_state(&state);
    memset(&save, 0, sizeof(save));
    memset(&frame, 0, sizeof(frame));

    /* Place monster 2 cells away */
    map_set_tile(&state.map, state.player.x, state.player.y - 2, TILE_FLOOR);
    turn_manager_spawn_monster_typed(&state, state.player.x,
                                     state.player.y - 2,
                                     MONSTER_TYPE_GOBLIN);
    frame_builder_build(&frame, &state, &save);

    for (i = 0; i < UI_MONSTER_PANEL_MAX; i++) {
        if (frame.monster_panel[i].active) {
            /* dir should be "" (no direction shown for non-adjacent) */
            assert(frame.monster_panel[i].dir[0] == '\0');
        }
    }
    printf("[PASS] frame_builder: non-adjacent monster has no direction\n");
}

/* ── 18. frame_builder: chest_loot copied when ttl > 0 ──────────────── */
static void test_frame_builder_chest_loot_shown(void)
{
    game_state_t   state;
    save_data_t    save;
    render_frame_t frame;

    init_state(&state);
    memset(&save, 0, sizeof(save));
    memset(&frame, 0, sizeof(frame));

    state.chest_loot_ttl   = 2;
    state.chest_loot_count = 1;
    strncpy(state.chest_loot_names[0], "Elixir",
            CHEST_LOOT_NAME_MAX - 1);

    frame_builder_build(&frame, &state, &save);

    assert(frame.chest_loot_count == 1);
    assert(strcmp(frame.chest_loot[0], "Elixir") == 0);
    printf("[PASS] frame_builder: chest_loot copied from state\n");
}

/* ── 19. frame_builder: chest_loot_count=0 when ttl=0 ───────────────── */
static void test_frame_builder_chest_loot_hidden(void)
{
    game_state_t   state;
    save_data_t    save;
    render_frame_t frame;

    init_state(&state);
    memset(&save, 0, sizeof(save));
    memset(&frame, 0, sizeof(frame));

    state.chest_loot_ttl   = 0;  /* not showing */
    state.chest_loot_count = 1;
    strncpy(state.chest_loot_names[0], "Elixir",
            CHEST_LOOT_NAME_MAX - 1);

    frame_builder_build(&frame, &state, &save);

    assert(frame.chest_loot_count == 0);
    printf("[PASS] frame_builder: chest_loot hidden when ttl=0\n");
}

/* ── 20. frame_builder: show_levelup when levelup_ttl > 0 ───────────── */
static void test_frame_builder_show_levelup(void)
{
    game_state_t   state;
    save_data_t    save;
    render_frame_t frame;

    init_state(&state);
    memset(&save, 0, sizeof(save));
    memset(&frame, 0, sizeof(frame));

    state.levelup_ttl = 2;
    frame_builder_build(&frame, &state, &save);
    assert(frame.show_levelup == 1);

    state.levelup_ttl = 0;
    memset(&frame, 0, sizeof(frame));
    frame_builder_build(&frame, &state, &save);
    assert(frame.show_levelup == 0);
    printf("[PASS] frame_builder: show_levelup tied to levelup_ttl\n");
}

/* ── 21. frame_builder: shop panel when shop active ──────────────────── */
static void test_frame_builder_shop_panel(void)
{
    game_state_t   state;
    save_data_t    save;
    render_frame_t frame;

    init_state(&state);
    memset(&save, 0, sizeof(save));
    memset(&frame, 0, sizeof(frame));

    shop_open(&state.shop, 3, 5);
    state.shop.page       = SHOP_PAGE_BUY;
    state.shop.buy_cursor = 1;

    frame_builder_build(&frame, &state, &save);

    assert(frame.in_shop == 1);
    assert(frame.shop_page == SHOP_PAGE_BUY);
    assert(frame.shop_buy_cursor == 1);
    assert(frame.shop_buy_count == ITEM_DB_COUNT);
    assert(frame.shop_buy_list[0].price > 0);
    printf("[PASS] frame_builder: shop panel populated (%d buy items)\n",
           frame.shop_buy_count);
}

/* ── 22. renderer_draw: new frame fields no crash ────────────────────── */
static void test_renderer_draw_new_fields(void)
{
    render_frame_t f;
    int            r;
    int            c;

    memset(&f, 0, sizeof(f));
    for (r = 0; r < MAP_HEIGHT; r++)
        for (c = 0; c < MAP_WIDTH; c++)
            f.tiles[r][c] = TILE_FLOOR;

    f.player_level    = 2;
    f.player_xp       = 30;
    f.player_xp_to_next = 100;
    f.show_levelup    = 1;
    f.monster_panel[0].active = 1;
    f.monster_panel[0].hp     = 15;
    f.monster_panel[0].max_hp = 20;
    strncpy(f.monster_panel[0].name, "Goblin", UI_NAME_MAX - 1);
    strncpy(f.monster_panel[0].dir, "N", 3);
    f.chest_loot_count = 1;
    strncpy(f.chest_loot[0], "Elixir", UI_NAME_MAX - 1);

    assert(renderer_init() == 0);
    assert(renderer_draw(&f) == 0);
    renderer_destroy();
    printf("[PASS] renderer_draw: no crash with new UI fields\n");
}

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void)
{
    printf("=== test_ui ===\n");
    test_monster_max_hp_init();
    test_monster_max_hp_scaled();
    test_game_state_chest_loot_fields();
    test_open_chest_populates_chest_loot();
    test_chest_loot_ttl_decrements();
    test_chest_loot_cleared_at_zero();
    test_levelup_ttl_set();
    test_levelup_ttl_decrements();
    test_last_attacked_idx_set();
    test_frame_player_fields_exist();
    test_frame_monster_panel_exists();
    test_frame_chest_loot_exists();
    test_frame_shop_fields_exist();
    test_frame_builder_player_level_xp();
    test_frame_builder_monster_panel_north();
    test_frame_builder_monster_panel_directions();
    test_frame_builder_nonadjacent_not_in_panel();
    test_frame_builder_chest_loot_shown();
    test_frame_builder_chest_loot_hidden();
    test_frame_builder_show_levelup();
    test_frame_builder_shop_panel();
    test_renderer_draw_new_fields();
    printf("=== All UI tests passed. ===\n");
    return 0;
}
