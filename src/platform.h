#ifndef PLATFORM_H
#define PLATFORM_H
#include "psx.h"
/* WIP host boundaries. No physical media or memory-card persistence. */
extern uint32 platform_boundary_calls[3];
extern uint32 card_backend_events[8][5];
extern uint8 card_backend_pending[8];
void card_backend_request_no_media(uint32 grp);
sint32 card_backend_test_event(uint32 handle);
void card_backend_reset_events(void);
uint32 card_backend_open_event(uint32 desc, uint32 spec, uint32 mode, uint32 callback);
sint32 card_backend_enable_event(uint32 handle);
sint32 card_backend_read(sint32 slot, void *data, uint32 bytes);
#endif
