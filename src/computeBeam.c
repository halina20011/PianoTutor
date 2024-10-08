#include "compute.h"

extern struct MeshBoundingBox *meshBoundingBoxes;

void notesUpdateBeamDepth(struct Notes *notes, uint8_t *maxBeamDepth){
    uint8_t beam = *maxBeamDepth;
    while(GET_BEAM(notes->beams, beam)){
        beam++;
    }

    *maxBeamDepth = beam;
}

void printBeams(struct BeamPosition *beams, size_t beamsSize){
    DEBUG_CHECK();

    // debugf("beams on depth %i: ", d);
    for(Division i = 0; i < beamsSize; i++){
        struct BeamPosition *b = &beams[i];
        printf("[%i:%i] ", b->left, b->right);
    }
    printf("\n");
}

void printBeamArray(Staff staff, Division size, uint8_t beamDepth){
    if(!debugPrintGetStatus()){
        return;
    }

    debugf("beam array: \n", size);
    for(Division d = 0; d < beamDepth; d++){
        printf("%i:", d);
        for(Division i = 0; i < size; i++){
            struct Notes *firstNote = staff[i];
            Beam beam = (!firstNote) ? 0 : GET_BEAM(firstNote->beams, beamDepth);
            // printf("b:%i\n", beam);
            printf("%i", beam != 0);
        }

        printf("\n");
    }
}

void findBeams(Staff staff, Division size, struct BeamPosition *beams, uint8_t beamDepth, Division *rBeamsSize){
    Division beamsSize = 0;
    for(Division i = 0; i < size; ){
        struct Notes *firstNote = staff[i];
        if(!firstNote || !GET_BEAM(firstNote->beams, beamDepth)){
            i++;
            continue;
        }
        Beam firstNoteBeam = GET_BEAM(firstNote->beams, beamDepth);
        debugf("division[%i] beam[%i]: %i\n", i, beamDepth, firstNoteBeam);

        Division right = i;
        debugf("right %i\n", right);
        
        // find next note that has beam flag off
        while(right <= size){
            if(right == size){
                fprintf(stderr, "right is equal to size\n");
                exit(1);
            }
            struct Notes *notes = staff[right];
            if(notes){
                Beam beam = GET_BEAM(notes->beams, beamDepth);
                debugf("note[%i] beams %i beam %i\n", right, notes->beams, beam);
                if(beam && GET_BIT(beam, BEAM_OFF)){
                    break;
                }
            }
            right++;
        }
        
        debugf("beam %i --- %i\n", i, right);
        beams[beamsSize].left = i;
        beams[beamsSize].right = right;
        beams[beamsSize].firstBeam = firstNoteBeam;
        beamsSize++;
        i = right + 1;
    }

    *rBeamsSize = beamsSize;
}

void steamPositions(float *rClosestBeamHeight, bool *rSide, float min, float max){
    // if the min and max are on the same side then use the unused side
    // else take the max value of them and use the smallest
    bool negativeMin = (min < 0);
    bool positiveMax = (0 <= max);
    bool diffHalfs = (negativeMin && positiveMax);
    
    float closestBeamHeight;
    bool side = 0;

    if(diffHalfs){
        // take the smallest extreme 
        side = (fabsf(min) < max) ? BEAM_LOWER_SIDE : BEAM_UPPER_SIDE;
        closestBeamHeight = (side == BEAM_LOWER_SIDE) ? min : max;
    }
    else{
        side = (max < 0) ? BEAM_UPPER_SIDE : BEAM_LOWER_SIDE;
        closestBeamHeight = (max < 0) ? max : min;
    }

    *rClosestBeamHeight = closestBeamHeight;
    *rSide = side;
}

void addBeamSteam(struct Notes *notes, StaffNumber staffIndex, struct ItemPVector *itemVector, float yBeamPos){
    float minLenght = fabsf(yBeamPos - notes->minY);
    float maxLenght = fabsf(yBeamPos - notes->maxY);

    float yNotePos = (minLenght < maxLenght) ? notes->maxY : notes->minY;
    
    float noteWidthOffset = (0 < yBeamPos) ? notes->width : 0;

    struct Item *item = itemStemInit(staffIndex, notes->x, noteWidthOffset, yNotePos, yBeamPos);
    ItemPVectorPush(itemVector, item);
}

