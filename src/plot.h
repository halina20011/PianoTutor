#ifndef __PLOT__
#define __PLOT__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

enum PlotType{
    PLOT_X,
    PLOT_ERROR,
    PLOT_NOTE_PREV_ERROR,
    PLOT_NOTE_ERROR,
    PLOT_ALPHA,
    PLOT_DIVISION,
    PLOT_SPEED_SCALE,
    PLOT_WAIT_SCALE,
    PLOT_SIZE
};

void plotInit(void);
void plotAction(enum PlotType *plotTypes, size_t size, bool enable);
void plot(enum PlotType plotType, float val);

// void plotSet(enum PlotType plotType, float val);
void plotFlush(void);

#endif
