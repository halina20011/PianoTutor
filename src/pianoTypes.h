#ifndef PIANO_TYPES
#define PIANO_TYPES

#include <stdlib.h>

#include "helpers.h"
#include "xmlTypes.h"
#include "vector.h"

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

    TEXT_START,
    TEXT_START1,
    TEXT_START2,
    TEXT_START3,

    MESHES_SIZE = TEXT_START + 11,
};

struct ItemMesh{
    float xPosition, yPosition;
};

struct ItemTie{
    float xStart, yStart;
    float xEnd, yEnd;
};

struct ItemSteam{
    float xStart, y1, y2;
};

struct ItemBeam{
    float xStart, yStart, xEnd, yEnd;
};

struct ItemFlag{
    bool inverted;
    float xPosition, yPosition;
};

enum ItemType{
    ITEM_MESH,
    ITEM_TIE,
    ITEM_STEAM,
    ITEM_BEAM,
    ITEM_FLAG,
    ITEM_BAR,
};

enum ItemSubGroup{
    ITEM_GROUP_NULL,
    ITEM_GROUP_NOTE_HEAD,
    ITEM_GROUP_TEXT
};

struct Item{
    enum ItemType type;
    enum Meshes meshId;
    enum ItemSubGroup typeSubGroup;
    StaffNumber staffIndex;
    void *data;
};

struct ItemMeasure{
    struct Item **items;
    size_t size;
    float width;
};

NEW_VECTOR_TYPE(size_t, SizeVector);
NEW_VECTOR_TYPE(struct Item*, ItemPVector);
NEW_VECTOR_TYPE(struct Item**, ItemPPVector);
NEW_VECTOR_TYPE(struct ItemMeasure*, ItemMeasurePVector);

struct Piano{
    float cursor;
    size_t currMeasure;
    struct Measure **measures;
    size_t measureSize;

    size_t *meshesDataStart, *meshesDataSize;
    struct Sheet *sheet;
};

struct PressedNote{
    Division endDivision;
    struct Note *note;
};

NEW_VECTOR_TYPE(struct PressedNote*, PressedNotePVector);

struct Sheet{
    struct ItemMeasure **measures;
    size_t measuresSize;
    NotePitchExtreme *extremePitches;
    StaffNumber staffNumber;
    float *staffOffsets;
};

struct MeshBoundingBox{
    float min[3];
    float max[3];
};

#endif