// --------
// ---- ---
// - -  
void addBeam(Staff staff, StaffNumber staffIndex, struct ItemPVector *itemVector, struct BeamPosition *bP, float y){
    debugf("staffIndex: %i\n", staffIndex);
    Division left = bP->left;
    Division right = bP->right;

    float leftPos, rightPos, leftNoteWidth, rightNoteWidth;
    leftPos = staff[left]->x;
    leftNoteWidth = staff[left]->width;
    
    // debugf("right: %i\n", right);
    // if the beam is a hook then calculate the beam start/end
    if(GET_BIT(bP->firstBeam, BEAM_HOOK_ENABLED)){
        // find the closest note
        int offset = (GET_BIT(bP->firstBeam, BEAM_HOOK_BACKWARD)) ? -1 : 1;
        Division n = bP->left + offset;
        while(staff[n] == NULL){
            n += offset;
        }

        rightPos = staff[n]->x;
        rightNoteWidth = staff[n]->width;
        // debugf("hook %i <%i> --> %i\n", bP->left, 
        float distance = rightPos - leftPos;
        rightPos = leftPos + distance / 2;
    }
    else{
        debugf("staff: %p r: %i\n", staff, right);
        rightPos = staff[right]->x;
        rightNoteWidth = staff[right]->width;
    }

    // if the beam is downwards reset the note widht offset
    if(y < 0){
        leftNoteWidth = 0;
        rightNoteWidth = 0;
    }

    struct Item *itemBeam = itemBeamInit(staffIndex, leftPos, leftNoteWidth, rightPos, rightNoteWidth, y);
    ItemPVectorPush(itemVector, itemBeam);
}

void computeBeams(struct Measure *measure, StaffNumber staffIndex, struct ItemPVector *itemVector){
    debugf("compute beams measure %zu staff %i\n", measure->sheetMeasureIndex - 1, staffIndex);
    Staff staff = measure->staffs[staffIndex];
    
    uint8_t maxBeamDepth = 0;

    Division measureSize = measure->measureSize;

    for(Division i = 0; i < measureSize; i++){
        struct Notes *notes = staff[i];
        if(notes && notes->beams){
            notesUpdateBeamDepth(notes, &maxBeamDepth);
        }
    }

    debugf("max beam depth: %i\n", maxBeamDepth);

    if(!maxBeamDepth){
        return;
    }

    // and array of all the beams found in the beamHeight
    struct BeamPosition beams[measureSize];
    memset(beams, 0, sizeof(struct BeamPosition) * measureSize);

    // beamsInfo is a hash map that will hold the initial beam info of the root beam
    struct BeamInfo beamsInfo[measureSize];
    memset(beamsInfo, 0, sizeof(struct BeamInfo) * measureSize);

    printBeamArray(staff, measure->measureSize, maxBeamDepth);
    
    // find all the beam roots and calculate all needed things
    //  - y pos of the beam
    //  - offset of nth beam depth
    //  - TODO: calculate slope of the beam
    Division beamsSize = 0;
    findBeams(staff, measureSize, beams, 0, &beamsSize);
    

    for(Division b = 0; b < beamsSize; b++){
        struct BeamPosition *bP = &beams[b];
        
        Division left = bP->left;
        Division right = bP->right;

        float min = FLT_MAX, max = FLT_MIN;
        uint8_t localMaxBeamDepth = 0;
        for(Division n = left; n <= right; n++){
            struct Notes *notes = staff[n];
            if(notes && notes->beams){
                min = MIN(min, notes->minY);
                max = MAX(max, notes->maxY);
                notesUpdateBeamDepth(notes, &localMaxBeamDepth);
            }
        }

        float closestBeamHeight;
        bool side;
        steamPositions(&closestBeamHeight, &side, min, max);

        float beamsHeight = BEAM_HEIGHT * localMaxBeamDepth;
        float beamsOffset = BEAM_DISTANCE * (localMaxBeamDepth - 1);
        float totalBeamHeight = BEAM_NOTE_HEAD_OFFSET + beamsHeight + beamsOffset;

        float sign = (side == BEAM_UPPER_SIDE) ? 1 : -1;

        float beamYPosition = closestBeamHeight + totalBeamHeight * sign;

        // beam that is bellow the center must go upwards => the above has to go downwards
        float beamOffset = (side == BEAM_UPPER_SIDE) ? -BEAM_OFFSET : BEAM_OFFSET;

        // debugf("beam %i +---+ %i extrems: <%f %f> %f <%f>\n", left, right, min, max, beamYPosition, beamOffset);

        for(Division d = left; d <= right; d++){
            struct Notes *notes = staff[d];
            if(!notes){
                continue;
            }

            struct BeamInfo *bI = &beamsInfo[d];
            bI->yPos = beamYPosition;
            bI->offset = beamOffset;
            
            addBeamSteam(notes, staffIndex, itemVector, beamYPosition);
        }
    }
    
    // loop throw each beam depth and add the beam lines
    for(uint8_t d = 0; d < maxBeamDepth; d++){
        debugf("depth %i\n", d);
        findBeams(staff, measureSize, beams, d, &beamsSize);
        printBeams(beams, beamsSize);

        for(Division b = 0; b < beamsSize; b++){
            struct BeamPosition *bP = &beams[b];
            Division left = bP->left;
            struct BeamInfo *bI = &beamsInfo[left];

            // debugf("adding beam on d %i y pos %f <%f>\n", left, bI->yPos, bI->offset * d);
            addBeam(staff, staffIndex, itemVector, bP, bI->yPos + d * bI->offset);
        }
    }
}
