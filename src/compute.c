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
    float pitch = TO_PITCH_P(notePitch);
    float center = 0;
    if(CLEF_G_15_UP <= clef && clef <= CLEF_G_15_DOWN){
        // center h, 4 => 6, 4
        struct NotePitch centerPitch = {6, 4, 0, 0};
        center = TO_PITCH(centerPitch) + (int8_t)(G_CLEF - clef) * 7;
    }
    else if(CLEF_F_15_UP <= clef && clef <= CLEF_F_15_DOWN){
        // center d, 3 => 1, 3
        struct NotePitch centerPitch = {1, 3, 0, 0};
        center = TO_PITCH(centerPitch) + (int8_t)(F_CLEF - clef) * 7;
    }
    else{
        fprintf(stderr, "clef %i not implemented\n", clef);
        exit(1);
    }

    float p = pitch - center;
    // debugf("center %i[%i] - %f => %f\n", pitch, clef, center, p);

    return p;
}

enum Meshes getAccidental(struct Note *note){
    if(!GET_BIT(note->flags, NOTE_FLAG_ACCIDENTAL)){
        return MESH_NULL;
    }
    
    // debugf("flags %i\n", note->flags);
    if(GET_BIT(note->flags, NOTE_FLAG_SHARP)){
        return SHARP;
    }
    
    if(GET_BIT(note->flags, NOTE_FLAG_NATURAL)){
        return NATURAL;
    }
    
    if(GET_BIT(note->flags, NOTE_FLAG_FLAT)){
        return FLAT;
    }

    fprintf(stderr, "missing accidental value: '%i'\n");
    exit(1);
}

void calculateNotesSizes(Staff *staffs, StaffNumber staffSize, Division d, struct Attributes *currAttributes, float *rAccidentalMaxWidth, float *rNoteMaxWidht, float *rFlagMaxWidth){
    float accidentalMaxWidth = 0;
    float noteMaxWidht = 0;
    float flagMaxWidth = 0;

    for(StaffNumber s = 0; s < staffSize; s++){
        Staff staff = staffs[s];
        struct Notes *notes = staff[d];
        if(!notes){
            continue;
        }

        float min = FLT_MAX, max = FLT_MIN;
        struct Note **chord = notes->chord;
        for(ChordSize n = 0; n < notes->chordSize; n++){
            struct Note *note = chord[n];

            // get y note pos
            float y = (GET_BIT(note->flags, NOTE_FLAG_REST)) ? 0 : positionFromCenter(currAttributes->clefs[s], &note->pitch);
            note->y = y;

            // update note extreme
            min = MIN(min, y);
            max = MAX(max, y);

            if(note->noteType == NOTE_TYPE_NULL){
                note->noteType = noteDurationToType(note, currAttributes);
                // debugf("calculated note type: %i\n", note->noteType);
            }

            enum Meshes accidentalMeshId = getAccidental(note);
            if(accidentalMeshId != MESH_NULL){
                accidentalMaxWidth = MAX(accidentalMaxWidth, MBB_MAX(accidentalMeshId)[0]);
            }

            enum Meshes noteMeshId = (GET_BIT(note->flags, NOTE_FLAG_REST)) ? noteRest(note) : noteHead(note);
            noteMaxWidht = MAX(noteMaxWidht, MBB_MAX(noteMeshId)[0]);

            enum Meshes noteFlagMeshId = getAccidental(note);
            if(noteFlagMeshId != MESH_NULL){
                flagMaxWidth = MAX(flagMaxWidth, MBB_MAX(noteFlagMeshId)[0]);
            }
        }

        notes->minY = min;
        notes->maxY = max;
    }

    *rAccidentalMaxWidth = accidentalMaxWidth;
    *rNoteMaxWidht = noteMaxWidht;
    *rFlagMaxWidth = flagMaxWidth;
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
        // debugf("added %i %i %i\n", meshId, num[i - 1], TEXT_START);
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
            float x = *offset + max * i;
            float y = positionFromCenter(CLEF_G, &keySignatureArray[i]);
            struct Item *item = itemMeshInit(meshId, s, x, y); 
            ItemPVectorPush(itemsVector, item);
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

void computeNote(struct ItemPVector *itemVector, struct Note *note, StaffNumber staffIndex, float offset, float accidentalOffset){
    if(GET_BIT(note->flags, NOTE_FLAG_ACCIDENTAL)){
        enum Meshes meshId = getAccidental(note);
        // debugf("accidental meshid: %i\n", meshId);
        // enum Meshes meshId = NATURAL;
        struct Item *item = itemMeshInit(meshId, staffIndex, offset, note->y);
        note->item = item;
        ItemPVectorPush(itemVector, item);
    }

    enum Meshes meshId = (GET_BIT(note->flags, NOTE_FLAG_REST)) ? noteRest(note) : noteHead(note);
    struct Item *item = itemMeshInit(meshId, staffIndex, offset + accidentalOffset, note->y);
    item->typeSubGroup = ITEM_GROUP_NOTE_HEAD;
    note->item = item;
    ItemPVectorPush(itemVector, item);
}

// debugf("lower lines %i <%f>\n", lowerLines, diff);
#define ADD_NOTE_LINES(start, _diff, lineOffset){\
    int lowerLines = (int)(_diff / 2.f + 1);\
    for(int i = 0; i < lowerLines; i++){\
        /* start of the note + the half */ \
        float xNoteCenter = notes->x + width / 2.0f; \
        struct Item *item = itemLineInit(staffIndex, xNoteCenter, start + (float)i * lineOffset, lineWidth); \
        ItemPVectorPush(itemVector, item); \
    }\
}

