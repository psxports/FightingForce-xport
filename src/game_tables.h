#ifndef GAME_TABLES_H
#define GAME_TABLES_H

#include "xport.h"

GDB_CALL uint32 game_tables_init_resources(void);
GDB_CALL uint32 game_tables_init_models(void);
GDB_CALL uint32 game_tables_init(void);
GDB_CALL uint32 game_tables_init_vehicles(void);
GDB_CALL uint32 game_tables_init_extra_resources(void);
GDB_CALL uint32 game_tables_init_npc_actions(void);
GDB_CALL uint32 game_tables_init_extra_models(void);
GDB_CALL uint32 game_tables_init_extra(void);

#endif
