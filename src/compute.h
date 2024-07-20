#ifndef COMPUTE
#define COMPUTE

#include <math.h>

#include "pianoTypes.h"
#include "xmlParser.h"


#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a < b) ? b : a)

#define MBB_MIN(id) meshBoundingBoxes[id].min
#define MBB_MAX(id) meshBoundingBoxes[id].max

#define TO_PITCH(notePitch)     (notePitch.octave * 8 + notePitch.step)
#define TO_PITCH_P(notePitch)   (notePitch->octave * 8 + notePitch->step)

#define NOTE_EXTREME(variable, min, max) (variable = (-min < max) ? max : min)

struct Item *itemInit(enum ItemType type, enum Meshes meshId, StaffNumber staffIndex, void *data);
struct Item *itemMeshInit(enum Meshes meshId, StaffNumber staffIndex, float xPosition, float yPosition);
struct Item *itemStreamInit(StaffNumber staffIndex, float xPosition, float yPosition, float height);


void computeKeySignature(struct Piano *piano, enum Clef *clefs, KeySignature keySignature, struct ItemPVector *itemsVector, StaffNumber staffNumber, float *offset);
void computeTimeSignature(struct Piano *piano, struct Attributes *attributes, struct ItemPVector *itemsVector, StaffNumber staffNumber, float *offset);

enum NoteType noteDurationToType(struct Note *note, struct Attributes *currAttributes);
enum Meshes noteHead(struct Note *note);
enum Meshes noteRest(struct Note *note);
void computeNote(struct ItemPVector *itemVector, struct Attributes *currAttributes, struct Note *note, NotePitchExtreme *notePitchExtremes, enum Clef clef, StaffNumber staffIndex, float *offset);
void computeNotes(struct ItemPVector *itemVector, struct Attributes *currAttributes, struct Notes *notes, NotePitchExtreme *notePitchExtremes, enum Clef clef, StaffNumber staffIndex, float *offset);


void notesUpdateExtremes(struct Notes *notes, enum Clef clef, float *rMin, float *rMax, uint8_t *maxBeamHeight);

float computeMeasure(struct Piano *piano, size_t measureIndex, struct ItemPVector *itemVector, struct Attributes *currAttributes, NotePitchExtreme *notePitchExtremes);
void computeMeasures(struct Piano *piano);

#endif
