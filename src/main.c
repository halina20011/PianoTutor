#include <stdio.h>


#include "inputParser.h"

#include "interface.h"

#include "xmlParser.h"
#include "piano.h"

// TODO: use correct tempto from Song struct

// TODO: draw keyboard notes
// TODO: repeats
// TODO: correct division
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

float upbeatDur = 0.0;
float upbeatRestDur = 0.0;

char *difficulty;
char *correctionFilePath;
uint8_t percisionLevel = 3;
bool printSongsNoteDurations;
bool asignNotes = false;
bool showNotes = false;

struct InputParser inputParser = {
    4,
    30,
    11,
    {
        FLAG_2_SIZE("-h", "--help", &printHelp, INPUT_TYPE_SWITCH, "print this help message"),
        FLAG_2_SIZE("-f", "--file", &mxlFilePath, INPUT_TYPE_STR, "specifi mxl file"),
        FLAG_1_SIZE("--device", &midiDevicePath, INPUT_TYPE_STR, "specifi path to midi device\n\
                options: \"auto\", [absolutePath] [onlyMidiDeviceName]\n\
                auto option will use any device that matches /dev/snd/midi* (default value is auto)"),
        FLAG_1_SIZE("--mode", &mode, INPUT_TYPE_STR, "select mode [play, learn] (default is play)"),

        FLAG_1_SIZE("--difficulty", &difficulty, INPUT_TYPE_STR, "select difficulty for play mode"),
        FLAG_1_SIZE("--upbeat", &upbeatDur, INPUT_TYPE_FLOAT, "define upbeat duration"),
        FLAG_1_SIZE("--upbeatRest", &upbeatRestDur, INPUT_TYPE_FLOAT, "define upbeat rest duration"),
        FLAG_1_SIZE("--percision", &percisionLevel, INPUT_TYPE_UINT8_T, "set percision for note durations"),
        FLAG_1_SIZE("-c", &correctionFilePath, INPUT_TYPE_STR, "define note duration file correction\n\
                first you need to use flag \"--durations\" and then edit generated file so the correct duration will be on the same line seperated by \",\" eg: \"1.422000,1.5\""),
        FLAG_1_SIZE("--durations", &printSongsNoteDurations, INPUT_TYPE_SWITCH, "get set of note durations from midi file"),
        FLAG_1_SIZE("--notes", &showNotes, INPUT_TYPE_SWITCH, "show duration of notes"),
    },
};

struct Interface *interface = NULL;
GLuint elementArrayBuffer = 0;

GLint globalMatUniform = 0;
GLint localMatUniform = 0;
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

    struct Piano *piano = pianoInit(measures, measuresSize);

    // piano->measureSize = 12;
    computeMeasures(piano);
    // computeKeyboard(piano, &(struct NotePitch){0, 0, 0, 0}, &(struct NotePitch){6, 6, 0, 0});
    Pitch start = piano->sheet->staffsPitchExtreme[piano->sheet->staffNumber - 1].min;
    Pitch end = piano->sheet->staffsPitchExtreme[0].max;
    computeKeyboard(piano, start, end);

    pianoPlaySong(piano);
    // pianoLearnSong(piano);

    glfwTerminate();
    return 0;
}
