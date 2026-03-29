/**
 * @file renderer.h
 * @brief Abstract renderer interface for the OneBit roguelike.
 *
 * This header defines the data structures and function signatures for
 * rendering the game state. Implementations must not contain any game
 * logic — they only draw what is described in render_frame_t.
 */
#ifndef RENDERER_H
#define RENDERER_H

/* ── Map dimensions ──────────────────────────────────────────────────── */
#define MAP_WIDTH    13   /* columns (border walls at x=0 and x=MAP_WIDTH-1) */
#define MAP_HEIGHT   16   /* visible viewport rows */
#define MAP_BUFFER_H  9   /* invisible pre-load buffer rows above visible area */
#define MAP_TOTAL_H  (MAP_HEIGHT + MAP_BUFFER_H) /* total rows in map_t */

/* ── Message buffer size ──────────────────────────────────────────────── */
#define MSG_BUF_SIZE   128
#define EQUIP_NAME_MAX  16  /* max chars for an equipped item name */

/* ── UI panel constants ───────────────────────────────────────────────── */
#define UI_NAME_MAX          16  /* max chars for a name in any UI panel      */
#define UI_MONSTER_PANEL_MAX  4  /* max monster entries in the info panel     */
#define UI_CHEST_PANEL_MAX    3  /* max chest loot lines shown                */
#define UI_SHOP_BUY_MAX      16  /* max buy-page entries (future item growth) */
#define UI_SELL_MAX          10  /* max sell-page entries (= INVENTORY_MAX)   */

/* ── UI panel entry types ─────────────────────────────────────────────── */

/**
 * @brief One monster displayed in the monster info panel.
 *
 * Monsters adjacent to the player carry a dir string ("N","S","W","E").
 * Non-adjacent tracked monsters (last attacked) have dir = "".
 */
typedef struct {
    char name[UI_NAME_MAX];  /* monster type name                    */
    int  hp;                 /* current HP                           */
    int  max_hp;             /* maximum HP (set at spawn)            */
    char dir[4];             /* "N","S","W","E" or "" (not adjacent) */
    int  active;             /* 1 = this slot holds a valid entry    */
} ui_monster_entry_t;

/** @brief One item entry on the shop buy page. */
typedef struct {
    char name[UI_NAME_MAX];
    int  price;
} ui_buy_entry_t;

/** @brief One item entry on the shop sell page. */
typedef struct {
    char name[UI_NAME_MAX];
    int  sell_price;
} ui_sell_entry_t;

/**
 * @brief All possible tile types that can occupy a map cell.
 */
typedef enum {
    TILE_FLOOR   = 0, /* '.' passable empty space */
    TILE_WALL    = 1, /* '#' impassable border/obstacle */
    TILE_PLAYER  = 2, /* 'P' player entity */
    TILE_MONSTER = 3, /* 'M' monster entity */
    TILE_CHEST   = 4, /* 'C' chest object */
    TILE_COIN    = 5, /* '$' collectible coin */
    TILE_SHOP       = 6, /* 'S' shop tile — buy items with coins */
    TILE_CHEST_OPEN = 7, /* 'c' opened chest — impassable, no interaction */
    TILE_SHOP_OPEN  = 8  /* 's' visited shop — impassable, no interaction */
} tile_type_t;

/**
 * @brief A snapshot of everything the renderer needs to draw one frame.
 *
 * Game logic fills this struct each turn, then passes it to renderer_draw().
 * The renderer must treat this data as read-only.
 */
typedef struct {
    /* ── Map grid ──────────────────────────────────────────────────── */
    tile_type_t tiles[MAP_HEIGHT][MAP_WIDTH]; /* viewport (rows[0] = top) */

    /* ── Player HUD ────────────────────────────────────────────────── */
    int  player_hp;
    int  player_max_hp;
    int  player_atk;
    int  player_def;
    int  player_coins;
    int  inventory_count;
    char equip_weapon[EQUIP_NAME_MAX];
    char equip_head[EQUIP_NAME_MAX];
    char equip_body[EQUIP_NAME_MAX];
    long scroll_count;
    long best_depth;

    /* Player level / XP */
    int  player_level;       /* current level */
    int  player_xp;          /* current XP    */
    int  player_xp_to_next;  /* XP needed for next level */
    int  show_levelup;       /* 1 = display LEVEL UP! notification */

    /* ── Monster info panel ─────────────────────────────────────────── */
    ui_monster_entry_t monster_panel[UI_MONSTER_PANEL_MAX];

    /* ── Chest loot panel ───────────────────────────────────────────── */
    char chest_loot[UI_CHEST_PANEL_MAX][UI_NAME_MAX];
    int  chest_loot_count;   /* 0 = hide panel */

    /* ── Shop panel ─────────────────────────────────────────────────── */
    int             in_shop;          /* 1 = shop UI is open           */
    int             shop_page;        /* SHOP_PAGE_BUY or SELL         */
    int             shop_buy_cursor;
    int             shop_sell_cursor;
    ui_buy_entry_t  shop_buy_list[UI_SHOP_BUY_MAX];
    int             shop_buy_count;
    ui_sell_entry_t shop_sell_list[UI_SELL_MAX];
    int             shop_sell_count;

    /* ── Event message ──────────────────────────────────────────────── */
    char message[MSG_BUF_SIZE];
} render_frame_t;

/* ── Renderer API ─────────────────────────────────────────────────────── */

/**
 * @brief Initialise the renderer backend.
 * @return 0 on success, negative value on failure.
 */
int renderer_init(void);

/**
 * @brief Draw a complete frame from the given snapshot.
 * @param p_frame  Pointer to the frame data; must not be NULL.
 * @return 0 on success, negative value on failure.
 */
int renderer_draw(const render_frame_t *p_frame);

/**
 * @brief Clear the terminal/display area.
 * @return 0 on success, negative value on failure.
 */
int renderer_clear(void);

/**
 * @brief Release any resources held by the renderer.
 */
void renderer_destroy(void);

#endif /* RENDERER_H */
