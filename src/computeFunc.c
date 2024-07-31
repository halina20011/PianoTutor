#include "compute.h"

struct Item *itemInit(enum ItemType type, enum Meshes meshId, StaffNumber staffIndex, void *data){
    struct Item *item = malloc(sizeof(struct Item));
    
    item->type = type;
    item->meshId = meshId;
    item->staffIndex = staffIndex;
    item->data = data;

    return item;
}

struct Item *itemMeshInit(enum Meshes meshId, StaffNumber staffIndex, float xPosition, float yPosition){
    struct Item *item = malloc(sizeof(struct Item));
    struct ItemMesh *itemMesh = malloc(sizeof(struct ItemMesh));
    itemMesh->xPosition = xPosition;
    itemMesh->yPosition = yPosition;

    item->data = itemMesh;
    item->meshId = meshId;
    item->type = ITEM_MESH;
    item->typeSubGroup = ITEM_GROUP_NULL;
    item->staffIndex = staffIndex;

    return item;
}

struct Item *itemStemInit(StaffNumber staffIndex, float xPosition, float noteOffset, float y1, float y2){
    struct ItemStem *itemStem = malloc(sizeof(struct ItemStem));
    itemStem->xStart = xPosition;
    itemStem->noteOffset = noteOffset;
    itemStem->y1 = y1;
    itemStem->y2 = y2;
    return itemInit(ITEM_STEM, MESH_NULL, staffIndex, itemStem);
}

struct Item *itemBeamInit(StaffNumber staffIndex, float x1, float w1, float x2, float w2, float yPosition){
    struct ItemBeam *itemBeam = malloc(sizeof(struct ItemBeam));
    
    itemBeam->xStart = x1;
    itemBeam->yStart = yPosition;

    itemBeam->w1 = w1;
    itemBeam->w2 = w2;

    itemBeam->xEnd = x2;
    itemBeam->yEnd = yPosition;
    
    return itemInit(ITEM_BEAM, MESH_NULL, staffIndex, itemBeam);
}

struct Item *itemLineInit(StaffNumber staffIndex, float xCenter, float y, float width){
    struct ItemLine *line = malloc(sizeof(struct ItemLine));
    line->x = xCenter - width / 2.0f;
    line->y = y;
    line->width = width;

    return itemInit(ITEM_LINE, MESH_NULL, staffIndex, line);
}

// TODO: create an map array
enum Meshes noteHead(struct Note *note){
    switch(note->noteType){
        case NOTE_TYPE_NULL:
            fprintf(stderr, "note does not have type\n");
            exit(1);
        // TODO: add all note meshes
        // case NOTE_TYPE_MAXIMA:
        // case NOTE_TYPE_LONG:
            // return DOUBLE_WHOLE_HEAD;
        case NOTE_TYPE_BREVE:
            return DOUBLE_WHOLE_HEAD;
        case NOTE_TYPE_WHOLE:
            return WHOLE_HEAD;
        case NOTE_TYPE_HALF:
            return HALF_HEAD;
        default:
            return QUATER_HEAD;
    }
}

enum Meshes noteRest(struct Note *note){
    switch(note->noteType){
        case NOTE_TYPE_NULL:
            fprintf(stderr, "note does not have type\n");
            exit(1);
        case NOTE_TYPE_WHOLE:
            return WHOLE_REST;
        case NOTE_TYPE_HALF:
            return HALF_REST;

        case NOTE_TYPE_EIGHTH:
            return EIGHT_REST;
        case NOTE_TYPE_16TH:
            return SIXTEENTH_REST;
        case NOTE_TYPE_32ND:
            return THIRTY_SECOND_REST;
        case NOTE_TYPE_64TH:
            return SIXTY_FOURTH_REST;
        case NOTE_TYPE_128TH:
            return HUNDERT_TWENTY_EIGHT_REST;

        // TODO: add all note rests
        // case NOTE_TYPE_MAXIMA:
        // case NOTE_TYPE_LONG:
        // case NOTE_TYPE_256TH:
        // case NOTE_TYPE_512TH:
        // case NOTE_TYPE_1024TH:
        default:
            return HUNDERT_TWENTY_EIGHT_REST;
    }
}

enum Meshes noteFlag(struct Note *note){
    if(NOTE_TYPE_EIGHTH <= note->noteType){
        return NOTE_TYPE_EIGHTH + FLAG1;
    }

    return MESH_NULL;
}
