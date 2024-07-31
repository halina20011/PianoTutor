#ifndef NOTES_XML
#define NOTES_XML

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <float.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <zip.h>

#include "vector.h"
#include "helpers.h"

#include "xmlTypes.h"
#include "pianoTypes.h"

#define SIGNATURE_TYPE 0xf
#define MAX_NOTE_MAGAZINE_SIZE 255

#define XML_CHAR (const xmlChar*)

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

#define SET_BIT(variable, bit) (variable |= (1 << bit))
#define GET_BIT(variable, bit) (variable & (1 << bit))

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
    struct Attributes **attributes;
    StaffNumber stavesNumber;
    MeasureSize measureSize;
};

struct Attributes *attributesInit();
void updateAttributes(struct Attributes *attributes, struct Attributes *currAttributes);
long parseBody(xmlNodePtr part);
long parseProp(xmlNodePtr note, char *name);


void printMeasure(struct Measure *measure);
void printMeasures(struct MeasurePVector *measuresVector);

struct Measure **readNotes(char filePath[], size_t *measureSize);
struct Measure **parseMeasures(xmlNodePtr part, size_t *measureSize);
struct Measure *parseMeasure(xmlNodePtr measure, struct NoteVectorPVector *notesVectorMagazine, struct Attributes *currAtrributes);
struct Attributes *parseAttributes(xmlNodePtr part, struct Attributes *currAtrributes);

struct Note *parseNote(xmlNodePtr part, StaffNumber *staveIndex, bool *isChord);
void notesMagazinePrint(struct NoteVectorPVector *notesVectorMagazine);
void flushNotes(Staff staff, struct NoteVectorPVector *notesVectorMagazine, size_t measureNoteSize);
enum NoteType parseNoteType(xmlNodePtr node);
void parsePitch(xmlNodePtr node, struct Note *note);
void parseAccidental(xmlNodePtr parent, struct Note *note);
void parseNotations(xmlNodePtr children, struct Note *note);
void parseTimeModification(xmlNodePtr parent, struct Note *note);
void parseBeam(xmlNodePtr parent, Beams *rBeams);

void parseKey(xmlNodePtr part, KeySignature *keySignature);
void parseTime(xmlNodePtr part, TimeSignature *numerator, TimeSignature *denominator);
void parseClef(xmlNodePtr part, struct Attributes *a, struct Attributes *currAtrributes);

#endif
