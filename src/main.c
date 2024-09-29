#include <stdio.h>

#include "inputParser.h"

#include "interface.h"

#include "xmlParser.h"
#include "piano.h"

#include "plot.h"

// TODO: program arguments
// TODO: note lines fix
// TODO: mouse scale sections/views
// TODO: draw prev measure
// TODO: repeats
// TODO: better division
// TODO: pickup measure => strip start and end
// TODO: correct bpm
// TODO: tuplets
// TODO: bars
// TODO: tie
// TODO: bounding box as static mesh

// OPTIONS
bool printHelp = false;
char *mxlFilePath = NULL;
char *midiDevicePath;
char *mode;
char *difficulty;

char *difficulty;
uint8_t percisionLevel = 3;
bool asignNotes = false;
bool hideKeyboard = false, hideNotes = false;
extern bool plotEnabled;
bool showPlot = false;
// bool useIndicator = false;

struct InputParser inputParser;

struct Interface *interface = NULL;
GLuint elementArrayBuffer = 0;

GLint globalMatUniform = 0;
GLint localMatUniform = 0;
GLint viewMatUniform = 0;
GLint colorUniform = 0;

struct InputParser inputParser = {};
void inputArguments(){
    ADD_FLAG2("-h", "--help", &printHelp, INPUT_TYPE_SWITCH, "print this help message");
    ADD_FLAG2("-f", "--file", &mxlFilePath, INPUT_TYPE_STR, "specifi mxl file");
    ADD_FLAG1("--device", &midiDevicePath, INPUT_TYPE_STR, "specifi path to midi device\n\
            options: \"auto\", [absolutePath] [onlyMidiDeviceName]\n\
            auto option will use any device that matches /dev/snd/midi* (default value is auto)");
    ADD_FLAG1("--mode", &mode, INPUT_TYPE_STR, "select mode [play, learn] (default is play)");

    ADD_INPUT_OPTION("--learn-mode", &mode, INPUT_TYPE_STR, "select learn mode [wait, practise] (default is wait)", 
            2, "wait", "practise");
    ADD_FLAG1("--difficulty", &difficulty, INPUT_TYPE_STR, "select difficulty for play mode\n\
            learn: does not go backword, it waits for every note to be played\n\
            practise: counts a miss played notes in measure, if it reatches certain treshold it will put you 'n'\n\
                      measures backwords");
    ADD_FLAG1("--hide-keyboard", &hideKeyboard, INPUT_TYPE_SWITCH, "hide keyboard section on the screen");
    ADD_FLAG1("--hide-notes", &hideKeyboard, INPUT_TYPE_SWITCH, "hide notes section on the screen");
    ADD_FLAG1("--plot", &plotEnabled, INPUT_TYPE_SWITCH, "plot data values on screen");
    ADD_FLAG1("--showPlot", &showPlot, INPUT_TYPE_SWITCH, "show error equation plot using gnuplot");
}

int main(int argc, char **argv){
    inputInit(inputArguments);
    parseInput(argc, argv);

    if(printHelp){
        printHelpMessage(inputParser);
        return 0;
    }

    if(showPlot){
        printPlotEquation();
        exit(0);
    }

    plotInit();
    
    interface = calloc(1, sizeof(struct Interface));
    interface->scale = 1;
    interface->paused = true;
    interface->showBoudningBox = 0;
    
    if(mxlFilePath == NULL){
        fprintf(stderr, "no xml file specified\n");
        return 1;
    }
    
    size_t measuresSize = 0;
    struct Measure **measures = readNotes(mxlFilePath, &measuresSize);

    struct Piano *piano = pianoInit(measures, measuresSize, hideKeyboard, hideNotes);

    // piano->measureSize = 12;
    computeMeasures(piano);
    // computeKeyboard(piano, &(struct NotePitch){0, 0, 0, 0}, &(struct NotePitch){6, 6, 0, 0});
    Pitch start = piano->sheet->staffsPitchExtreme[piano->sheet->staffNumber - 1].min;
    Pitch end = piano->sheet->staffsPitchExtreme[0].max;
    debugf("start, end: %i %i\n", start, end);
    computeKeyboard(piano, start, end);

    // plotNoteError(100);

    // plotNoteErrorSize(100);
    // float maxError = noteErrorSize(10);
    // debugf("max note error %f\n", maxError);
    // return 0;

    pianoPlaySong(piano);
    // pianoLearnSong(piano);

    glfwTerminate();
    return 0;
}
