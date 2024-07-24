#include "compute.h"


void notesUpdateBeamDepth(struct Notes *notes, uint8_t *maxBeamDepth){
    uint8_t beam = *maxBeamDepth;
    while(GET_BEAM(notes->beams, beam)){
        beam++;
    }

    *maxBeamDepth = beam;
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

        Division right = i;
        // debugf("right %i\n", right);
        while(right < size){
            struct Notes *notes = staff[right];
            if(notes){
                Beam beam = GET_BEAM(notes->beams, beamDepth);
                // debugf("note[%i] beams %i beam %i\n", right, notes->beams, beam);
                if(beam && GET_BIT(beam, BEAM_OFF)){
                    break;
                }
            }
            right++;
        }
        
        // debugf("beam %i --- %i\n", i, right);
        beams[beamsSize].left = i;
        beams[beamsSize].right = right;
        beams[beamsSize].firstBeam = firstNoteBeam;
        beamsSize++;
        i = right + 1;
    }

    *rBeamsSize = beamsSize;
}

void beamXPosition(float *notesPositions, Division pos, float y, float *rPos){
    float offset = 0;

    // if the y is above the center then the note steam will start on the right side
    if(0 < y){
        offset += 1.0f;
        // struct Note *note = (notes->chordSize) ? notes->chord[0] : notes->note;
        // return MBB_MAX(note->item->meshId)[0];
        // offset1 = noteWidth(staff[left]);
        // offset2 = noteWidth(staff[end]);
        // offset2 = MBB_MAX(QUATER_HEAD)[0];
        // debugf("offset %f\n", offset);
    }

    *rPos = notesPositions[pos] + offset;
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

void addBeamSteam(struct Notes *notes, enum Clef clef, StaffNumber staffIndex, struct ItemPVector *itemVector, float beamXPos, float beamYpos){
    float min = FLT_MAX, max = FLT_MIN;
    notesUpdateExtremes(notes, clef, &min, &max);

    float minLenght = fabsf(beamYpos - min);
    float maxLenght = fabsf(beamYpos - max);

    float yNotePos = (minLenght < maxLenght) ? max : min;
    
    struct Item *item = itemStreamInit(staffIndex, beamXPos, yNotePos, beamYpos);
    ItemPVectorPush(itemVector, item);
}

// --------
// ---- ---
// - -  
void addBeam(Staff staff, StaffNumber staffIndex, struct ItemPVector *itemVector, float *notesPositions, struct BeamPosition *bP, float y){
    Division left = bP->left;
    Division right = bP->right;

    float leftPos, rightPos;
    beamXPosition(notesPositions, left, y, &leftPos);
    
    if(GET_BIT(bP->firstBeam, BEAM_HOOK_ENABLED)){
        int8_t offset = (GET_BIT(bP->firstBeam, BEAM_HOOK_BACKWARD)) ? -1 : 1;
        Division n = bP->left + offset;
        while(staff[n] == NULL){
            n += offset;
        }
        
        // debugf("hook %i <%i> --> %i\n", bP->left, 
        beamXPosition(notesPositions, n, y, &rightPos);
        float distance = rightPos - leftPos;
        rightPos = leftPos + distance / 2;
    }
    else{
        beamXPosition(notesPositions, right, y, &rightPos);
    }

    struct Item *itemSteam = itemBeamInit(staffIndex, leftPos, rightPos, y);
    ItemPVectorPush(itemVector, itemSteam);
}

void computeBeams(struct Measure *measure, StaffNumber staffIndex, struct Attributes *currAttributes, struct ItemPVector *itemVector, float *notesPositions){
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
            enum Clef clef = currAttributes->clefs[staffIndex];
            if(notes && notes->beams){
                notesUpdateExtremes(notes, clef, &min, &max);
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

        debugf("beam %i +---+ %i extrems: <%f %f> %f <%f>\n", left, right, min, max, beamYPosition, beamOffset);

        for(Division d = left; d <= right; d++){
            struct Notes *notes = staff[d];
            if(!notes){
                continue;
            }

            struct BeamInfo *bI = &beamsInfo[d];
            bI->yPos = beamYPosition;
            bI->offset = beamOffset;
            // bI->firstBeam = firstBeam;
            
            enum Clef clef = currAttributes->clefs[staffIndex];
            float xBeamPosition = 0;
            beamXPosition(notesPositions, d, beamYPosition, &xBeamPosition);
            addBeamSteam(notes, clef, staffIndex, itemVector, xBeamPosition, beamYPosition);
        }
    }
    
    // loop throw eatch beam depth and add the beam lines
    for(uint8_t d = 0; d < maxBeamDepth; d++){
        debugf("depth %i\n", d);
        findBeams(staff, measureSize, beams, d, &beamsSize);
        debugf("beams on depth %i: ", d);
        for(Division i = 0; i < beamsSize; i++){
            struct BeamPosition *b = &beams[i];
            printf("[%i:%i] ", b->left, b->right);
        }
        printf("\n");

        for(Division b = 0; b < beamsSize; b++){
            struct BeamPosition *bP = &beams[b];
            Division left = bP->left;
            struct BeamInfo *bI = &beamsInfo[left];

            debugf("adding beam on d %i y pos %f <%f>\n", left, bI->yPos, bI->offset * d);
            addBeam(staff, staffIndex, itemVector, notesPositions, bP, bI->yPos + d * bI->offset);
        }
    }
}