void computeNoteLines(StaffNumber staffIndex, struct Notes *notes, struct ItemPVector *itemVector){
    // C
    float firstLowerHelpLine = positionFromCenter(CLEF_G, &NOTE_PITCH(0, 4));
    float firstUpperHelpLine = positionFromCenter(CLEF_G, &NOTE_PITCH(5, 5));

    struct Note **chord = notes->chord;

    // fprintf(stderr, "%p %i\n", chord, notes->chordSize | 1);
    float width = 0;
    for(ChordSize c = 0; c < notes->chordSize; c++){
        struct Note *note = chord[c];
        width = MAX(width, MBB_MAX(note->item->meshId)[0]);
    }

    // debugf("<%f %f> %f %f\n", min, max, firstLowerHelpLine, firstUpperHelpLine);
    // float positionFromCenter()
    float lineWidth = width * 1.5f;
    if(notes->minY <= firstLowerHelpLine){
        float diff = firstLowerHelpLine - notes->minY;
        ADD_NOTE_LINES(firstLowerHelpLine, diff, -2.0f);
    }
    
    if(firstUpperHelpLine <= notes->maxY){
        float diff = notes->maxY - firstUpperHelpLine;
        // // debugf("%f - %f = %f\n", max, firstUpperHelpLine, d);
        // int upperLines = (int)(d / 2.f + 1);
        // debugf("upper lines %i <%f>\n", upperLines, d);
        ADD_NOTE_LINES(firstUpperHelpLine, diff, 2.0f);
    }
}

