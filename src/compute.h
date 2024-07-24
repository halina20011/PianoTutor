#ifndef COMPUTE
#define COMPUTE

#include <math.h>
#include <float.h>

#include "helpers.h"

#include "pianoTypes.h"
#include "xmlParser.h"
#include "helpers.h"

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a < b) ? b : a)

#define MBB_MIN(id) meshBoundingBoxes[id].min
#define MBB_MAX(id) meshBoundingBoxes[id].max

#define TO_PITCH(notePitch)     (notePitch.octave * 8 + notePitch.step)
#define TO_PITCH_P(notePitch)   (notePitch->octave * 8 + notePitch->step)

#define BEAM_NOTE_HEAD_OFFSET 4.0f
#define BEAM_HEIGHT 0.5f
#define BEAM_DISTANCE 0.25f
#define BEAM_OFFSET (BEAM_HEIGHT + BEAM_DISTANCE)

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

struct Item *itemInit(enum ItemType type, enum Meshes meshId, StaffNumber staffIndex, void *data);
struct Item *itemMeshInit(enum Meshes meshId, StaffNumber staffIndex, float xPosition, float yPosition);
struct Item *itemStreamInit(StaffNumber staffIndex, float xPosition, float y1, float y2);
struct Item *itemBeamInit(StaffNumber staffIndex, float x1, float x2, float yPosition);


void computeKeySignature(KeySignature keySignature, struct ItemPVector *itemsVector, StaffNumber staffNumber, float *offset);
void computeTimeSignature(struct Attributes *attributes, struct ItemPVector *itemsVector, StaffNumber staffNumber, float *offset);

// computeBeam.c
void notesUpdateBeamDepth(struct Notes *notes, uint8_t *maxBeamDepth);
void findBeams(Staff staff, Division size, struct BeamPosition *beams, uint8_t beamDepth, Division *rBeamsSize);
void beamXPosition(float *notesPositions, Division pos, float y, float *rPos);
void steamPositions(float *rClosestBeamHeight, bool *rSide, float min, float max);
void addBeamSteam(struct Notes *notes, enum Clef clef, StaffNumber staffIndex, struct ItemPVector *itemVector, float beamXPos, float beamYpos);
void addBeam(Staff staff, StaffNumber staffIndex, struct ItemPVector *itemVector, float *notesPositions, struct BeamPosition *bP, float y);
void computeBeams(struct Measure *measure, StaffNumber staffIndex, struct Attributes *currAttributes, struct ItemPVector *itemVector, float *notesPositions);


enum NoteType noteDurationToType(struct Note *note, struct Attributes *currAttributes);
enum Meshes noteHead(struct Note *note);
enum Meshes noteRest(struct Note *note);
void computeNote(struct ItemPVector *itemVector, struct Attributes *currAttributes, struct Note *note, NotePitchExtreme *notePitchExtremes, enum Clef clef, StaffNumber staffIndex, float offset, float accidentalOffset);
void computeNotes(struct ItemPVector *itemVector, struct Attributes *currAttributes, struct Notes *notes, NotePitchExtreme *notePitchExtremes, enum Clef clef, StaffNumber staffIndex, float offset, float accidentalOffset);


void updateBeamHeight(struct Notes *notes, uint8_t *maxBeamHeight);
void notesUpdateExtremes(struct Notes *notes, enum Clef clef, float *rMin, float *rMax);

float computeMeasure(struct Piano *piano, size_t measureIndex, struct ItemPVector *itemVector, struct Attributes *currAttributes, NotePitchExtreme *notePitchExtremes);
void computeMeasures(struct Piano *piano);

#endif
