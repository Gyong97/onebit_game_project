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

/* ── Map dimensions (viewport is always 10 cols × 10 rows) ───────────── */
#define MAP_WIDTH  10
#define MAP_HEIGHT 10   /* visible viewport height; logical map is infinite */

/* ── Message buffer size ──────────────────────────────────────────────── */
#define MSG_BUF_SIZE   128
#define EQUIP_NAME_MAX  16  /* max chars for an equipped item name */

/**
 * @brief All possible tile types that can occupy a map cell.
 */
typedef enum {
    TILE_FLOOR   = 0, /* '.' passable empty space */
    TILE_WALL    = 1, /* '#' impassable border/obstacle */
    TILE_PLAYER  = 2, /* 'P' player entity */
    TILE_MONSTER = 3, /* 'M' monster entity */
    TILE_CHEST   = 4, /* 'C' chest object */
    TILE_COIN    = 5  /* '$' collectible coin */
} tile_type_t;

/**
 * @brief A snapshot of everything the renderer needs to draw one frame.
 *
 * Game logic fills this struct each turn, then passes it to renderer_draw().
 * The renderer must treat this data as read-only.
 */
typedef struct {
    tile_type_t tiles[MAP_HEIGHT][MAP_WIDTH]; /* viewport tile grid (rows[0] = top) */
    int         player_hp;                    /* current HP */
    int         player_max_hp;               /* maximum HP */
    int         player_atk;                  /* attack power */
    int         player_def;                  /* defense power */
    int         player_coins;               /* coins collected */
    int         inventory_count;            /* items in bag */
    /* Equipped item names; empty string "" means slot is empty */
    char        equip_weapon[EQUIP_NAME_MAX];
    char        equip_head[EQUIP_NAME_MAX];
    char        equip_body[EQUIP_NAME_MAX];
    long        scroll_count;                 /* rows scrolled = depth traveled */
    char        message[MSG_BUF_SIZE];        /* event log line (may be empty) */
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