void computeNotes(struct ItemPVector *itemVector, struct Notes *notes, StaffNumber staffIndex, float offset, float accidentalOffset){
    for(ChordSize c = 0; c < notes->chordSize; c++){
        struct Note *note = notes->chord[c];
        computeNote(itemVector, note, staffIndex, offset, accidentalOffset);
        notes->width = MAX(notes->width, MBB_MAX(note->item->meshId)[0]);
    }

    struct Note *note = notes->chord[0];

    if(GET_BIT(note->flags, NOTE_FLAG_REST) == 0){
        computeNoteLines(staffIndex, notes, itemVector);
    }

    // debugf("beams: %i\n", notes->beams);
    if(GET_BIT(note->flags, NOTE_FLAG_REST) == 0 && notes->beams == 0 && NOTE_TYPE_HALF <= note->noteType){
        if(note->noteType < NOTE_TYPE_EIGHTH){
            return;
        }

        float closestBeamHeight;
        bool side;
        steamPositions(&closestBeamHeight, &side, notes->minY, notes->maxY);
        
        float sign = (side == BEAM_UPPER_SIDE) ? 1 : -1;
        float beamYPosition = closestBeamHeight + BEAM_NOTE_HEAD_OFFSET * sign;

        // TODO: chords with dirrerent notes durations
        addBeamSteam(notes, staffIndex, itemVector, beamYPosition);

        enum Meshes meshId = noteFlag(note);
        if(FLAG1 <= meshId && meshId <= FLAG5){
            bool invert = (side == BEAM_UPWARDS);
            // struct Item *item = itemMeshInit(meshId, staffIndex, offset + accidentalOffset, beamYPosition);
            struct Item *item = itemFlagInit(meshId, staffIndex, offset + accidentalOffset, beamYPosition, notes->width, invert);
            ItemPVectorPush(itemVector, item);
        }
    }
}

void notesUpdateExtremes(struct Notes *notes, enum Clef clef, float *rMin, float *rMax){
    float min = *rMin, max = *rMax;
    for(ChordSize i = 0; i < notes->chordSize; i++){
        float y = positionFromCenter(clef, &notes->chord[i]->pitch);
        min = MIN(min, y);
        max = MAX(max, y);
        // debugf("%f => min %f max %f\n", y, min, max);
    }

    *rMin = min;
    *rMax = max;
}

