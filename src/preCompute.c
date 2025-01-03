#include "preCompute.h"

// TODO: smart measure division max size

void adjustStaffClef(struct Attributes *currAtrributes, struct Measure *measure, StaffNumber staffIndex){
    // if measure has clef change inside it, abord adjustment
    for(Division d = 1; d < measure->measureSize; d++){
        if(measure->attributes[staffIndex]->clefs != NULL){
            return;
        }
    }

    enum Clef *clef = currAtrributes->clefs; 

    // for(Division d = 0; note->min
    // float y = (GET_BIT(note->flags, NOTE_FLAG_REST)) ? 0 : positionFromCenter(currAttributes->clefs[s], &note->pitch);
    // measure->pitchExtreme[staffIndex].max;
    // if(measure->stavesNumber
}

void preCompute(struct Piano *piano){
    struct Attributes currAttributes = {};
    updateAttributes(piano->measure->attributes, &currAttributes);

    for(size_t m = 0; m < piano->measureSize; m++){
        struct Measure *measure = piano->measures[m];
        updateAttributes(measure->attributes, &currAttributes);
        for(StaffNumber s = 0; s < measure->stavesNumber; s++){
            // adjustStaffClef();
        }
    }    
}
