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
struct Song{
    struct NotesPressGroup **notesArray;
    size_t notesArraySize;
    struct TrackInfo *tracksInfo[MAX_TRACK_SIZE];
    uint8_t trackSize;
};

struct Sheet{
    struct Measure *measures;
    size_t measureSize;
    struct NoteDuration *validNotes, *songNotes;
    size_t validNotesSize, songNotesSize;
};

struct SongInfo{
    uint32_t percision;
    float upbeat;
    float upbeatRest;
    struct Sheet sheet;
};

struct NotePressGroup{
    uint8_t type;
    float start, duration;
    uint8_t trackIndex;
};

struct NotesPressGroup{
    struct NotePressGroup **notes;
    uint8_t size;
    float timer, timerEnd;
    uint32_t BPM;
    uint8_t numerator, denominator;
};

struct NoteDuration{
    float duration;
    float correctDuration;
    uint8_t noteType;
    uint8_t flags;
};

NEW_VECTOR_TYPE(struct NoteDuration, NoteDurationVector);

struct Note{
    uint8_t value;
    uint8_t trackIndex;
    struct NoteDuration *duration;
};

NEW_VECTOR_TYPE(struct Note, NoteVector);

// struct measure:
// start index will point to the first note in the song notes array
struct Measure{
    size_t index;
    struct Note **tracks;
    size_t trackSize;
    float duration;
    uint32_t BPM;
    uint8_t numerator, denominator;
};

NEW_VECTOR_TYPE(struct Measure, MeasureVector);

#endif
