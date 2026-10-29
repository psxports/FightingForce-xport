#ifndef ITEM_PICKUP_H
#define ITEM_PICKUP_H

#include "xport.h"
#include "actors.h"

struct MODEL_OBJECT;

enum
{
    ITEM_PICKUP_PLAYER_COUNT = 2
};

typedef struct ITEM_PICKUP
{
    uint32 nearby[ITEM_PICKUP_PLAYER_COUNT];
    sint8 pending[ACTOR_COUNT];
} ITEM_PICKUP;

extern ITEM_PICKUP item_pickup;

uint32 item_pickup_select(sint32 idx, sint32 radius);
void item_pickup_draw(struct MODEL_OBJECT *item);

uint32 item_draw_explosive_countdown(const struct MODEL_OBJECT *item);
void item_draw_explosive(const struct MODEL_OBJECT *item);

sint32 item_pickup_find_for_npc(sint32 x, sint32 z);

#endif
