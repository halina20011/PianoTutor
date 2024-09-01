#ifndef COMPUTE
#define COMPUTE

#include <math.h>
#include <float.h>

#include "helpers.h"

#include "pianoComputeItems.h"
// #include "pianoTypes.h"
#include "xmlParser.h"
#include "helpers.h"

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a < b) ? b : a)

#define MBB_MIN(id) meshBoundingBoxes[id].min
#define MBB_MAX(id) meshBoundingBoxes[id].max

#define TO_PITCH(notePitch)     (notePitch.octave * 7 + notePitch.step)
#define TO_PITCH_P(notePitch)   (notePitch->octave * 7 + notePitch->step)

#define BEAM_NOTE_HEAD_OFFSET 4.0f
#define BEAM_HEIGHT 0.75f
#define BEAM_DISTANCE 0.5f
#define BEAM_OFFSET (BEAM_HEIGHT + BEAM_DISTANCE)

#define SHEET_STRETCH 1.1f

struct BeamInfo{
    float yPos, offset;
};

struct BeamPosition{
    Division left, right;
    Beam firstBeam;
};

#define BEAM_LOWER_SIDE false
#define BEAM_UPPER_SIDE true

#define BEAM_UPWARDS 0
#define BEAM_DOWNWARDS 1

// ./computeFunc.c
struct Item *itemInit(enum ItemType type, enum Meshes meshId, StaffNumber staffIndex, void *data);
struct Item *itemMeshInit(enum Meshes meshId, StaffNumber staffIndex, float xPosition, float yPosition);
struct Item *itemStemInit(StaffNumber staffIndex, float xPosition, float noteOffset, float y1, float y2);
struct Item *itemBeamInit(StaffNumber staffIndex, float x1, float w1, float x2, float w2, float yPosition);
struct Item *itemLineInit(StaffNumber staffIndex, float xCenter, float y, float width);
struct Item *itemFlagInit(enum Meshes id, StaffNumber staffIndex, float x, float y, float width, bool inverted);

enum Meshes noteHead(struct Note *note);
enum Meshes noteRest(struct Note *note);
enum Meshes noteFlag(struct Note *note);

//
void computeKeySignature(KeySignature keySignature, struct ItemPVector *itemsVector, StaffNumber staffNumber, float *offset);
void computeTimeSignature(struct Attributes *attributes, struct ItemPVector *itemsVector, StaffNumber staffNumber, float *offset);

// computeBeam.c
void notesUpdateBeamDepth(struct Notes *notes, uint8_t *maxBeamDepth);
void findBeams(Staff staff, Division size, struct BeamPosition *beams, uint8_t beamDepth, Division *rBeamsSize);
void beamXPosition(Staff staff, float *notesPositions, Division pos, float y, float *rPos, float *rOffset);
void steamPositions(float *rClosestBeamHeight, bool *rSide, float min, float max);
void addBeamSteam(struct Notes *notes, StaffNumber staffIndex, struct ItemPVector *itemVector, float yBeamPos);
void addBeam(Staff staff, StaffNumber staffIndex, struct ItemPVector *itemVector, struct BeamPosition *bP, float y);
void computeBeams(struct Measure *measure, StaffNumber staffIndex, struct ItemPVector *itemVector);


enum NoteType noteDurationToType(struct Note *note, struct Attributes *currAttributes);

void computeNote(struct ItemPVector *itemVector, struct Note *note, StaffNumber staffIndex, float offset, float accidentalOffset);
void computeNotes(struct ItemPVector *itemVector, struct Notes *notes, StaffNumber staffIndex, float offset, float accidentalOffset);


void updateBeamHeight(struct Notes *notes, uint8_t *maxBeamHeight);
void notesUpdateExtremes(struct Notes *notes, enum Clef clef, float *rMin, float *rMax);

float computeMeasure(struct Piano *piano, size_t measureIndex, struct ItemPVector *itemVector, struct Attributes *currAttributes, NotePositionExtreme *staffsPositionNoteExtremes);
void computeMeasures(struct Piano *piano);

#endif
