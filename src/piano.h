#ifndef PIANO
#define PIANO

#include <string.h>
#include <float.h>
#include <stdlib.h>

#include "vector.h"
#include "helpers.h"

#include "pianoTypes.h"

#include "xmlParser.h"
#include "graphics.h"

#include "compute.h"

#include "midiDevice.h"
#include "view.h"

#include "plot.h"

#define KEYBOARD_KEY_SIZE 128
typedef uint8_t MeshStrIdSize;
#define MAX_MODEL_ID_SIZE 255

#define WHITE_KEY 0
#define BLACK_KEY 1

#define NOTE_OCTAVE(pitch) ((pitch) / 12)

#define NOTE_UNPRESED UINT8_MAX

#define ENABLED_STAFF(piano, staff) ((piano->hiddenStaffs & (1 << staff)) == 0)

enum KeyboardMode{
    KEYBOARD_PLAYED_NOTES_MODE,
    KEYBOARD_PRESSED_NOTES_MODE
};

// pianoFunc.c
enum Meshes assignMeshId(char *name);
void meshBoundingBoxClear(struct MeshBoundingBox *bb);
void meshBoundingBox(struct MeshBoundingBox *bb, float *data, uint32_t size);
void meshBoundingBoxUpdate(struct MeshBoundingBox *bbToUpdate, struct MeshBoundingBox *itemUpdatingBb, vec3 offset);
void meshBoundingBoxPrint(struct MeshBoundingBox *bb);
void meshBoundingBoxDraw(struct MeshBoundingBox *bb, struct Color color);

void loadPianoMeshes(struct Piano *piano);

uint8_t getKeyType(uint8_t note);
enum Meshes pitchToNote(Pitch p);
enum PianoNotes mesheNoteToNote(enum Meshes note);
void computeKeyboard(struct Piano *piano, Pitch start, Pitch end);

void dissableStaff(struct Piano *piano, StaffNumber staffNumber);
void enableStaff(struct Piano *piano, StaffNumber staffNumber);


struct Piano *pianoInit(struct Measure **measures, size_t measureSize, bool hideKeyboard, bool hideNotes);
void computeMeasures(struct Piano *piano);

void pressedNotes(struct Piano *piano);
void pressNote(struct Piano *piano, struct Note *note, StaffNumber staff, Division divisionCounter);
void unpressNote(struct Piano *piano, size_t i);

void pianoPlayInit(struct Piano *piano, enum PianoMode pianoMode);
void pianoPlayCalculateError(struct Piano *piano);
void pianoPlayCalculateError();
bool pianoPlayUpdate(struct Piano *piano);


void plotNoteError(size_t step);
void printPlotEquation(void);
float errorEquation(float p);
float noteAlphaError(Pitch pitch, float notePercentage, bool add, float multiplier);
float noteErrorSize(size_t steps);
void plotNoteErrorSize(size_t to);
void pianoPlayCalculateError(struct Piano *piano);
void pianoRewind(struct Piano *piano);
void turnNotes(struct Piano *piano);

// pianoDraw.c
void draw(struct Piano *piano, enum KeyboardMode keyboardMode);
void drawSheet(struct Piano *piano);
void drawKeyboard(struct Piano *piano, enum KeyboardMode keyboardMode);
void drawNotes(struct Piano *piano);
void drawError(struct Piano *piano);
void setNoteStaffColors(struct Piano *piano, struct Color colors[], size_t colorSize);

void pianoSetMeasureRange(struct Piano *piano, Division from, Division to);
void pianoPlaySong(struct Piano *piano);
void pianoLearnSong(struct Piano *piano);

#endif
