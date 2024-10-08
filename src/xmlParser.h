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

struct Attributes *attributesInit(void);
void updateAttributes(struct Attributes *attributes, struct Attributes *currAttributes);
long parseBody(xmlNodePtr part);
long parseProp(xmlNodePtr note, char *name);

Pitch notePitchToPitch(struct NotePitch *pitch);

void printMeasure(struct Measure *measure);
void printMeasures(struct MeasurePVector *measuresVector);

struct Measure **readNotes(char filePath[], size_t *measureSize);
void measurePitchExtreme(struct Measure *measure, struct Note *note, StaffNumber staffIndex);
struct Measure **parseMeasures(xmlNodePtr part, size_t *measureSize);
struct Measure *parseMeasure(xmlNodePtr measure, struct NoteVectorPVector *notesVectorMagazine, struct Attributes *currAtrributes);
struct Attributes *parseAttributes(xmlNodePtr part, struct Attributes *currAtrributes);

// ./xmlNoteParser.c
struct Note *parseNote(xmlNodePtr part, StaffNumber *staveIndex, bool *isChord);
void noteNonZeroDuration(struct Note *note, size_t measureSize, struct Attributes *currAttribute);
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
