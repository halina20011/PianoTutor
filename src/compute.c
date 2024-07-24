#include "compute.h"
#include "piano.h"
#include "pianoTypes.h"

extern struct MeshBoundingBox *meshBoundingBoxes;

// C, G, D, A, E, H, F, C
#define NOTE_PITCH(step, octave) ((struct NotePitch){step, octave, 0, 0})
#define P NOTE_PITCH
struct NotePitch sharp[7] = {P(3, 5), P(0, 5), P(4, 5), P(1, 5), P(5, 3), P(2, 5), P(6, 3)};
struct NotePitch flats[7] = {P(6, 4), P(2, 5), P(5, 4), P(1, 5), P(4, 4), P(0, 5), P(3, 4)};

float positionFromCenter(enum Clef clef, struct NotePitch *notePitch){
    Pitch pitch = TO_PITCH_P(notePitch);
    Pitch center = 0;
    if(CLEF_G_15_UP <= clef && clef <= CLEF_G_15_DOWN){
        // center h, 4 => 6, 4
        struct NotePitch centerPitch = {6, 4, 0, 0};
        center = TO_PITCH(centerPitch) + (uint8_t)(G_CLEF - clef) * 8;
    }
    else if(CLEF_F_15_UP <= clef && clef <= CLEF_F_15_DOWN){
        // center d, 3 => 2, 3
        struct NotePitch centerPitch = {2, 3, 0, 0};
        center = TO_PITCH(centerPitch) + (uint8_t)(F_CLEF - clef) * 8;
    }
    else{
        fprintf(stderr, "clef %i not implemented\n", clef);
        exit(1);
    }

    int8_t p = (int8_t)(pitch - center);
    // debugf("center %i[%i] - %i => %i\n", pitch, clef, center, p);

    return p;
}

enum Meshes getAccidental(struct Note *note){
    if(!GET_BIT(note->flags, NOTE_FLAG_ACCIDENTAL)){
        return MESH_NULL;
    }
    
    debugf("flags %i\n", note->flags);
    if(GET_BIT(note->flags, NOTE_FLAG_SHARP)){
        return SHARP;
    }
    
    if(GET_BIT(note->flags, NOTE_FLAG_NATURAL)){
        return NATURAL;
    }
    
    if(GET_BIT(note->flags, NOTE_FLAG_FLAT)){
        return FLAT;
    }

    fprintf(stderr, "missing accidental value\n");
    exit(1);
}

void calculateAccidentalOffset(Staff *staffs, StaffNumber staffSize, Division d, float *accidentalOffset){
    float maxOffset = 0;

    for(StaffNumber s = 0; s < staffSize; s++){
        Staff staff = staffs[s];
        struct Notes *notes = staff[d];
        if(notes){
            if(notes->chordSize){
                for(ChordSize n = 0; n < notes->chordSize; n++){
                    struct Note *note = notes->chord[n];
                    enum Meshes meshId = getAccidental(note);
                    if(meshId != MESH_NULL){
                        maxOffset = MAX(maxOffset, MBB_MAX(meshId)[0]);
                    }
                }
            }
            else{
                struct Note *note = notes->note;
                enum Meshes meshId = getAccidental(note);
                if(meshId != MESH_NULL){
                    maxOffset = MAX(maxOffset, MBB_MAX(meshId)[0]);
                }
            }
        }
    }

    *accidentalOffset = maxOffset;
}

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

struct Item *itemStreamInit(StaffNumber staffIndex, float xPosition, float y1, float y2){
    struct ItemSteam *itemSteam = malloc(sizeof(struct ItemSteam));
    itemSteam->xStart = xPosition;
    itemSteam->y1 = y1;
    itemSteam->y2 = y2;
    return itemInit(ITEM_STEAM, MESH_NULL, staffIndex, itemSteam);
}

struct Item *itemBeamInit(StaffNumber staffIndex, float x1, float x2, float yPosition){
    struct ItemBeam *itemBeam = malloc(sizeof(struct ItemBeam));
    
    itemBeam->xStart = x1;
    itemBeam->yStart = yPosition;

    itemBeam->xEnd = x2;
    itemBeam->yEnd = yPosition;
    
    return itemInit(ITEM_BEAM, MESH_NULL, staffIndex, itemBeam);
}

void computeNumber(struct ItemPVector *itemVector, uint8_t n, float x, float y, float scale){
    uint8_t num[4] = {};
    uint8_t numSize = 0;

    float offset = 0;

    do{
        uint8_t digit = n % 10;
        num[numSize++] = digit;
        n /= 10;
    } while(n);

    for(uint8_t i = numSize; 0 < i; i--){
        enum Meshes meshId = num[i - 1] + TEXT_START;
        debugf("added %i %i %i\n", meshId, num[i - 1], TEXT_START);
        struct Item *item = itemMeshInit(meshId, 0, x + offset, y);
        item->typeSubGroup = ITEM_GROUP_TEXT;
        offset += MBB_MAX(meshId)[0] * 1.1f;
        ItemPVectorPush(itemVector, item);
    }
}

