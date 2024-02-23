#ifndef MIDI_PARSER
#define MIDI_PARSER

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "singlyLinkedList.h"
#include "vector.h"

#define HEX_VALUE   "%02x"

#define BIT_MASK_7 0b01111111
#define BIT_MASK_4 0b00001111

#define N_BYTE(byte, n) ((byte & 0xFF) << (n * 8))

#define BUFFER_TO_32(buffer) (N_BYTE(buffer[0], 3) | N_BYTE(buffer[1], 2) | N_BYTE(buffer[2], 1) | N_BYTE(buffer[3], 0))
#define BUFFER_TO_24(buffer) (N_BYTE(buffer[0], 2) | N_BYTE(buffer[1], 1) | N_BYTE(buffer[2], 0))
#define BUFFER_TO_16(buffer) (N_BYTE(buffer[0], 1) | N_BYTE(buffer[1], 0))

#define REVERSE_24(N) (((N & 0xFF0000) >> 16) | (N & 0x00FF00) | ((N & 0x0000FF) << 16))

#define SIZE_OF_SIZE1 3
#define SIZE_OF_SIZE2 6

// Major MIDI messages
#define EVENT_NOTE_OFF                  0b10000000
#define EVENT_NOTE_ON                   0b10010000
#define EVENT_POLYPHONIC_KEY_PREASSURE  0b10100000
#define EVENT_CONTROL_CHANGE            0b10110000
#define EVENT_PROGRAM_CHANGE            0b11000000
#define EVENT_CHANNEL_PREASURE          0b11010000
#define EVENT_PITCH_WHEEL_CHANGE        0b11100000
#define EVENT_CHANNEL_MODE_MESSAGE      0b10110000

#define EVENT_SONG_SELECT               0b11110011

#define META_SET_TEMPO              0x51
#define META_SET_TEMPO_SIZE         3 
#define META_SMPTE_OFFSET           0x54
#define META_SMPTE_OFFSET_SIZE      5
#define META_TIME_SIGNATURE         0x58
#define META_TIME_SIGNATURE_SIZE    4

#define CHECK_META_DATA_SIZE(M_SIZE) do{ \
    if(metaDataLength != M_SIZE){ \
        fprintf(stderr, "metadata: " HEX_VALUE " has wrong size\n", metaDataLength); \
        exit(1); \
    } \
} while(0)

#define NOTE_BUFFER_SIZE 127

#define NUMBER_OF_NOTES 12
static char noteNames[NUMBER_OF_NOTES][3] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "H"
};

enum NoteValue{
    WHOLE,
    WHOLE_DOT,
    HALF,
    HALF_DOT,
    QUARTER,
    QUARTER_DOT,
    EIGHT,
    EIGHT_DOT,
};

// static char NoteValuesNames[NUMBER_OF_NOTES * 2][15] = {
//     "whole",
//     "whole dot",
//     "half",
//     "half dot",
//     "quarter",
//     "quarter dot",
//     "eight",
//     "eight dot",
// };
//
// struct NoteDuration{
//     double duration;
//     uint8_t noteValue;
// };

// static struct NoteDuration noteDurations[NUMBER_OF_NOTES * 2];

struct Chunk{
    uint8_t name[4];
    uint32_t size;
    uint8_t *data;
};

struct Note{
    uint8_t type;
    float start, duration;
    uint8_t trackMask;
};

struct Notes{
    struct Note **notes;
    uint8_t size;
    float timer, timerEnd;
};

struct TrackInfo{
    uint8_t minNote, maxNote;
};

#define MAX_TRACK_SIZE 8

struct Song{
    struct Notes **notes;
    uint32_t PPQ;
    size_t size;
    struct TrackInfo *tracksInfo[MAX_TRACK_SIZE];
};

struct Song *midiParser(const char *filePath);

struct Chunk *chunkInit();
struct Note *noteInit(uint8_t type, float start, float duration, uint8_t trackMask);
struct Notes *notesInit(struct Note **n, size_t size);

uint32_t variableLengthValue(uint8_t *data, uint32_t *endOffset);
void calculateNoteDurations();
double getClosestNote(double);

void addEvent(struct Note *notes, struct List *list, uint8_t velocity, uint32_t delta, uint32_t PPQ);

struct List *parseTrack(uint8_t *data, uint32_t size, uint32_t PPQ, uint8_t trackMask, struct TrackInfo **returnTrackInfo);

void analyse(struct Song *song, int percision);
struct Song *generateSong();

#endif
