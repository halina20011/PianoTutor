#ifndef SHEETS
#define SHEETS

#include <math.h>

#include "notes.h"
#include "midiParser.h"
#include "midiPiano.h"

#define NOTES_TYPE_COUNT 7
#define MIN_NOTE_COUNT 32

enum{
    NO_DOT,
    DOTTED,
    DOUBLE_DOTTED,
};

enum{
    NOT_TUPLET
};

enum{
    MAXIMA,
    LONG,
    DOUBLE_WHOLE_NOTE,
    WHOLE_NOTE,
    HALF_NOTE,
    QUATER_NOTE,
    EIGHT_NOTE,
    SIXTEENTH_NOTE,
    THIRTY_SECOND_NOTE,
    SIXTY_FOURTH_NOTE,
    HUNDRED_TWENTY_EIGHTH_NOTE,
    TWO_HUNDRED_FIFTY_SIXTH_NOTE
};

#define NOTE_VALUE(type, duration, doted, triplet) {duration, duration, type, doted, triplet}

#define MAX_CORRECTION_FILE_LINE_SIZE 100

void validNotesPrint(struct MidiNoteDuration *noteDurations, size_t size);
struct MidiNoteDuration *generateValidNotes(uint32_t percision, size_t *rSize);

void songsNoteDurations(struct MidiSong *song, struct MidiSongInfo *songInfo, struct MidiSheet *sheet, bool debug);

void correctDurations(char *correctionFilePath, struct MidiSongInfo *songInfo, struct MidiSheet *sheet);

void generateMeasure(struct MidiSong *song, struct MidiSongInfo *songInfo, struct MidiSheet *sheet);
void generateSheet(struct MidiSong *song, struct MidiSongInfo songInfo);

#endif
