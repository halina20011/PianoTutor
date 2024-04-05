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

#define META_TRACK_NAME             0x03
#define META_PREFIX_PORT            0x20
#define META_PREFIX_PORT_SIZE       2
#define META_MIDI_PORT              0x21
#define META_MIDI_PORT_SIZE         2
#define META_SET_TEMPO              0x51
#define META_SET_TEMPO_SIZE         3 
#define META_SMPTE_OFFSET           0x54
#define META_SMPTE_OFFSET_SIZE      5
#define META_TIME_SIGNATURE         0x58
#define META_TIME_SIGNATURE_SIZE    4
#define META_KEY_SIGNATURE          0x59
#define META_KEY_SIGNATURE_SIZE     2

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

#define MAX_TRACK_SIZE 8

struct Chunk{
    uint8_t name[4];
    uint32_t size;
    uint8_t *data;
};

struct Note{
    uint8_t type;
    float start, duration;
    uint8_t trackIndex;
};

struct NotesPressGroup{
    struct Note **notes;
    uint8_t size;
    float timer, timerEnd;
    uint32_t BMP;
};

struct TrackInfo{
    uint8_t minNote, maxNote;
    uint8_t n, d, c, b;
};

// struct Song
// holds an array of all notes
// holds an array of measures
struct Song{
    struct Measure *measures;
    size_t measureSize;
    struct NotesPressGroup **notesArray;
    size_t notesArraySize;
    struct TrackInfo *tracksInfo[MAX_TRACK_SIZE];
    uint8_t trackSize;
};

// struct measure:
// start index will point to the fisrt note in the song notes array
// will have an table of size n that will map 
// n is calculated with (measure duration/min duration)
struct Measure{
    size_t index;
    struct NotePressGroup **tracks;
    size_t trackSize;
    uint32_t PPQ;
};

struct NoteInfo{
    float correctDuration;
    float firstTimerValue;
    size_t count;
};

struct SongInfo{
    int percision;
    float max;
    float upbeat;
    struct NoteInfo *noteInfo;
    size_t noteInfoSize;
};

struct Song *midiParser(const char *filePath);

struct Chunk *chunkInit();
struct Note *noteInit(uint8_t type, float start, float duration, uint8_t trackMask);

uint32_t variableLengthValue(uint8_t *data, uint32_t *endOffset);
void calculateNoteDurations();
double getClosestNote(double);

void addEvent(struct Note *notes, struct List *list, uint8_t velocity, uint32_t delta, uint32_t PPQ);

struct List *parseTrack(uint8_t *data, uint32_t size, uint32_t PPQ, uint8_t trackIndex, struct TrackInfo **returnTrackInfo);

struct Song *generateSong();

void analyse(struct Song *song, struct SongInfo *songInfo);
void printAnalysis(struct SongInfo songInfo);

void printSongInfo(struct SongInfo songInfo);

#endif
