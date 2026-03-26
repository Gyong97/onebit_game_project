/**
 * @file item_db.h
 * @brief Item database (dictionary) for the OneBit roguelike.
 *
 * Provides a static, ID-indexed table of all item templates.
 * Callers use item_db_get() to look up an item by its integer ID.
 *
 * item_db_get() return codes:
 *   0  — success; *p_out is populated
 *  -1  — error (id out of range or p_out is NULL)
 */
#ifndef ITEM_DB_H
#define ITEM_DB_H

#include "item.h"   /* item_t, item_type_t */

/* ── Database constants ───────────────────────────────────────────────── */
#define ITEM_DB_COUNT  8  /* total number of item entries in the table */

/* ── Item DB API ──────────────────────────────────────────────────────── */

/**
 * @brief Return the number of items in the database.
 * @return ITEM_DB_COUNT (always).
 */
int item_db_count(void);

/**
 * @brief Look up one item by its integer ID.
 *
 * @param id    Item index [0, ITEM_DB_COUNT).
 * @param p_out Output item; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int item_db_get(int id, item_t *p_out);

#endif /* ITEM_DB_H */