void computeKeySignature(KeySignature keySignature, struct ItemPVector *itemsVector, StaffNumber staffNumber, float *offset){
    if(!keySignature){
        return;
    }

    struct NotePitch *keySignatureArray = sharp;
    uint8_t meshId = SHARP;
    if(keySignature < 0){
        keySignature = ABS(keySignature);
        keySignatureArray = flats;
        meshId = FLAT;
    }

    float max = MBB_MAX(meshId)[0];
    float currOffset = max * (float)keySignature;

    for(KeySignature i = 0; i < keySignature; i++){
        for(StaffNumber s = 0; s < staffNumber; s++){
            struct Item *item = malloc(sizeof(struct Item));
            item->meshId = meshId;
            item->type = ITEM_MESH;
            item->staffIndex = s;
            struct ItemMesh *iM = malloc(sizeof(struct ItemMesh));
            iM->xPosition = *offset + max * i;
            item->data = iM;
            // iM->yPosition = 0;
            iM->yPosition = positionFromCenter(CLEF_G, &keySignatureArray[i]);
            ItemPVectorPush(itemsVector, item);
            // debugf("added %zu\n", itemsVector->size);
        }
    }

    *offset += currOffset;
    // debugf("%f %i\n", currOffset, keySignature);
}

void computeTimeSignature(struct Attributes *attributes, struct ItemPVector *itemsVector, StaffNumber staffNumber, float *offset){
    float maxOffset = 0;
    for(StaffNumber s = 0; s < staffNumber; s++){
        uint8_t n = attributes->numerator + TIME_0;
        uint8_t d = attributes->denominator + TIME_0;
        ItemPVectorPush(itemsVector, itemMeshInit(n, s, *offset, 0));
        float yOffset = MBB_MAX(d)[1];
        ItemPVectorPush(itemsVector, itemMeshInit(n, s, *offset, -yOffset));
        maxOffset = MAX(maxOffset, MAX(MBB_MAX(n)[0], MBB_MAX(d)[0]));
    }

    *offset += maxOffset;
}

enum NoteType noteDurationToType(struct Note *note, struct Attributes *currAttributes){
    // // remove the dot durations
    // //         b   1   2   3
    // // 8.   => 8 + 4         = 12
    // // 8..  => 8 + 4 + 2     = 14
    // // 8... => 8 + 4 + 2 + 1 = 15
    // // b    => 1  2/3 1/7 1/15
    // // for(uint8_t i = note->dots; i > 0; i--){
    // //     2 ^ 3
    // // }
    
    // o * fraction = note dur
    NoteDuration d = note->duration;
    // 1/1 * n/m = 3/2
    // n = 3; m = 2
    if(note->dots == 1){
        d = (float)note->duration * ((float)2/(float)3);
    }
    // 1/1 * n/m = 7/3
    // n = 7; m = 3
    else if(note->dots == 2){
        d = (float)note->duration * ((float)3/(float)7);
    }
    // 1/1 * n/m = 15/8
    // n = 15; m = 8
    else if(note->dots == 3){
        d = (float)note->duration * ((float)8/(float)15);
    }

    float durationInQuaterNotes = (float)d / (float)currAttributes->division;

    uint8_t powIndex = log2(durationInQuaterNotes);
    // debugf("dur %i[%i] => %i pow: %i\n", note->duration, note->dots, d, powIndex);

    return NOTE_TYPE_QUARTER - powIndex;
}

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

// enum Meshes accidental(struct Attributes *currAttributes, struct Note *note){
//     
// }


