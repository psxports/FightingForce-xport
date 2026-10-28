#ifndef HIGH_SCORES_H
#define HIGH_SCORES_H

#include "xport.h"

enum
{
    HIGH_SCORE_COUNT = 10,
    HIGH_SCORE_NAME_BYTES = 4
};

typedef union HIGH_SCORE_NAME
{
    char text[HIGH_SCORE_NAME_BYTES];
    uint32 bits;
} HIGH_SCORE_NAME;

typedef struct HIGH_SCORE
{
    uint32 score;
    HIGH_SCORE_NAME name;
    uint32 flags;
} HIGH_SCORE;

typedef union HIGH_SCORE_INPUT_NAME
{
    char text[8];
    uint32 bits;
} HIGH_SCORE_INPUT_NAME;

typedef struct HIGH_SCORE_WORKSPACE
{
    HIGH_SCORE_INPUT_NAME name;
    char text[129];
} HIGH_SCORE_WORKSPACE;

extern HIGH_SCORE_WORKSPACE high_score_workspace;
extern HIGH_SCORE high_scores[HIGH_SCORE_COUNT];
extern const HIGH_SCORE high_scores_defaults[HIGH_SCORE_COUNT];

GDB_CALL sint32 high_scores_insert(uint32 score);
GDB_CALL sint32 high_scores_draw(void);
GDB_CALL uint32 high_scores_run(uint32 score0, uint32 score1);
GDB_CALL uint32 high_scores_resume(const uint8 *saved);

#endif
