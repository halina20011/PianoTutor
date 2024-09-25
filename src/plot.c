#include "plot.h"

bool plotValues = false;

char *plotNames[PLOT_SIZE] = {};

#define plotAssign(enum, val) do{\
    plotNames[enum] = strdup(val);\
}while(0)

void plot(enum PlotType plotType, float val){
    if(plotValues){
        printf("PLOT: [%s] %f\n", plotNames[plotType], val);
        fflush(stdout);
    }
}

// #define PLOT(type, val) do{\
//     printf("PLOT: [%s] %f.3", type, val);\
// }while(0)

void plotInit(){
    plotAssign(PLOT_X, "x");
    plotAssign(PLOT_ERROR, "error");
    plotAssign(PLOT_ALPHA, "alpha");
    plotAssign(PLOT_NOTE_ERROR, "noteError");
    plotAssign(PLOT_DIVISION, "division");
    for(size_t i = 0; i < PLOT_SIZE; i++){
        if(plotNames[i] == NULL){
            fprintf(stderr, "empty plot type with index %zu\n", i);
            exit(1);
        }
    }

    plot(PLOT_X, 0);
    plot(PLOT_ERROR, 0);
    // plotAssign(, "");
}