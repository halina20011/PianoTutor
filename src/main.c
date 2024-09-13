#include <stdio.h>

#include "inputParser.h"

#include "interface.h"

#include "xmlParser.h"
#include "piano.h"

// TODO: use correct tempto from Song struct

// TODO: mouse scale sections/views
// TODO: draw prev measure
// TODO: repeats
// TODO: better division
// TODO: learnSong
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
// bool useIndicator = false;

struct InputParser inputParser = {
    4,
    30,
    7,
    {
        FLAG_2_SIZE("-h", "--help", &printHelp, INPUT_TYPE_SWITCH, "print this help message"),
        FLAG_2_SIZE("-f", "--file", &mxlFilePath, INPUT_TYPE_STR, "specifi mxl file"),
        FLAG_1_SIZE("--device", &midiDevicePath, INPUT_TYPE_STR, "specifi path to midi device\n\
                options: \"auto\", [absolutePath] [onlyMidiDeviceName]\n\
                auto option will use any device that matches /dev/snd/midi* (default value is auto)"),
        FLAG_1_SIZE("--mode", &mode, INPUT_TYPE_STR, "select mode [play, learn] (default is play)"),
        FLAG_1_SIZE("--difficulty", &difficulty, INPUT_TYPE_STR, "select difficulty for play mode\n\
                learn: does not go backword, it waits for every note to be played\n\
                practise: counts a miss played notes in measure, if it reatches certain treshold it will put you 'n'\n\
                          measures backwords"),
        FLAG_1_SIZE("--hide-keyboard", &hideKeyboard, INPUT_TYPE_SWITCH, "hide keyboard section on the screen"),
        FLAG_1_SIZE("--hide-notes", &hideKeyboard, INPUT_TYPE_SWITCH, "hide notes section on the screen"),
        // FLAG_1_SIZE("--indicator", &useIndicator, INPUT_TYPE_SWITCH, "use keyboard indicator"),
    },
};

struct Interface *interface = NULL;
GLuint elementArrayBuffer = 0;

GLint globalMatUniform = 0;
GLint localMatUniform = 0;
GLint viewMatUniform = 0;
GLint colorUniform = 0;

int main(int argc, char **argv){
    parseInput(inputParser, argc, argv);

    if(printHelp){
        printHelpMessage(inputParser);
        return 0;
    }
    
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

    // float maxError = noteErrorSize(10);
    // debugf("max note error %f\n", maxError);
    // return 0;

    // pianoPlaySong(piano);
    pianoLearnSong(piano);

    glfwTerminate();
    return 0;
}
