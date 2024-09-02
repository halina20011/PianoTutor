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

#define KEYBOARD_KEY_SIZE 128
typedef uint8_t MeshStrIdSize;
#define MAX_MODEL_ID_SIZE 255

#define WHITE_KEY 0
#define BLACK_KEY 1

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

void pressNote(struct Piano *piano, struct Note *note, Division divisionCounter);
void unpressNote(struct Piano *piano, size_t i);

// pianoDraw.c
void draw(struct Piano *piano, double percentage, enum KeyboardMode keyboardMode);
void drawSheet(struct Piano *piano);
void drawKeyboard(struct Piano *piano, enum KeyboardMode keyboardMode);
void drawNotes(struct Piano *piano, double_t percentage);

void pianoPlaySong(struct Piano *piano);
void pianoLearnSong(struct Piano *piano);

#endif