float computeMeasure(struct Piano *piano, size_t measureIndex, struct ItemPVector *itemVector, struct Attributes *currAttributes, NotePositionExtreme *staffsPositionNoteExtremes){
    struct Measure *measure = piano->measures[measureIndex];
    // printf("\n\n");
    // debugf("COMPUTING MEASURE %zu\n", measureIndex);
    // printMeasure(measure);

    StaffNumber staffNumber = piano->measures[0]->attributes[0]->stavesNumber;
    // StaffNumber staffNumber = piano->sheet->staffNumber;
    float offset = 0;

    MeasureSize measureSize = measure->measureSize;
    
    float notesPositions[measureSize];
    memset(notesPositions, 0, measureSize * sizeof(float));

    size_t itemVectorStart = itemVector->size;
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

        // calculate all the widths that the note needs
        // + add a correct note type if it only has a duration
        float accidentalMaxWidth = 0;
        float noteMaxWidht = 0;
        float flagMaxWidth = 0;
        calculateNotesSizes(measure->staffs, staffNumber, d, currAttributes, &accidentalMaxWidth, &noteMaxWidht, &flagMaxWidth);

        // for every staff
        for(StaffNumber s = 0; s < staffNumber; s++){
            Staff staff = measure->staffs[s];
            struct Notes *notes = staff[d];
            if(notes){
                notes->x = offset + accidentalMaxWidth;
                computeNotes(itemVector, notes, s, offset, accidentalMaxWidth);

                staffsPositionNoteExtremes[s][0] = MIN(staffsPositionNoteExtremes[s][0], notes->minY);
                staffsPositionNoteExtremes[s][1] = MAX(staffsPositionNoteExtremes[s][1], notes->maxY);
            }
        }

        noteMaxWidht = MAX(noteMaxWidht, MBB_MAX(QUATER_HEAD)[0]);

        notesPositions[d] = offset + accidentalMaxWidth;
        offset += accidentalMaxWidth + noteMaxWidht + flagMaxWidth;
    }


    // add beams for every staff
    for(StaffNumber s = 0; s < staffNumber; s++){
        computeBeams(measure, s, itemVector);
    }
    
    measure->boundingBoxes = malloc(sizeof(struct MeshBoundingBox) * measure->stavesNumber);
    for(StaffNumber s = 0; s < measure->stavesNumber; s++){
        struct MeshBoundingBox *boundingBox = &measure->boundingBoxes[s];
        meshBoundingBoxClear(boundingBox);
    }

    for(size_t itemIndex = itemVectorStart; itemIndex < itemVector->size; itemIndex++){
        struct Item *item = itemVector->data[itemIndex];
        if(item->type != ITEM_MESH){
            continue;
        }
        // debugf("staffIndex: %i\n", item->staffIndex);
        // meshBoundingBoxUpdate(boundingBox, &item->boundingBox);
        struct ItemMesh *itemMesh = ((struct ItemMesh*)item->data);
        meshBoundingBoxUpdate(&measure->boundingBoxes[item->staffIndex], &meshBoundingBoxes[item->meshId], (vec3){itemMesh->xPosition, itemMesh->yPosition, 0});
    }

    // meshBoundingBoxPrint(boundingBox);

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
    NotePositionExtreme *staffsPositionNoteExtremes = calloc(staffNumber, sizeof(NotePositionExtreme));
    debugf("staff number: %i\n", staffNumber);

    struct PitchExtreme *staffsPitchExtreme = calloc(staffNumber, sizeof(struct PitchExtreme));
    for(StaffNumber s = 0; s < staffNumber; s++){
        struct PitchExtreme *pitchExtreme = &staffsPitchExtreme[s];
        pitchExtreme->min = PITCH_MAX;
        pitchExtreme->max = PITCH_MIN;
    }

    for(size_t i = 0; i < piano->measureSize; i++){
    // for(size_t i = 0; i < 1; i++){
        float offset = computeMeasure(piano, i, itemVector, &currAttributes, staffsPositionNoteExtremes);

        for(StaffNumber s = 0; s < staffNumber; s++){
            struct Measure *measure = piano->measures[i];
            staffsPitchExtreme[s].min = MIN(staffsPitchExtreme[s].min, measure->pitchExtreme->min);
            staffsPitchExtreme[s].max = MAX(staffsPitchExtreme[s].max, measure->pitchExtreme->max);
        }

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
        float c1 = MAX(staffsPositionNoteExtremes[s - 1][0], 4.5f + 4.0f + BEAM_NOTE_HEAD_OFFSET);
        float c2 = MAX(staffsPositionNoteExtremes[s][1], 4.5f + 4.0f + BEAM_NOTE_HEAD_OFFSET);
        staffOffsets[s] = staffOffsets[s - 1] + (c1 + c2);
        // debugf("staff %i offset %f %f => %f\n", s, c1, c2, staffOffsets[s]);
    }

    struct Sheet *sheet = calloc(1, sizeof(struct Sheet));
    sheet->measures = ItemMeasurePVectorDuplicate(itemMeasureV, &sheet->measuresSize);
    sheet->staffNumber = staffNumber;
    sheet->staffOffsets = staffOffsets;

    sheet->staffsPitchExtreme = staffsPitchExtreme;
    for(StaffNumber s = 0; s < staffNumber; s++){
        struct PitchExtreme *pitchExtreme = &staffsPitchExtreme[s];
        debugf("%i => [%i:%i]\n", s, pitchExtreme->min, pitchExtreme->max);
    }

    ItemMeasurePVectorFree(itemMeasureV);
    
    struct MeshBoundingBox *sheetBoundingBox = &piano->boundingBox;
    meshBoundingBoxClear(sheetBoundingBox);
    // update every staff's boundingBox in measure
    for(MeasureSize m = 0; m < piano->measureSize; m++){
        struct Measure *measure = piano->measures[m];
        meshBoundingBoxClear(&measure->boundingBox);
        for(StaffNumber s = 0; s < staffNumber; s++){
            meshBoundingBoxUpdate(&measure->boundingBox, &measure->boundingBoxes[s], (vec3){0, -staffOffsets[s], 0});
            // staffOffsets[s]
        }

        meshBoundingBoxUpdate(sheetBoundingBox, &measure->boundingBox, (vec3){});
    }
    
    float height = sheetBoundingBox->max[1] - sheetBoundingBox->min[1];
    debugf("sheet height: %f\n", height);
    sheet->height = height;

    piano->sheet = sheet;
}
