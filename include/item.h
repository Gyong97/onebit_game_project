/**
 * @file item.h
 * @brief Item and inventory definitions for the OneBit roguelike.
 *
 * Items are generated when chests are opened and stored in the player's
 * inventory. Phase 3 will implement equipping items and applying stat
 * changes; Phase 2 only handles acquisition and storage.
 */
#ifndef ITEM_H
#define ITEM_H

/* ── Inventory constants ──────────────────────────────────────────────── */
#define INVENTORY_MAX    10  /* maximum items a player can carry */
#define ITEM_NAME_MAX    16  /* max characters in an item name (incl. '\0') */

/* ── Item rewards (used when generating chest drops) ─────────────────── */
#define ITEM_WEAPON_ATK_BONUS   5   /* ATK added when a weapon is equipped  */
#define ITEM_ARMOR_DEF_BONUS    3   /* DEF added when body armor is equipped */
#define ITEM_HELMET_DEF_BONUS   2   /* DEF added when helmet is equipped    */
#define ITEM_POTION_HP_RESTORE  20  /* HP restored when a potion is used    */

/**
 * @brief Item category.
 *
 * ITEM_NONE marks an empty inventory slot (all-zeros initialisation).
 */
typedef enum {
    ITEM_NONE   = 0,  /* empty / unset                   */
    ITEM_WEAPON = 1,  /* weapon  — equip → Atk up        */
    ITEM_ARMOR  = 2,  /* armor   — equip (body) → Def up */
    ITEM_POTION = 3,  /* potion  — use  → HP up          */
    ITEM_HELMET = 4   /* helmet  — equip (head) → Def up */
} item_type_t;

/**
 * @brief A single item instance.
 *
 * Only the field relevant to the item's type carries a non-zero value.
 * All other bonus fields are 0.
 */
typedef struct {
    item_type_t type;
    int         atk_bonus;            /* > 0 for ITEM_WEAPON */
    int         def_bonus;            /* > 0 for ITEM_ARMOR  */
    int         hp_restore;           /* > 0 for ITEM_POTION */
    char        name[ITEM_NAME_MAX];  /* human-readable label */
} item_t;

#endif /* ITEM_H */