// void computeNote(struct Note *note, enum Clef clef, struct ItemPVector *itemVector, float offset, struct Attributes *currAttributes, NotePitchExtreme *notePitchExtremes){
void computeNote(struct ItemPVector *itemVector, struct Attributes *currAttributes, struct Note *note, NotePitchExtreme *notePitchExtremes, enum Clef clef, StaffNumber staffIndex, float offset, float accidentalOffset){
    float y = (GET_BIT(note->flags, NOTE_FLAG_REST)) ? 0 : positionFromCenter(clef, &note->pitch);

    if(GET_BIT(note->flags, NOTE_FLAG_ACCIDENTAL)){
        enum Meshes meshId = getAccidental(note);
        debugf("accidental meshid: %i\n", meshId);
        // enum Meshes meshId = NATURAL;
        struct Item *item = itemMeshInit(meshId, staffIndex, offset, y);
        note->item = item;
        ItemPVectorPush(itemVector, item);
    }

    if(note->noteType == NOTE_TYPE_NULL){
        note->noteType = noteDurationToType(note, currAttributes);
        debugf("calculated note type: %i\n", note->noteType);
    }

    Pitch c = TO_PITCH(note->pitch);
    notePitchExtremes[staffIndex][0] = MIN(y, c);
    notePitchExtremes[staffIndex][1] = MAX(y, c);

    enum Meshes meshId = (GET_BIT(note->flags, NOTE_FLAG_REST)) ? noteRest(note) : noteHead(note);
    struct Item *item = itemMeshInit(meshId, staffIndex, offset + accidentalOffset, y);
    item->typeSubGroup = ITEM_GROUP_NOTE_HEAD;
    note->item = item;
    ItemPVectorPush(itemVector, item);
}

void computeNotes(struct ItemPVector *itemVector, struct Attributes *currAttributes, struct Notes *notes, NotePitchExtreme *notePitchExtremes, enum Clef clef, StaffNumber staffIndex, float offset, float accidentalOffset){
    float min = 0, max = 0;
    notesUpdateExtremes(notes, clef, &min, &max);
    
    if(notes->chordSize){
        for(ChordSize c = 0; c < notes->chordSize; c++){
            struct Note *note = notes->chord[c];
            computeNote(itemVector, currAttributes, note, notePitchExtremes, clef, staffIndex, offset, accidentalOffset);
        }
    }
    else{
        computeNote(itemVector, currAttributes, notes->note, notePitchExtremes, clef, staffIndex, offset, accidentalOffset);
    }

    struct Note *note = (notes->chordSize) ? notes->chord[0] : notes->note;
    // debugf("beams: %i\n", notes->beams);
    if(GET_BIT(note->flags, NOTE_FLAG_REST) == 0 && notes->beams == 0 && NOTE_TYPE_HALF <= note->noteType){
        if(note->noteType < NOTE_TYPE_EIGHTH){
            return;
        }

        float closestBeamHeight;
        bool side;
        steamPositions(&closestBeamHeight, &side, min, max);
        
        float sign = (side == BEAM_UPPER_SIDE) ? 1 : -1;
        float beamYPosition = closestBeamHeight + BEAM_NOTE_HEAD_OFFSET * sign;

        float beamOffet = (side == BEAM_UPPER_SIDE) ? 1 : 0; 
        // float noteWidth
        // struct Note *note = (notes->chordSize) ? notes->chord[0] : notes->note;
        // return MBB_MAX(note->item->meshId)[0];

        // TODO: chords with dirrerent notes durations
        addBeamSteam(notes, clef, staffIndex, itemVector, offset + accidentalOffset + beamOffet, beamYPosition);

        // if(FLAG1 <= meshId && meshId <= FLAG5){
        //     enum Meshes meshId = noteFlag(note);
        //     itemMeshInit(meshId, staffIndex, 0, 0);
        //     itemFlagInit();
        // }
    }
}

void notesUpdateExtremes(struct Notes *notes, enum Clef clef, float *rMin, float *rMax){
    float min = *rMin, max = *rMax;
    if(notes->chordSize){
        for(ChordSize i = 0; i < notes->chordSize; i++){
            float y = positionFromCenter(clef, &notes->chord[i]->pitch);
            min = MIN(min, y);
            max = MAX(max, y);
            // debugf("%f => min %f max %f\n", y, min, max);
        }
    }
    else{
        float y = positionFromCenter(clef, &notes->note->pitch);
        min = MIN(min, y);
        max = MAX(max, y);
        // debugf("%f => min %f max %f\n", y, min, max);
    }

    *rMin = min;
    *rMax = max;
}

