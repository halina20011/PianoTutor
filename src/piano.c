#include "piano.h"

#include "interface.h"
extern struct Interface *interface;

extern GLuint elementArrayBuffer;

extern GLint globalMatUniform;
extern GLint localMatUniform;
extern GLint viewMatUniform;
extern GLint colorUniform;

struct MeshBoundingBox *meshBoundingBoxes;

VECTOR_TYPE_FUNCTIONS(struct Item*, ItemPVector, "%p");
VECTOR_TYPE_FUNCTIONS(struct Item**, ItemPPVector, "%p");
VECTOR_TYPE_FUNCTIONS(struct ItemMeasure*, ItemMeasurePVector, "%p");

VECTOR_TYPE_FUNCTIONS(struct PlayedNote*, PlayedNotePVector, "%p");

struct Piano *pianoInit(struct Measure **measures, size_t measureSize, bool hideKeyboard, bool hideNotes){
    graphicsInit();
    struct Piano *piano = calloc(1, sizeof(struct Piano));
    interface->piano = piano;
    loadPianoMeshes(piano);

    piano->measures = measures;
    piano->measureSize = measureSize;

    piano->midiDevice = -1;
    piano->midiDevice = midiDeviceInit("auto");
    // if(piano->midiDevice == -1){
    //     exit(1);
    // }

    piano->playedNotesVector = PlayedNotePVectorInit();
    piano->pressedNotesVector = PressedNoteVectorInit();

    piano->keyboard.keysDataStart = malloc(sizeof(size_t) * KEYBOARD_KEY_SIZE);
    
    viewInit(&piano->view, 3);

    // TODO: --hide-keyboard
    // TODO: --hide-notes 
    float sheetHeight = 0.6f;
    // float keyboardHeigh = 
    // if(hideKeyboard && hideNotes){
    //
    // }

    viewSet(&piano->view, VIEW_ITEM_TYPE_SHEET, sheetHeight);
    // viewSet for keyboard and notes is set in computeKeyboard
    
    return piano;
}

char noteNames[][3] = {
    "C", "C#", 
    "D", "D#", 
    "E",
    "F", "F#", 
    "G", "G#", 
    "A", "A#",
    "H",
};

void printNote(Pitch pitch){
    printf("note %i [%i:%s]\n", pitch, NOTE_OCTAVE(pitch), noteNames[pitch % 12]);   
}

void pressNote(struct Piano *piano, struct Note *note, Division divisionCounter){
    Pitch pitch = notePitchToPitch(&note->pitch);
    debugf("played ");
    printNote(pitch);
    if(piano->pianoPlay->pianoMode == PIANO_MODE_PLAY){
        sendNoteEvent(piano->midiDevice, NOTE_ON, &note->pitch, 100);
    }

    piano->playedNotes[notePitchToPitch(&note->pitch)] = true;

    struct PlayedNote *pressedNote = malloc(sizeof(struct PlayedNote));
    pressedNote->note = note;
    pressedNote->startDivision = divisionCounter;
    pressedNote->endDivision = divisionCounter + note->duration;
    // debugf("adding note %p at %zu\n", pressedNote, pressedNotes->size - 1);
    PlayedNotePVectorPush(piano->playedNotesVector, pressedNote);
    // interface->paused = true;
}

void unpressNote(struct Piano *piano, size_t i){
    struct PlayedNote *pressedNote = piano->playedNotesVector->data[i];
    
    if(piano->pianoPlay->pianoMode == PIANO_MODE_PLAY){
        sendNoteEvent(piano->midiDevice, NOTE_OFF, &pressedNote->note->pitch, 0);
    }

    piano->playedNotes[notePitchToPitch(&pressedNote->note->pitch)] = false;

    // debugf("removing %zu note %p\n", i, pressedNote);
    if(piano->playedNotesVector->size - 1 != i){
        struct PlayedNote *lastNote = piano->playedNotesVector->data[piano->playedNotesVector->size - 1], *temp = NULL;
        // debugf("swap {%p} %p\n", pressedNote, lastNote);
        temp = pressedNote;
        pressedNote = lastNote;
        lastNote = temp;
        // debugf("free %p\n", lastNote);
        free(lastNote);
        piano->playedNotesVector->data[i] = pressedNote;
    }

    piano->playedNotesVector->size--;
    // interface->paused = true;
}

