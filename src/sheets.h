#ifndef SHEETS
#define SHEETS

#include <math.h>

#include "midiParser.h"
#include "piano.h"

#define NOTES_TYPE_COUNT 7
#define MIN_NOTE_COUNT 32

enum{
    FULL,
    HALF,
    DOT,
    TRIPLET,
};

struct NoteDuration{
    float duration;
    uint8_t note;
    uint8_t flags;
};

#define MAX_CORRECTION_FILE_LINE_SIZE 100

void printNoteDurations(struct NoteDuration *noteDurations, size_t size);
struct NoteDuration *generateNotes(int percision, size_t *rSize);

void generateMeasure(struct Song *song, struct SongInfo songInfo);
void generateSheet(struct Song *song, struct SongInfo songInfo);

void correctDurations(char *correctionFilePath, struct SongInfo *songInfo);

#endif
