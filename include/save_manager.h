/**
 * @file save_manager.h
 * @brief Persistent save data interface for the OneBit roguelike.
 *
 * Manages two categories of data (per game spec §5):
 *   Persistent  — best_depth, total_coins (survive across runs)
 *   Volatile    — level, inventory, current depth (reset on death)
 *
 * Data is serialised to / deserialised from SAVE_FILE_PATH using
 * binary fwrite/fread of save_data_t.
 *
 * Return codes:
 *   0   success
 *  -1   error (NULL argument or I/O failure)
 *   1   non-fatal condition (see individual function docs)
 */
#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include "turn_manager.h"  /* game_state_t */

/* ── Save file path ───────────────────────────────────────────────────── */
#define SAVE_FILE_PATH  "save.dat"

/**
 * @brief Persistent metadata stored across game runs.
 */
typedef struct {
    long best_depth;    /* highest scroll_count reached in any run */
    long total_coins;   /* cumulative coins collected across all runs */
} save_data_t;

/* ── Save manager API ─────────────────────────────────────────────────── */

/**
 * @brief Load save data from SAVE_FILE_PATH.
 *
 * If the file does not exist, p_data is zeroed and 0 is returned
 * (first-run initialisation).
 *
 * @param p_data  Output; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int save_manager_load(save_data_t *p_data);

/**
 * @brief Write save data to SAVE_FILE_PATH.
 *
 * @param p_data  Data to persist; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int save_manager_save(const save_data_t *p_data);

/**
 * @brief Update persistent records after a game-over and save to disk.
 *
 * - Updates p_data->best_depth if p_state->map.scroll_count is higher.
 * - Adds p_state->player.coins to p_data->total_coins.
 * - Calls save_manager_save() to persist.
 *
 * @param p_data   Persistent data to update; must not be NULL.
 * @param p_state  Final game state; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int save_manager_update_on_death(save_data_t *p_data,
                                 const game_state_t *p_state);

#endif /* SAVE_MANAGER_H */