void pianoPlayInit(struct Piano *piano, enum PianoMode pianoMode){
    struct PianoPlay *pianoPlay = calloc(1, sizeof(struct PianoPlay));

    // glfwSwapInterval(0);
    struct Measure **measures = piano->measures;
    updateAttributes(measures[0]->attributes[0], &pianoPlay->currAttributes);

    pianoPlay->speedScale = 1.0f;
    pianoPlay->currBmp = 120;
    piano->pianoPlay = pianoPlay;

    pianoPlay->pianoMode = pianoMode;

    pianoPlay->pause = 1;
    // turnNotes(piano);
}

void pressedNotes(struct Piano *piano){
    debugf("");
    printf("pressed notes: [");
    for(size_t i = 0; i < piano->pressedNotesVector->size; i++){
        struct PressedNote *pressedNote = &piano->pressedNotesVector->data[i];
        printf("%i, ", pressedNote->pitch);
    }
    printf("]\n");
}

void pianoPlayCalculateError(struct Piano *piano){
    if(0 < piano->pianoPlay->pause){
        return;
    }

    // if(piano->pressedNotesVector->size != 0){
    //     pressedNotes(piano);
    // }

    for(size_t n = 0; n < piano->playedNotesVector->size; n++){
        struct PlayedNote *playedNote = piano->playedNotesVector->data[n];
        Pitch pitch = notePitchToPitch(&playedNote->note->pitch);
        // check if the played note is being pressed
        if(!piano->pressedNotes[pitch]){
            // debugf("note %i [%i:%s] should have been pressed\n", pitch, NOTE_OCTAVE(pitch), noteNames[pitch % 12]);   
            // double p = piano->pianoPlay->percentage;
            double notePercentage = (double)(piano->pianoPlay->divisionCounter - playedNote->startDivision) / (double)(playedNote->note->duration) + piano->pianoPlay->percentage * (1.0f / playedNote->note->duration);
            // printf("%f\n", p);
            // printf("%f\n", notePercentage);
            // double error = -4 * p * (1.0 - p);
            double error = notePercentage * notePercentage;
            double prevError = piano->pressedNotesPrevError[pitch];
            double errorAlpha = error - prevError;
            piano->pressedNotesError[pitch] = error;
            piano->pressedNotesPrevError[pitch] = error;
            // piano->error += errorAlpha / piano->playedNotesVector->size;
            piano->error += errorAlpha;
        }
    }
    
    // debugf("error %f\n", piano->error);
}

void pianoRewind(struct Piano *piano){
    struct PianoPlay *pianoPlay = piano->pianoPlay;

    piano->pianoPlay->pause = 5;
    piano->error = 0;

    pianoPlay->divisionTimer = 0;
    pianoPlay->currDivision = 0;
    pianoPlay->divisionCounter = pianoPlay->measureDivisionStart;

    piano->playedNotesVector->size = 0;

    for(uint8_t i = 0; i < NOTE_PRESS_BUFFER_SIZE; i++){
        piano->pressedNotesError[i] = 0;
        piano->pressedNotesPrevError[i] = 0;
    }
}

void turnNotes(struct Piano *piano){
    struct PianoPlay *pianoPlay = piano->pianoPlay;
    struct Measure *currMeasure = piano->measures[pianoPlay->measureIndex];
    
    for(StaffNumber s = 0; s < pianoPlay->currAttributes.stavesNumber; s++){
        Staff staff = currMeasure->staffs[s];
        struct Notes *notes = staff[pianoPlay->currDivision];
        if(!notes){
            continue;
        }

        for(ChordSize c = 0; c < notes->chordSize; c++){
            struct Note *note = notes->chord[c];
            
            if(!GET_BIT(note->flags, NOTE_FLAG_REST)){
                pressNote(piano, note, pianoPlay->divisionCounter);
            }
        }
    }
}

