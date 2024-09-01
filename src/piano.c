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

void pressNote(struct Piano *piano, struct Note *note, Division divisionCounter){
    sendNoteEvent(piano->midiDevice, NOTE_ON, &note->pitch, 100);
    
    piano->playedNotes[notePitchToPitch(&note->pitch)] = true;

    struct PlayedNote *pressedNote = malloc(sizeof(struct PlayedNote));
    pressedNote->note = note;
    pressedNote->endDivision = note->duration + divisionCounter;
    // debugf("adding note %p at %zu\n", pressedNote, pressedNotes->size - 1);
    PlayedNotePVectorPush(piano->playedNotesVector, pressedNote);
}

void unpressNote(struct Piano *piano, size_t i){
    struct PlayedNote *pressedNote = piano->playedNotesVector->data[i];
    sendNoteEvent(piano->midiDevice, NOTE_OFF, &pressedNote->note->pitch, 0);

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
}

void pianoPlaySong(struct Piano *piano){
    struct Measure **measures = piano->measures;
    size_t measureIndex = 0;
    Division currDivision = -1;
    double currBmp = 106;
    double prevTime = 0;
    struct Attributes currAttributes = {};
    double divisionTimer = 0;

    glfwSwapInterval(0);
    updateAttributes(measures[0]->attributes[0], &currAttributes);
    size_t divisionCounter = 0;
    int currTimerLevel = 0;
    int fps = 0;
    double percentage = 0;
    while(!glfwWindowShouldClose(interface->g->window) && measureIndex < piano->measureSize){
        draw(piano, percentage, KEYBOARD_SHEET_MODE);
        // glfwSwapBuffers(interface->g->window);
        // processPollEvents();
        // glfwPollEvents();

        if(interface->paused){
            continue;
        }

        struct Measure *currMeasure = measures[measureIndex];
        double time = glfwGetTime();
        double alpha = time - prevTime;
        prevTime = time;

        double beat = 60.0 / currBmp;
        double divisionDuration = (beat * 4.0) / (float)currMeasure->measureSize;
        // debugf("dur %f\n", divisionDuration);
        
        // calculate "fps"
        fps++;
        if((int)time > currTimerLevel){
            currTimerLevel++;
            debugf("%i\n", fps);
            fps = 0;
        }

        // debugf("time %f\n", time);
        divisionTimer += alpha;
        // debugf("%f\n", divisionTimer);

        float oneD = 1.0f / (float)currMeasure->measureSize;
        percentage = currDivision * oneD + oneD * divisionTimer / divisionDuration;
        // printf("%f\n", percentage * 100);

        if(divisionTimer < divisionDuration){
            continue;
        }

        // new division
        divisionTimer = 0;

        // the prev division already passed so increase the index
        currDivision++;
        divisionCounter++;

        if(currMeasure->measureSize <= currDivision){
            measureIndex++;
            currMeasure = measures[measureIndex];
            currDivision = 0;
            piano->sheet->currMeasure = measureIndex;
            // exit(0);
        }

        // debugf("size: %zu\n", pressedNotes->size);
        // debugf("position: %zu %i %f\n", measureIndex, currDivision, divisionDuration);

        // turn off any notes
        for(size_t i = 0; i < piano->playedNotesVector->size; ){
            struct PlayedNote *pressedNote = piano->playedNotesVector->data[i];
            if(pressedNote->endDivision <= divisionCounter){
                unpressNote(piano, i);
            }
            else{
                i++;
            }
        }

        for(StaffNumber s = 0; s < currAttributes.stavesNumber; s++){
            Staff staff = currMeasure->staffs[s];
            struct Notes *notes = staff[currDivision];
            if(!notes){
                continue;
            }

            for(ChordSize c = 0; c < notes->chordSize; c++){
                struct Note *note = notes->chord[c];
                
                if(!GET_BIT(note->flags, NOTE_FLAG_REST)){
                    pressNote(piano, note, divisionCounter);
                }
            }
        }
    }
}

void pianoLearnSong(struct Piano *piano){
    struct Measure **measures = piano->measures;
    size_t measureIndex = 0;
    Division currDivision = -1;
    double currBmp = 12;
    double prevTime = 0;
    struct Attributes currAttributes = {};
    double divisionTimer = 0;

    glfwSwapInterval(0);
    updateAttributes(measures[0]->attributes[0], &currAttributes);
    size_t divisionCounter = 0;
    int currTimerLevel = 0;
    int fps = 0;
    double percentage = 0;
    while(!glfwWindowShouldClose(interface->g->window) && measureIndex < piano->measureSize){
        draw(piano, percentage, KEYBOARD_PIANO_MODE);
        midiRead(piano);
    }
}
