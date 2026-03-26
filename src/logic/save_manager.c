/**
 * @file save_manager.c
 * @brief Persistent save data implementation for the OneBit roguelike.
 *
 * Serialises save_data_t to/from a binary file using fwrite/fread.
 * On load, a missing file is treated as a clean first run (zeroed data).
 *
 * No stdio output beyond file I/O: pure game-logic module per
 * architecture constraints.
 */
#include <stddef.h>   /* NULL   */
#include <stdio.h>    /* fopen, fread, fwrite, fclose, remove */
#include <string.h>   /* memset */
#include "save_manager.h"

/* ── Public API ───────────────────────────────────────────────────────── */

int save_manager_load(save_data_t *p_data)
{
    FILE *p_file;

    if (p_data == NULL) {
        return -1;
    }

    p_file = fopen(SAVE_FILE_PATH, "rb");
    if (p_file == NULL) {
        /* File does not exist yet — first run, zero the struct */
        memset(p_data, 0, sizeof(save_data_t));
        return 0;
    }

    if (fread(p_data, sizeof(save_data_t), 1, p_file) != 1) {
        fclose(p_file);
        memset(p_data, 0, sizeof(save_data_t));
        return 0; /* Treat corrupt/short file as fresh start */
    }

    fclose(p_file);
    return 0;
}

int save_manager_save(const save_data_t *p_data)
{
    FILE *p_file;

    if (p_data == NULL) {
        return -1;
    }

    p_file = fopen(SAVE_FILE_PATH, "wb");
    if (p_file == NULL) {
        return -1;
    }

    if (fwrite(p_data, sizeof(save_data_t), 1, p_file) != 1) {
        fclose(p_file);
        return -1;
    }

    fclose(p_file);
    return 0;
}

int save_manager_update_on_death(save_data_t *p_data,
                                 const game_state_t *p_state)
{
    if (p_data == NULL || p_state == NULL) {
        return -1;
    }

    /* Update best depth */
    if (p_state->map.scroll_count > p_data->best_depth) {
        p_data->best_depth = p_state->map.scroll_count;
    }

    /* Accumulate coins from this run */
    p_data->total_coins += (long)p_state->player.coins;

    return save_manager_save(p_data);
}
