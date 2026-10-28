#ifndef ACTOR_ITEMS_H
#define ACTOR_ITEMS_H

#include "xport.h"
#include "actors.h"
#include "animation.h"

typedef enum ACTOR_ITEM_HAND
{
    ACTOR_ITEM_LEFT,
    ACTOR_ITEM_RIGHT,
    ACTOR_ITEM_HAND_COUNT
} ACTOR_ITEM_HAND;

typedef union ACTOR_ITEMS
{
    sint8 slot[ACTOR_ITEM_HAND_COUNT];
    /* The original empty-hand test also returns these packed sixteen bits */
    uint16 bits;
} ACTOR_ITEMS;

extern ACTOR_ITEMS actor_items[ACTOR_COUNT];

GDB_CALL sint32 actor_items_drop_on_hit(sint32 idx);
GDB_CALL uint32 actor_items_drop(sint32 idx);
GDB_CALL uint32 actor_items_calc_weapon_point(sint32 idx, sint32 half, uint32 out[3]);
GDB_CALL sint32 actor_items_hit_weapon(sint32 idx, sint32 half);
GDB_CALL sint32 animation_start_held(sint32 idx, ANIMATION_BANK_ID table, sint32 animation);

GDB_CALL sint32 actor_items_fire_projectile(sint32 idx);
GDB_CALL sint32 actor_items_fire_ray(sint32 idx);
GDB_CALL sint32 actor_items_fire_spread(sint32 idx);

#endif
