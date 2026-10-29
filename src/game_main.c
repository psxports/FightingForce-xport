#include "game.h"
#include "diagnostics.h"

int frontend_run(void);

int game_run_arguments(int argc, char **argv)
{
    if (!diagnostic_options(&argc, argv))
        return 2;
    return frontend_run();
}
