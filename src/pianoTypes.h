#ifndef PIANO_TYPES
#define PIANO_TYPES

#include <stdlib.h>

#include "view.h"
#include "helpers.h"
#include "xmlTypes.h"
#include "vector.h"

#include "pianoComputeItems.h"

enum PianoNotes{
    C_NOTE, C_SHARP_NOTE, 
    D_NOTE, D_SHARP_NOTE, 
    E_NOTE, 
    F_NOTE, F_SHARP_NOTE, 
    G_NOTE, G_SHARP_NOTE, 
    A_NOTE, A_SHARP_NOTE,
    H_NOTE,
};

// NEW_VECTOR_TYPE(size_t, SizeVector);

#define NOTE_PRESS_BUFFER_SIZE 128

enum Clef{
    CLEF_F_15_UP = 1,
    CLEF_F_8_UP,
    CLEF_F,
    CLEF_F_8_DOWN,
    CLEF_F_15_DOWN,

    CLEF_G_15_UP,
    CLEF_G_8_UP,
    CLEF_G,
    CLEF_G_8_DOWN,
    CLEF_G_15_DOWN,
    CLEF_C,
};

enum NoteType{
    NOTE_TYPE_NULL,
    NOTE_TYPE_MAXIMA,
    NOTE_TYPE_LONG,
    NOTE_TYPE_BREVE,
    NOTE_TYPE_WHOLE,
    NOTE_TYPE_HALF,
    NOTE_TYPE_QUARTER,
    NOTE_TYPE_EIGHTH,
    NOTE_TYPE_16TH,
    NOTE_TYPE_32ND,
    NOTE_TYPE_64TH,
    NOTE_TYPE_128TH,
    NOTE_TYPE_256TH,
    NOTE_TYPE_512TH,
    NOTE_TYPE_1024TH
};


struct Attributes{
    Division division;
    KeySignature keySignature;
    TimeSignature numerator, denominator;
    StaffNumber stavesNumber;

    enum Clef *clefs;
};

enum NoteFlag{
    NOTE_FLAG_NULL,
    NOTE_FLAG_REST,
    NOTE_FLAG_ACCIDENTAL,
    NOTE_FLAG_SHARP,
    NOTE_FLAG_NATURAL,
    NOTE_FLAG_FLAT,
    TIE_FLAG
};

struct NotePitch{
    Pitch step, octave, stepChar;
    uint8_t alter;
};

struct PitchExtreme{
    Pitch min, max;
};

struct NotePitchExtreme{
    struct NotePitch min, max;
};

struct Note{
    uint8_t dots;
    Beams beams;
    NoteFlags flags;
    enum NoteType noteType;
    struct NotePitch pitch;
    // Pitch octave, step, stepChar;
    uint8_t accidentals;
    NoteDuration duration;
    Tuplet tuplet;
    struct Item *item;
    float y;
};

NEW_VECTOR_TYPE(struct Measure*, MeasurePVector);
NEW_VECTOR_TYPE(struct Note*, NotePVector);
NEW_VECTOR_TYPE(struct NotePVector*, NoteVectorPVector);

struct Notes{
    Beams beams;
    float x, minY, maxY;
    float width;
    struct Note **chord;
    ChordSize chordSize;
};

typedef struct Notes** Staff;

struct Measure{
    uint16_t sheetMeasureIndex;
    Staff *staffs;
    struct PitchExtreme *pitchExtreme;
    struct Attributes **attributes;
    StaffNumber stavesNumber;
    MeasureSize measureSize;
    struct MeshBoundingBox *boundingBoxes;
    struct MeshBoundingBox boundingBox;
};

struct Keyboard{
    uint8_t firstOctave;
    float keyboardWidth;
    // // index where where on gpu starts the keys
    // size_t whiteKeysDataStartOffset, blackKeysDataStartOffset, linesDataStartOffset;
    // keyboard mesh keys that are loaded at the start from global mesh
    float *keys;
    // map of notePitch => mesh start, that is used for drawing pressed notes
    size_t *keysDataStart;
    // size_t whiteKeysTrigCount, blackKeysTrigCount;
    // size_t linesTrigCount;
    float octaveWidth;
};

struct Piano{
    struct Measure **measures;
    size_t measureSize;
    struct MeshBoundingBox boundingBox;

    // size_t meshesDataStartOffset;
    // map for enum Mesh => vertex position and vertex size
    size_t *meshesDataStart, *meshesDataSize;
    
    struct Keyboard keyboard;

    struct Sheet *sheet;
    int midiDevice;

    struct View view;
    
    double error;
    // notes that are being played by the sheet
    uint8_t playedNotes[NOTE_PRESS_BUFFER_SIZE];
    struct PlayedNotePVector *playedNotesVector;
    
    // notes that re being pressed by the user
    uint8_t pressedNotes[NOTE_PRESS_BUFFER_SIZE];
    float pressedNotesPrevError[NOTE_PRESS_BUFFER_SIZE];
    double pressedNotesError[NOTE_PRESS_BUFFER_SIZE];
    struct PressedNoteVector *pressedNotesVector;

    struct PianoPlay *pianoPlay;
};

enum PianoMode{
    PIANO_MODE_PLAY,
    PIANO_MODE_LEARN
};

struct PianoPlay{
    enum PianoMode pianoMode;
    float speedScale;
    Division pause;

    float correctStreak, incorrectStreak;

    size_t measureIndex;
    Division currDivision;
    double currBmp;
    double prevTime;
    struct Attributes currAttributes;
    double divisionTimer;

    Division measureDivisionStart;
    Division divisionCounter;
    int currTimerLevel;
    int fps;
    double percentage;
};

struct Sheet{
    float cursor;
    // size_t currMeasure;

    struct ItemMeasure **measures;
    // struct NotePitchExtreme *notePitchExtreme;
    size_t measuresSize;
    StaffNumber staffNumber;
    float *staffOffsets;
    struct PitchExtreme *staffsPitchExtreme;
    float height;
};

struct PlayedNote{
    Division startDivision, endDivision;
    struct Note *note;
};

NEW_VECTOR_TYPE(struct PlayedNote*, PlayedNotePVector);

#endif
