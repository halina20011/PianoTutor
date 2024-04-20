#ifndef SHEETS
#define SHEETS

#include <math.h>

#include "notes.h"
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

#define MAX_CORRECTION_FILE_LINE_SIZE 100

void validNotesPrint(struct NoteDuration *noteDurations, size_t size);
struct NoteDuration *generateValidNotes(uint32_t percision, size_t *rSize);

void songsNoteDurations(struct Song *song, struct SongInfo *songInfo, struct Sheet *sheet, bool debug);

void correctDurations(char *correctionFilePath, struct SongInfo *songInfo, struct Sheet *sheet);

void generateMeasure(struct Song *song, struct SongInfo *songInfo, struct Sheet *sheet);
void generateSheet(struct Song *song, struct SongInfo songInfo);

#endif