float computeMeasure(struct Piano *piano, size_t measureIndex, struct ItemPVector *itemVector, struct Attributes *currAttributes, NotePitchExtreme *notePitchExtremes){
    struct Measure *measure = piano->measures[measureIndex];
    printf("\n\n");
    debugf("COMPUTING MEASURE %zu\n", measureIndex);
    printMeasure(measure);
    StaffNumber staffNumber = piano->measures[0]->attributes[0]->stavesNumber;
    // StaffNumber staffNumber = piano->sheet->staffNumber;
    float offset = 0;

    MeasureSize measureSize = measure->measureSize;
    
    float notesPositions[measureSize];
    memset(notesPositions, 0, measureSize * sizeof(float));

    computeNumber(itemVector, measure->sheetMeasureIndex, 0, 8, 0.5);

    // for every item in measure
    for(Division d = 0; d < measureSize; d++){
        // void *aP = (measure->attributes) ? measure->attributes[d] : NULL;
        // debugf("division %i attributes: %p\n", d, aP);

        // attributes are only 1d array
        if(measure->attributes && measure->attributes[d]){
            struct Attributes *attributes = measure->attributes[d];
            updateAttributes(attributes, currAttributes);
            if(attributes->clefs){
                // computeClefs
                enum Clef *clefs = attributes->clefs;
                float maxOffset = 0;
                for(StaffNumber i = 0; i < attributes->stavesNumber; i++){
                    // clef starts from 1
                    uint8_t modelIndex = clefs[i];
                    if(modelIndex){
                        float currOffset = MBB_MAX(modelIndex)[0];
                        struct Item *item = itemMeshInit(modelIndex, i, offset, 0);
                        ItemPVectorPush(itemVector, item);

                        if(maxOffset < currOffset){
                            maxOffset = currOffset;
                        }
                    }
                }

                offset += maxOffset;
            }

            if(attributes->keySignature){
                computeKeySignature(currAttributes->keySignature, itemVector, staffNumber, &offset);
            }

            if(attributes->numerator || attributes->denominator){
                computeTimeSignature(attributes, itemVector, staffNumber, &offset);
            }
        }

        // check if the any of the notes has an accidental
        float accidentalOffset = 0;
        calculateAccidentalOffset(measure->staffs, staffNumber, d, &accidentalOffset);

        // for every staff
        for(StaffNumber s = 0; s < staffNumber; s++){
            Staff staff = measure->staffs[s];
            struct Notes *notes = staff[d];
            if(notes){
                computeNotes(itemVector, currAttributes, notes, notePitchExtremes, currAttributes->clefs[s], s, offset, accidentalOffset);   
            }
        }

        notesPositions[d] = offset + accidentalOffset;
        offset += accidentalOffset + 1.0f;
    }

    // add beams for every staff
    for(StaffNumber s = 0; s < staffNumber; s++){
        computeBeams(measure, s, currAttributes, itemVector, notesPositions);
    }

    return offset;
}

void computeMeasures(struct Piano *piano){
    // create a vector that will hold a measure item
    struct ItemMeasurePVector *itemMeasureV = ItemMeasurePVectorInit();

    // local item vector that will be all the items that are in the measure
    struct ItemPVector *itemVector = ItemPVectorInit();
    struct Attributes currAttributes = {};

    // add first bar
    ItemPVectorPush(itemVector, itemInit(ITEM_BAR, MESH_NULL, 0, NULL));

    StaffNumber staffNumber = piano->measures[0]->attributes[0]->stavesNumber;
    NotePitchExtreme *notePitchExtremes = calloc(staffNumber, sizeof(NotePitchExtreme));
    debugf("staff number: %i\n", staffNumber);

    for(size_t i = 0; i < piano->measureSize; i++){
    // for(size_t i = 0; i < 1; i++){
        float offset = computeMeasure(piano, i, itemVector, &currAttributes, notePitchExtremes);

        // add measue end bar
        struct Item *bar = itemInit(ITEM_BAR, MESH_NULL, 0, NULL);
        ItemPVectorPush(itemVector, bar);

        struct ItemMeasure *itemMeasure = malloc(sizeof(struct ItemMeasure));
        struct Item **items = ItemPVectorDuplicate(itemVector, &itemMeasure->size);
        itemMeasure->items = items;
        itemMeasure->width = offset;

        ItemMeasurePVectorPush(itemMeasureV, itemMeasure);
        itemVector->size = 0;
    }

    float *staffOffsets = malloc(sizeof(float) * staffNumber);
    staffOffsets[0] = 0;
    for(StaffNumber s = 1; s < staffNumber; s++){
        // eatch staff has height of 9.0f (4.5f) from center,
        // there should be at least one staff between them
        // the extrem should be at least on A3 (G clef) additional 2 lines eg 4f
        // C1 -------
        //    min + two lines offset
        //
        //    max + two lines offset
        // C2 ------
        float c1 = MAX(notePitchExtremes[s - 1][0], 4.5f + 4.0f);
        float c2 = MAX(notePitchExtremes[s][1], 4.5f + 4.0f);
        staffOffsets[s] = staffOffsets[s - 1] + (c1 + c2);
        debugf("staff %i offset %f %f => %f\n", s, c1, c2, staffOffsets[s]);
    }

    struct Sheet *sheet = malloc(sizeof(struct Sheet));
    sheet->measures = ItemMeasurePVectorDuplicate(itemMeasureV, &sheet->measuresSize);
    sheet->staffNumber = staffNumber;
    sheet->extremePitches = notePitchExtremes;
    sheet->staffOffsets = staffOffsets;

    ItemMeasurePVectorFree(itemMeasureV);

    piano->sheet = sheet;
}
