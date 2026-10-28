#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include "xport.h"
#include "psx.h"

enum
{
    SCREENSHOT_NAME_BYTES = 16,
    SCREENSHOT_TGA_HEADER_BYTES = 18
};

typedef struct SCREENSHOT
{
    uint8 filename[SCREENSHOT_NAME_BYTES];
    uint8 tga_header[SCREENSHOT_TGA_HEADER_BYTES];
    PSX_RECT rect;
} SCREENSHOT;

extern SCREENSHOT screenshot;
extern const SCREENSHOT screenshot_defaults;
GDB_CALL uint32 screenshot_capture(void);

#endif
