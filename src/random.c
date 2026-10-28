#include "game.h"

/* Cold bootstrap clears the LIBC2 seed before the first gameplay or demo use */
uint32 random_seed;

GDB_CALL uint32 random_next(void)
{
    FUNCTION_MARKER(0x8006d348, "GAME.EXE");
    return psyq_rand_next(&random_seed);
}
