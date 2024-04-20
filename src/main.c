#include <stdio.h>

#include "midiParser.h"
#include "piano.h"
#include "sheets.h"
#include "inputParser.h"

// void test(uint8_t *data){
//     uint32_t i = 0;
//     uint32_t r = variableLengthValue(data, &i);
//     printf("%X %u\n", r, r);
// }

// TODO: correct track numbers
// TODO: use correct tempto from Song struct
// TODO: create note sheet window

// OPTIONS
bool printHelp = false;
char *midiFilePath = NULL;
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
        FLAG_2_SIZE("-m", "--midi", &midiFilePath, INPUT_TYPE_STR, "specifi midi file"),
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

int main(int argc, char **argv){
    parseInput(inputParser, argc, argv);

    if(printHelp){
        printHelpMessage(inputParser);
        return 0;
    }
    
    // TODO: max percisionLevel is 10 ^ 9 so it would fit in uint32_t and float has limited number of fraction
    uint32_t percision = pow(10, percisionLevel);

    size_t validNotesSize = 0;
    struct Sheet sheet;
    struct NoteDuration *validNotes = generateValidNotes(percision, &sheet.validNotesSize);
    sheet.validNotes = validNotes;
    if(showNotes){
        validNotesPrint(validNotes, sheet.validNotesSize);
        return 0;
    }

    if(midiFilePath == NULL){
        fprintf(stderr, "no midi file specified\n");
        return 1;
    }

    struct Song *song = midiParser(midiFilePath);
    struct SongInfo songInfo = {percision, upbeatDur,upbeatRestDur, NULL, 0};
    songsNoteDurations(song, &songInfo, &sheet, printSongsNoteDurations);

    if(printSongsNoteDurations){
        return 0;
    }
    
    if(correctionFilePath){
        correctDurations(correctionFilePath, &songInfo, &sheet);
    }

    generateMeasure(song, &songInfo, &sheet);
    generateSheet(song, songInfo);

    // return 0;
    int midiDevice = midiDeviceInit("auto");
    uint8_t trackMask = 1;
    uint8_t noteMask = (1 << C);
    // uint8_t noteMask = 0xff;
    // selectRegion(song, trackMask);
    pianoInit();
    playSong(song, midiDevice, noteMask);
    // learnSong(song, TUTORIAL, trackMask, noteMask);
    pianoExit();
    return 0;
}
