#ifndef __PLOT__
#define __PLOT__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

enum PlotType{
    PLOT_X,
    PLOT_ERROR,
    PLOT_NOTE_ERROR,
    PLOT_ALPHA,
    PLOT_DIVISION,
    PLOT_SIZE
};

void plotInit();
void plot(enum PlotType plotType, float val);

#endif
