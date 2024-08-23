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

#define KEYBOARD_KEY_SIZE 128
typedef uint8_t MeshStrIdSize;
#define MAX_MODEL_ID_SIZE 255

// size the the shared buffer for gpu
#define VERTEX_SIZE (3)
#define VERTEX_BYTES_SIZE (VERTEX_SIZE * sizeof(float))
#define VERTEX_BUFFER_COUNT ((4 * 1024 * 1024) / VERTEX_BYTES_SIZE)
#define VERTEX_BUFFER_BYTES_SIZE (VERTEX_BYTES_SIZE * VERTEX_BUFFER_COUNT)

#define WHITE_KEY 0
#define BLACK_KEY 1

enum KeyboardMode{
    KEYBOARD_SHEET_MODE,
    KEYBOARD_PIANO_MODE
};

// pianoFunc.c
enum Meshes assignMeshId(char *name);
void meshBoundingBox(struct MeshBoundingBox *bb, float *data, uint32_t size);
void meshBoundingBoxPrint(enum Meshes id);
void loadPianoMeshes(struct Piano *piano);

uint8_t getKeyType(uint8_t note);
enum Meshes pitchToNote(Pitch p);
void computeKeyboard(struct Piano *piano, Pitch start, Pitch end);


struct Piano *pianoInit(struct Measure **measures, size_t measureSize);
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