bool pianoPlayUpdate(struct Piano *piano){
    struct Measure **measures = piano->measures;

    struct PianoPlay *pianoPlay = piano->pianoPlay;

    struct Measure *currMeasure = measures[pianoPlay->measureIndex];
    double time = glfwGetTime();
    double alpha = (time - pianoPlay->prevTime) * pianoPlay->speedScale;
    pianoPlay->prevTime = time;

    if(interface->paused){
        if(interface->key == 'N'){
            alpha = 1.0f / 60.0f;
            interface->key = 0;
        }
        else{
            pianoPlay->prevTime = glfwGetTime();
            return false;
        }
    }

    // debugf("measure index: %zu\n", pianoPlay->measureIndex);

    // debugf("alpha: %f\n", alpha);

    double beat = 60.0 / pianoPlay->currBmp;
    double divisionDuration = (beat * 4.0) / (float)currMeasure->measureSize;
    // debugf("dur %f\n", divisionDuration);
    
    // calculate "fps"
    pianoPlay->fps++;
    if(pianoPlay->currTimerLevel < (int)time){
        pianoPlay->currTimerLevel++;
        // debugf("fps: %i\n", pianoPlay->fps);
        pianoPlay->fps = 0;
    }

    // debugf("time %f\n", time);
    pianoPlay->divisionTimer += alpha;

    // pianoPlay->percentage = pianoPlay->currDivision + (pianoPlay->divisionTimer / divisionDuration);
    pianoPlay->percentage = (pianoPlay->divisionTimer / divisionDuration);

    // debugf("%f\n", pianoPlay->percentage * 100);
    // debugf("division timer: %f percentage: %f\n", pianoPlay->divisionTimer, pianoPlay->percentage);

    if(pianoPlay->divisionTimer < divisionDuration){
        return true;
    }

    pianoPlay->divisionTimer = 0;
    pianoPlay->percentage = 0;

    if(pianoPlay->pause){
        pianoPlay->pause--;
        if(pianoPlay->pause != 0){
            return false;
        }
        
        // pianoPlay->percentage = 0;
        turnNotes(piano);
        return false;
    }

    // the prev division already passed so increase the index
    pianoPlay->currDivision++;
    pianoPlay->divisionCounter++;

    if(currMeasure->measureSize <= pianoPlay->currDivision){
        pianoPlay->measureIndex++;
        currMeasure = measures[pianoPlay->measureIndex];
        pianoPlay->currDivision = 0;
        pianoPlay->measureDivisionStart = pianoPlay->divisionCounter;
        // piano->sheet->currMeasure = pianoPlay->measureIndex;
    }

    // debugf("size: %zu\n", pressedNotes->size);
    // debugf("position: %zu %i %f\n", measureIndex, currDivision, divisionDuration);

    // turn off any notes
    for(size_t i = 0; i < piano->playedNotesVector->size; ){
        struct PlayedNote *pressedNote = piano->playedNotesVector->data[i];
        if(pressedNote->endDivision <= piano->pianoPlay->divisionCounter){
            Pitch pitch = notePitchToPitch(&pressedNote->note->pitch);
            piano->error -= piano->pressedNotesError[pitch];
            // debugf("error -= %f => %f\n", piano->pressedNotesError[pitch], piano->error);
            piano->pressedNotesError[pitch] = 0;
            piano->pressedNotesPrevError[pitch] = 0;
            unpressNote(piano, i);
        }
        else{
            i++;
        }
    }

    turnNotes(piano);

    return true;
}

void pianoPlaySong(struct Piano *piano){
    pianoPlayInit(piano, PIANO_MODE_PLAY);

    struct PianoPlay *pianoPlay = piano->pianoPlay;

    pianoPlay->speedScale = 0.4f;

    while(!glfwWindowShouldClose(interface->g->window) && pianoPlay->measureIndex < piano->measureSize){
        draw(piano, KEYBOARD_SHEET_MODE);

        if(!pianoPlayUpdate(piano)){
            continue;
        }
    }
}

void pianoLearnSong(struct Piano *piano){
    pianoPlayInit(piano, PIANO_MODE_LEARN);

    struct PianoPlay *pianoPlay = piano->pianoPlay;

    pianoPlay->speedScale = 0.05;
    
    while(!glfwWindowShouldClose(interface->g->window) && pianoPlay->measureIndex < piano->measureSize){
        draw(piano, KEYBOARD_PIANO_MODE);
        
        if(!pianoPlayUpdate(piano)){
            continue;
        }
        
        midiRead(piano);
        pianoPlayCalculateError(piano);

        // if(0.5 < piano->error){
        //     pianoRewind(piano);
        // } 
        // printf("%f\n", piano->error);
    }
}
