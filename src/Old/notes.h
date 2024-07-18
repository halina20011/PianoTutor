#ifndef NOTES
#define NOTES

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "vector.h"

#define MAX_TRACK_SIZE 8

#define NOTE_DURATION(duration, percision) ((float)((uint32_t)(duration * percision)) / (float)percision)

// struct Song
// holds an array of all notes
// holds an array of measures
struct MidiSong{
    struct MidiNotesPressGroup **notesArray;
    size_t notesArraySize;
    struct MidiTrackInfo *tracksInfo[MAX_TRACK_SIZE];
    uint8_t trackSize;
    uint32_t PPQ;
};

struct MidiSheet{
    struct MidiMeasure *measures;
    size_t measureSize;
    struct MidiNoteDuration *validNotes, *songNotes;
    size_t validNotesSize, songNotesSize;
};

struct MidiSongInfo{
    uint32_t percision;
    float upbeat;
    float upbeatRest;
    struct MidiSheet sheet;
};

struct MidiNotePressGroup{
    uint8_t type;
    float duration;
    uint32_t ticks, start;
    uint8_t trackIndex;
};

struct MidiNotesPressGroup{
    struct MidiNotePressGroup **notes;
    uint8_t size;
    float timer, timerEnd;
    uint32_t BPM;
    uint8_t numerator, denominator;
};

struct MidiNoteDuration{
    float duration, correctDuration;
    uint8_t noteType;
    // bit fields
    uint8_t dot : 2;
    uint8_t triplet : 1;
};

NEW_VECTOR_TYPE(struct MidiNoteDuration, MidiNoteDurationVector);

struct MidiNote{
    uint8_t value;
    uint8_t trackIndex;
    struct MidiNoteDuration *duration;
};

NEW_VECTOR_TYPE(struct MidiNote, MidiNoteVector);

// struct measure:
// start index will point to the first note in the song notes array
struct MidiMeasure{
    size_t index;
    struct MidiNote **tracks;
    size_t trackSize;
    float duration;
    uint32_t BPM;
    uint8_t numerator, denominator;
};

NEW_VECTOR_TYPE(struct MidiMeasure, MidiMeasureVector);

#endif
