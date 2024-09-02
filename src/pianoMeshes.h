#ifndef __PIANO_MESHES__
#define __PIANO_MESHES__

#include <stdlib.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define TEXT_SIZE 10

enum Meshes{
    // CLEFS
    MESH_NULL,
    F_CLEF_15_UP = 1,
    F_CLEF_8_UP,
    F_CLEF,
    F_CLEF_8_DOWN,
    F_CLEF_15_DOWN,

    G_CLEF_15_UP,
    G_CLEF_8_UP,
    G_CLEF,
    G_CLEF_8_DOWN,
    G_CLEF_15_DOWN,
    C_CLEF,

    COMMON_TIME,
    CUT_TIME,
    
    TIME_0,
    _TIME_1,
    TIME_2,
    TIME_3,
    TIME_4,
    _TIME_5,
    TIME_6,
    _TIME_7,
    TIME_8,
    
    WHOLE_REST,
    HALF_REST,
    EIGHT_REST,
    SIXTEENTH_REST,
    THIRTY_SECOND_REST,
    SIXTY_FOURTH_REST,
    HUNDERT_TWENTY_EIGHT_REST,

    DOUBLE_WHOLE_HEAD,
    WHOLE_HEAD,
    HALF_HEAD,
    QUATER_HEAD,

    TIE,
    NOTE_STEAM,
    NOTE_FLAG,
    NOTE_BEAM,

    FLAG1,
    FLAG2,
    FLAG3,
    FLAG4,
    FLAG5,

    SHARP,
    NATURAL,
    FLAT,

    C, D, E, F, G, A, H,
    C_SHARP, D_SHARP, F_SHARP, G_SHARP, A_SHARP,
    LINES,

    NOTE_START,

    TEXT_START = NOTE_START + 12,
    
    MESHES_SIZE = TEXT_START + TEXT_SIZE,
};

#define MESHES_SIZE TEXT_START + TEXT_SIZE

// size the the shared buffer for gpu
#define VERTEX_SIZE (3)
#define VERTEX_BYTES_SIZE (VERTEX_SIZE * sizeof(float))
#define VERTEX_BUFFER_COUNT ((4 * 1024 * 1024) / VERTEX_BYTES_SIZE)
#define VERTEX_BUFFER_BYTES_SIZE (VERTEX_BYTES_SIZE * VERTEX_BUFFER_COUNT)

enum VertexBufferItemType{
    VERTEX_BUFFER_MESHES,
    // VERTEX_BUFFER_TEXT,
    VERTEX_BUFFER_WHITE_KEYS,
    VERTEX_BUFFER_BLACK_KEYS,
    VERTEX_BUFFER_LINES,
    
    VERTEX_BUFFER_SIZE
};

void vertexBufferSet(enum VertexBufferItemType item, float *data, size_t size);
GLint vertexBufferGetPosition(enum VertexBufferItemType item);
GLint vertexBufferGetSize(enum VertexBufferItemType item);

#endif
