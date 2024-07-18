#ifndef PIANO
#define PIANO

#include <string.h>
#include <float.h>
#include <stdlib.h>

#include "vector.h"

#include "pianoTypes.h"
#include "xmlParser.h"
#include "graphics.h"

#include "compute.h"

typedef uint8_t MeshStrIdSize;
typedef uint8_t MeshId;
#define MAX_MODEL_ID_SIZE 255

struct Piano *pianoInit(struct Measure **measures, size_t measureSize);
void computeMeasures(struct Piano *piano);
void drawSheet(struct Piano *piano);

#endif
