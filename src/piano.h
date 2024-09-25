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

enum KeyboardMode{
    KEYBOARD_SHEET_MODE,
    KEYBOARD_PIANO_MODE
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


struct Piano *pianoInit(struct Measure **measures, size_t measureSize, bool hideKeyboard, bool hideNotes);
void computeMeasures(struct Piano *piano);

void pressedNotes(struct Piano *piano);
void pressNote(struct Piano *piano, struct Note *note, Division divisionCounter);
void unpressNote(struct Piano *piano, size_t i);

void pianoPlayInit(struct Piano *piano, enum PianoMode pianoMode);
void pianoPlayCalculateError(struct Piano *piano);
void pianoPlayCalculateError();
bool pianoPlayUpdate(struct Piano *piano);


float errorEquation(float p);
float noteAlphaError(Pitch pitch, float notePercentage);
float noteErrorSize(size_t steps);
void pianoPlayCalculateError(struct Piano *piano);
void pianoRewind(struct Piano *piano);
void turnNotes(struct Piano *piano);

// pianoDraw.c
void draw(struct Piano *piano, enum KeyboardMode keyboardMode);
void drawSheet(struct Piano *piano);
void drawKeyboard(struct Piano *piano, enum KeyboardMode keyboardMode);
void drawNotes(struct Piano *piano);
void drawError(struct Piano *piano);

void pianoPlaySong(struct Piano *piano);
void pianoLearnSong(struct Piano *piano);

#endif
