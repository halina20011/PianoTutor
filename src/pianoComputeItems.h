#ifndef __PIANO_COMPUTE_ITEMS__
#define __PIANO_COMPUTE_ITEMS__

#include <stdlib.h>

#include "xmlTypes.h"
#include "vector.h"

#include "pianoMeshes.h"

struct MeshBoundingBox{
    float min[3];
    float max[3];
};

struct ItemMesh{
    float xPosition, yPosition;
};

struct ItemTie{
    float xStart, yStart;
    float xEnd, yEnd;
};

struct ItemStem{
    float xStart, noteOffset, y1, y2;
};

struct ItemBeam{
    float xStart, yStart, w1, w2, xEnd, yEnd;
};

struct ItemLine{
    float x, y, width;
};

struct ItemFlag{
    bool inverted;
    float xPosition, yPosition, width;
};

enum ItemType{
    ITEM_MESH,
    ITEM_TIE,
    ITEM_STEM,
    ITEM_BEAM,
    ITEM_FLAG,
    ITEM_BAR,
    ITEM_LINE
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
    struct MeshBoundingBox boundingBox;
};

struct ItemMeasure{
    struct Item **items;
    struct MeshBoundingBox boundingBox;
    size_t size;
    float width;
};

NEW_VECTOR_TYPE(struct Item*, ItemPVector);
NEW_VECTOR_TYPE(struct Item**, ItemPPVector);
NEW_VECTOR_TYPE(struct ItemMeasure*, ItemMeasurePVector);

#endif
