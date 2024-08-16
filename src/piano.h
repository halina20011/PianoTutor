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

struct Piano *pianoInit(struct Measure **measures, size_t measureSize);
void computeMeasures(struct Piano *piano);
void computeKeyboard(struct Piano *piano, struct NotePitch *s, struct NotePitch *e);

uint8_t fromPitch(struct NotePitch *pitch);

void pressNote(struct Piano *piano, struct Note *note, Division divisionCounter);
void unpressNote(struct Piano *piano, size_t i);

void drawSheet(struct Piano *piano);
void drawKeyboard(struct Piano *piano);

void pianoPlaySong(struct Piano *piano);

#endif
