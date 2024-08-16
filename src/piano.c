#include "piano.h"

#include "interface.h"
#include <string.h>
extern struct Interface *interface;

extern GLuint elementArrayBuffer;

extern GLint globalMatUniform;
extern GLint localMatUniform;
extern GLint colorUniform;

struct MeshBoundingBox *meshBoundingBoxes;

#define MATCH(MATCH_NAME, MATCH_ID){ \
    if(strcmp(name, MATCH_NAME) == 0){ \
        return MATCH_ID; \
    } \
}

VECTOR_TYPE_FUNCTIONS(size_t, SizeVector, "%zu");
VECTOR_TYPE_FUNCTIONS(struct Item*, ItemPVector, "%p");
VECTOR_TYPE_FUNCTIONS(struct Item**, ItemPPVector, "%p");
VECTOR_TYPE_FUNCTIONS(struct ItemMeasure*, ItemMeasurePVector, "%p");

VECTOR_TYPE_FUNCTIONS(struct PressedNote*, PressedNotePVector, "%p");

#define WHITE_KEYS_SIZE 7
#define BLACK_KEYS_SIZE 5

enum Meshes whiteKeysMeshId[WHITE_KEYS_SIZE] = {
    C, D, E, F, G, A, H,
};

char whiteKeysNames[WHITE_KEYS_SIZE][2] = {
    "C", "D", "E", "F", "G", "A", "H",
};

enum Meshes blackKeysMeshId[BLACK_KEYS_SIZE] = {
    C_SHARP, D_SHARP, F_SHARP, G_SHARP, A_SHARP
};

char blackKeysNames[BLACK_KEYS_SIZE][3] = {
    "C#", "D#", "F#", "G#", "A#"
};

// :)
enum Meshes assignMeshId(char *name){
    size_t s = strlen(name);
    if(s == 1 && name[0] - '0' < 10){
        return name[0] - '0' + TEXT_START;
    }
    
    if(strncmp(name, "note", 4) == 0){
        return name[4] - '0' + NOTE_START;
    }

    for(uint8_t i = 0; i < WHITE_KEYS_SIZE; i++){
        MATCH(whiteKeysNames[i], whiteKeysMeshId[i]);
    }
    
    for(uint8_t i = 0; i < BLACK_KEYS_SIZE; i++){
        MATCH(blackKeysNames[i], blackKeysMeshId[i]);
    }

    MATCH("fClef", F_CLEF);
    MATCH("fClef8Up", F_CLEF_8_UP);
    MATCH("fClef15Up", F_CLEF_15_UP);
    MATCH("fClef8Down", F_CLEF_8_DOWN);
    MATCH("fClef15Down", F_CLEF_15_DOWN);
    MATCH("gClef", G_CLEF);
    MATCH("gClef8Up", G_CLEF_8_UP);
    MATCH("gClef15Up", G_CLEF_15_UP);
    MATCH("gClef8Down", G_CLEF_8_DOWN);
    MATCH("gClef15Down", G_CLEF_15_DOWN);
    MATCH("cClef", C_CLEF);
    
    MATCH("commonTime", COMMON_TIME);
    MATCH("cutTime", CUT_TIME);
    
    MATCH("time2", TIME_2);
    MATCH("time3", TIME_3);
    MATCH("time4", TIME_4);
    MATCH("time6", TIME_6);
    MATCH("time8", TIME_8);

    MATCH("wholeRest", WHOLE_REST);
    MATCH("halfRest", HALF_REST);
    MATCH("eightRest", EIGHT_REST);
    MATCH("sixteenthRest", SIXTEENTH_REST);
    MATCH("thirtySecondRest", THIRTY_SECOND_REST);
    MATCH("sixtyFourthRest", SIXTY_FOURTH_REST);
    MATCH("hundertTwentyEightRest", HUNDERT_TWENTY_EIGHT_REST);
    MATCH("doubleWholeHead", DOUBLE_WHOLE_HEAD);
    MATCH("wholeHead", WHOLE_HEAD);
    MATCH("halfHead", HALF_HEAD);
    MATCH("quaterHead", QUATER_HEAD);

    MATCH("flag1", FLAG1);
    MATCH("flag2", FLAG2);
    MATCH("flag3", FLAG3);
    MATCH("flag4", FLAG4);
    MATCH("flag5", FLAG5);

    MATCH("sharp", SHARP);
    MATCH("natural", NATURAL);
    MATCH("flat", FLAT);
    
    MATCH("lines", LINES);

    fprintf(stderr, "model str id '%s' not found\n", name);
    exit(1);
}

void meshBoundingBox(struct MeshBoundingBox *bb, float *data, uint32_t size){
    bb->min[0] = FLT_MAX;
    bb->min[1] = FLT_MAX;
    bb->min[2] = FLT_MAX;

    bb->max[0] = FLT_MIN;
    bb->max[1] = FLT_MIN;
    bb->max[2] = FLT_MIN;

    for(uint32_t i = 0; i < size; i += 3){
        bb->min[0] = MIN(data[i + 0], bb->min[0]);
        bb->min[1] = MIN(data[i + 1], bb->min[1]);
        bb->min[2] = MIN(data[i + 2], bb->min[2]);
        
        bb->max[0] = MAX(data[i + 0], bb->max[0]);
        bb->max[1] = MAX(data[i + 1], bb->max[1]);
        bb->max[2] = MAX(data[i + 2], bb->max[2]);
    }
}

void meshBoundingBoxPrint(enum Meshes id){
    struct MeshBoundingBox *bb = &meshBoundingBoxes[id];

    SET_COLOR(colorUniform, GREEN);
    // A ---- B
    // |      |
    // |      |
    // D ---- C
    vec3 a = {bb->min[0], bb->max[1], 0};
    vec3 b = {bb->max[0], bb->max[1], 0};
    vec3 c = {bb->max[0], bb->min[1], 0};
    vec3 d = {bb->min[0], bb->min[1], 0};

    drawLineVec(a, b);
    drawLineVec(b, c);
    drawLineVec(c, d);
    drawLineVec(d, a);

    drawLineVec(a, c);
    drawLineVec(b, d);

    SET_COLOR(colorUniform, WHITE);
}

void loadPianoMeshes(struct Piano *piano){
    FILE *file = fopen("Assets/models.bin", "rb");
    if(!file){
        fprintf(stderr, "failed to load mesh library\n");
        exit(1);
    }

    size_t *meshesDataStart = calloc(MESHES_SIZE, sizeof(size_t));
    size_t *meshesDataSize = calloc(MESHES_SIZE, sizeof(size_t));
    struct MeshBoundingBox *mbbs = calloc(MESHES_SIZE, sizeof(struct MeshBoundingBox));

    size_t dataSize = 0;
    float **dataArray = calloc(MESHES_SIZE, sizeof(float*));

    uint32_t numberOfMeshes = 0;
    fread(&numberOfMeshes, sizeof(uint32_t), 1, file);

    size_t keyMeshsesSize = 0;
    for(size_t i = 0; i < numberOfMeshes; i++){
        // debugf("file pos: %i\n", ftell(file));
        MeshStrIdSize idSize;
        fread(&idSize, sizeof(MeshStrIdSize), 1, file);

        char modelStrId[MAX_MODEL_ID_SIZE];
        fread(modelStrId, sizeof(char), idSize, file);
        modelStrId[idSize] = 0;
        enum Meshes id = assignMeshId(modelStrId);

        uint32_t size = 0;
        fread(&size, sizeof(uint32_t), 1, file);

        float *data = malloc(sizeof(float) * size);
        fread(data, sizeof(float), size, file);
        dataArray[id] = data;
        dataSize += size;

        struct MeshBoundingBox *mbb = &(mbbs[id]);
        meshBoundingBox(mbb, data, size);
        
        meshesDataSize[id] = size;
        // debugf("%s [%i] => %i\n", modelStrId, size, id);

        if(C <= id && id <= LINES){
            keyMeshsesSize += size;       
        }
    }
    
    // copy the keayboardkey to the buffer
    float *keys = malloc(sizeof(float) * keyMeshsesSize);
    size_t keysPointer = 0;
    // meshes C to H + C# to A# + LINES
    for(enum Meshes id = C; id <= LINES; id++){
        size_t meshSize = meshesDataSize[id];
        memcpy(&keys[keysPointer], dataArray[id], sizeof(float) * meshSize);
        keysPointer += meshSize;
    }
    piano->keyboard.keys = keys;

    float *data = malloc(sizeof(float) * dataSize);
    size_t dataPointer = 0;

    meshesDataStart[0] = 0;
    for(size_t i = 1; i < MESHES_SIZE; i++){
        if(meshesDataSize[i]){
            // debugf("dataSize: %zu %p\n", meshesDataSize[i], dataArray[i]);
            memcpy(&data[dataPointer], dataArray[i], sizeof(float) * meshesDataSize[i]);
            dataPointer += meshesDataSize[i];
            // debugf("dataPointer: %zu\n", dataPointer);
        }

        if(i - 1){
            meshesDataStart[i] = meshesDataStart[i - 1] + meshesDataSize[i - 1];
            // debugf("data start %zu\n", meshesDataStart[i]);
        }
    }

    // for(size_t i = 0; i < 12; i += 3){
    //     debugf("%f %f %f\n", data[i], data[i + 1], data[i + 2]);
    // }
    glBufferData(GL_ARRAY_BUFFER, VERTEX_BUFFER_BYTES_SIZE, NULL, GL_DYNAMIC_DRAW);
    size_t dataSizeInBytes = sizeof(float) * dataSize;
    // piano->meshesDataStartOffset = VERTEX_BUFFER_SIZE - dataSizeInBytes;
    // glBufferSubData(GL_ARRAY_BUFFER, piano->meshesDataStartOffset, dataSizeInBytes, data);
    piano->meshesDataStartOffset = VERTEX_BUFFER_COUNT - dataSize / 3;
    glBufferSubData(GL_ARRAY_BUFFER, piano->meshesDataStartOffset * VERTEX_BYTES_SIZE, dataSizeInBytes, data);
    free(data);
    free(dataArray);
    // piano->data = data;
    // piano->dSize = dataSizeInBytes;

    piano->meshesDataStart = meshesDataStart;
    piano->meshesDataSize = meshesDataSize;
    meshBoundingBoxes = mbbs;
}

struct Piano *pianoInit(struct Measure **measures, size_t measureSize){
    graphicsInit();
    struct Piano *piano = calloc(1, sizeof(struct Piano));
    interface->piano = piano;
    loadPianoMeshes(piano);
    piano->cursor = 0;
    piano->currMeasure = 0;

    piano->measures = measures;
    piano->measureSize = measureSize;

    // piano->midiDevice = midiDeviceInit("auto");
    piano->midiDevice = -1;

    piano->pressedNotesVector = PressedNotePVectorInit();

    piano->keyboard.keysDataStart = malloc(sizeof(size_t) * KEYBOARD_KEY_SIZE);

    return piano;
}

void drawSheet(struct Piano *piano){
    glClear(GL_COLOR_BUFFER_BIT);

    float stretch = 1.1f;
    float clearance = 0.7f;

    float offset = 0 + clearance;
    struct Sheet *sheet = piano->sheet;

    float scale = 0.008f;
    drawKeyboard(piano);
    // glEnable(GL_SCISSOR_TEST);
    // // if(interface->g->xPos < interface->g->width
    // glScissor(interface->xPos, -interface->yPos + interface->g->height - 100, interface->g->width, 120);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glDisable(GL_SCISSOR_TEST);

    mat4 globMatrix = {};
    vec3 globScale = {interface->scale, interface->scale, interface->scale};
    vec3 globPos = {interface->xPos, interface->yPos, 0};
    glm_mat4_identity(globMatrix);
    glm_scale(globMatrix, globScale);
    glm_translate(globMatrix, globPos);
    glUniformMatrix4fv(globalMatUniform, 1, GL_FALSE, (float*)globMatrix);

    mat4 clearMat = {};
    glm_mat4_identity(clearMat);
    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)clearMat);

    // float lastLine = sheet->staffOffsets[sheet->staffNumber - 1] + (piano->sheet->staffNumber-1) * 8.f;
    //
    // float start = 1;
    // float yScale = scale * interface->g->screenRatio;
    // float height = lastLine * yScale + 0.2f;
    // SET_COLOR(colorUniform, BLACK);
    // drawRectangle(-1, start, 1, start - height);

    // size_t s = 0;
    vec3 cursor = {-0.8f, 0.8f, 0};
    float lastStaffOffset = piano->sheet->staffOffsets[piano->sheet->staffNumber - 1];
    float barHeight = lastStaffOffset + (piano->sheet->staffNumber - 1) * 4.f;

    vec3 scaleVec = {scale, scale * interface->g->screenRatio, scale};
    mat4 cleanMat = {};
    glm_mat4_identity(cleanMat);
    glm_translate(cleanMat, cursor);
    glm_scale(cleanMat, scaleVec);
    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)cleanMat);
    SET_COLOR(colorUniform, WHITE);
    
    // for(size_t m = 0; m < 0; m++){
    for(size_t m = piano->currMeasure; m < sheet->measuresSize; m++){
        struct ItemMeasure *measure = sheet->measures[m];
        size_t itemsSize = measure->size;
        for(size_t i = 0; i < itemsSize; i++){
            struct Item *item = measure->items[i];
            float staffOffset = sheet->staffOffsets[item->staffIndex];
            // debugf("%i %f\n", item->staffIndex, staffOffset);
            enum Meshes meshId = item->meshId;
            // debugf("meshId: %i\n", meshId);
            if(item->type == ITEM_MESH){
                // useShader(interface->shader);
                SET_COLOR(colorUniform, WHITE);

                bool playedNote = false;
                if(item->typeSubGroup == ITEM_GROUP_NOTE_HEAD){
                    for(size_t p = 0; p < piano->pressedNotesVector->size; p++){
                        // debugf("pn: %p\n", pressedNotes->data[p]);
                        struct Note *note = piano->pressedNotesVector->data[p]->note;
                        if(note->item == item){
                            // debugf("%p == %p\n", note->item, item);
                            playedNote = true;
                            SET_COLOR(colorUniform, RED);
                            break;
                        }
                    }
                }

                size_t trigCount = piano->meshesDataSize[meshId] / 3;
                GLint index = piano->meshesDataStart[meshId] / 3 + piano->meshesDataStartOffset;
                mat4 mMat = {};
                glm_mat4_identity(mMat);
                struct ItemMesh *iM = item->data;
                glm_translate(mMat, cursor);
                glm_scale(mMat, scaleVec);
                // vec3 pos = {(offset + iM->xPosition) * 1.5, 50, 0};
                float width = MBB_MAX(meshId)[0] / 2.0f;
                vec3 pos = {(offset + iM->xPosition) * stretch, iM->yPosition - staffOffset, 0};
                glm_translate(mMat, pos);

                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mMat);

                // debugf("mesh id: %i %i %zu\n", meshId, index, trigCount);
                glDrawArrays(GL_TRIANGLES, index, trigCount);

                if(playedNote == true){
                    SET_COLOR(colorUniform, WHITE);
                }

                // if(interface->showBoudningBox){
                //     //useShader(interface->shader);
                //     glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mMat);
                //     meshBoundingBoxPrint(item->meshId);
                //     glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)cleanMat);
                // }
            }
            else if(item->type == ITEM_BAR){

            }
            else if(item->type == ITEM_STEM){
                //useShader(interface->shader);
                // staffOffset
                struct ItemStem *itemStem = item->data;
                // itemStem->
                float x = (itemStem->xStart + offset) * stretch + itemStem->noteOffset;

                float y1 = itemStem->y1 - staffOffset;
                float y2 = itemStem->y2 - staffOffset;
                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)cleanMat);

                drawLine(x, y1, 0, x, y2, 0);
            }
            else if(item->type == ITEM_BEAM){
                //useShader(interface->shader);

                struct ItemBeam *beam = item->data;
                float x1 = (beam->xStart + offset) * stretch;
                float x2 = (beam->xEnd   + offset) * stretch;
                float w1 = beam->w1;
                float w2 = beam->w2;
                float y = beam->yStart - staffOffset;

                // debugf("%f %f %f\n", x1, x2, y);
                // drawLine(x1, y, 0, x2, y, 0);
                vec3 p1 = {x1 + w1, y, 0};
                vec3 p2 = {x2 + w2, y, 0};
                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)cleanMat);
                drawLineWeight(p1, p2, cursor, scaleVec, BEAM_HEIGHT, localMatUniform);
            }
            else if(item->type == ITEM_LINE){
                //useShader(interface->shader);

                mat4 mMat = {};
                glm_mat4_identity(mMat);
                glm_translate(mMat, cursor);
                glm_scale(mMat, scaleVec);
                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mMat);

                struct ItemLine *line = item->data;
                float x1 = (line->x + offset) * stretch;
                float x2 = ((line->x + line->width) + offset) * stretch;
                float y = line->y - staffOffset;

                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)cleanMat);
                drawLine(x1, y, 0, x2, y, 0);
            }
            else if(item->type == ITEM_FLAG){
                //useShader(interface->modelShader);
                SET_COLOR(colorUniform, WHITE);

                size_t trigCount = piano->meshesDataSize[meshId] / 3;
                GLint index = piano->meshesDataStart[meshId] / 3;
                
                mat4 mMat = {};
                glm_mat4_identity(mMat);
                struct ItemFlag *flag = item->data;
                glm_translate(mMat, cursor);
                glm_scale(mMat, scaleVec);
                // vec3 pos = {(offset + iM->xPosition) * 1.5, 50, 0};
                vec3 pos = {(offset + flag->xPosition) * stretch + flag->width, flag->yPosition - staffOffset, 0};
                glm_translate(mMat, pos);
                if(flag->inverted){
                    glm_scale(mMat, (vec3){1, -1, 1});
                }

                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mMat);
                glDrawArrays(GL_TRIANGLES, index, trigCount);
            }
            else{
                fprintf(stderr, "item type '%i' not implemented\n", item->type);
                exit(1);
            }
        }

        offset += measure->width;
        // //useShader(interface->shader);
        // // mat4 mMat = {};
        // // glm_mat4_identity(mMat);
        // // glm_translate(mMat, cursor);
        // // glm_scale(mMat, scaleVec);
        // // glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mMat);
        // float x = (float)(offset + m) * stretch;
        float x = offset * stretch;
        drawLine(x, 4.f, 0, x, - barHeight, 1);
        offset += clearance;
        //
        // SET_COLOR(colorUniform, WHITE);
    }

    // draw staffs
    for(StaffNumber s = 0; s < sheet->staffNumber; s++){
        float staffOffset = sheet->staffOffsets[s];
        for(int8_t yOffset = -2; yOffset <= 2; yOffset++){
            float y = yOffset * 2.0f + staffOffset;
            drawLine(0, -y, 0, offset * stretch, -y, 0);
        }
    }

    glfwSwapBuffers(interface->g->window);
    processPollEvents();
    glfwPollEvents();
}

uint8_t stepArray[] = {0, 2, 4, 5, 7, 9, 11};

uint8_t fromPitch(struct NotePitch *pitch){
    uint8_t step = stepArray[pitch->step];
    uint8_t r = (pitch->octave) * 12 + step + pitch->alter;
    // debugf("%i %i %i\n", step, pitch->stepChar, r);
    return r;
}

void pressNote(struct Piano *piano, struct Note *note, Division divisionCounter){
    sendNoteEvent(piano->midiDevice, NOTE_ON, &note->pitch, 100);
    
    piano->playedNotes[fromPitch(&note->pitch)] = true;

    struct PressedNote *pressedNote = malloc(sizeof(struct PressedNote));
    pressedNote->note = note;
    pressedNote->endDivision = note->duration + divisionCounter;
    // debugf("adding note %p at %zu\n", pressedNote, pressedNotes->size - 1);
    PressedNotePVectorPush(piano->pressedNotesVector, pressedNote);
}

void unpressNote(struct Piano *piano, size_t i){
    struct PressedNote *pressedNote = piano->pressedNotesVector->data[i];
    sendNoteEvent(piano->midiDevice, NOTE_OFF, &pressedNote->note->pitch, 0);

    piano->playedNotes[fromPitch(&pressedNote->note->pitch)] = false;

    // debugf("removing %zu note %p\n", i, pressedNote);
    if(piano->pressedNotesVector->size - 1 != i){
        struct PressedNote *lastNote = piano->pressedNotesVector->data[piano->pressedNotesVector->size - 1], *temp = NULL;
        // debugf("swap {%p} %p\n", pressedNote, lastNote);
        temp = pressedNote;
        pressedNote = lastNote;
        lastNote = temp;
        // debugf("free %p\n", lastNote);
        free(lastNote);
        piano->pressedNotesVector->data[i] = pressedNote;
    }

    piano->pressedNotesVector->size--;
}

void pianoPlaySong(struct Piano *piano){
    // uint8_t buffer[300] = {};
    // while(1){
    //     midiRead(midiDevice, buffer);
    // }

    struct Measure **measures = piano->measures;
    size_t measureIndex = 0;
    Division currDivision = -1;
    float currBmp = 120;
    float prevTime = 0;
    struct Attributes currAttributes = {};
    float divisionTimer = 0;

    updateAttributes(measures[0]->attributes[0], &currAttributes);
    size_t divisionCounter = 0;
    while(!glfwWindowShouldClose(interface->g->window) && measureIndex < piano->measureSize){
        drawSheet(piano);

        if(interface->paused){
            continue;
        }

        struct Measure *currMeasure = measures[measureIndex];
        float time = glfwGetTime();
        float alpha = time - prevTime;
        prevTime = time;

        float beat = 60.0f / currBmp;
        float divisionDuration = (beat * 4.f) / (float)currMeasure->measureSize;
        // debugf("dur %f %f %f\n", measuresPerMin,  measureDuration, divisionDuration);

        // debugf("time %f\n", time);
        divisionTimer += alpha;
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
            piano->currMeasure = measureIndex;
            // exit(0);
        }

        // debugf("size: %zu\n", pressedNotes->size);
        // debugf("position: %zu %i %f\n", measureIndex, currDivision, divisionDuration);

        // turn off any notes
        for(size_t i = 0; i < piano->pressedNotesVector->size; ){
            struct PressedNote *pressedNote = piano->pressedNotesVector->data[i];
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

#define WHITE_KEY 0
#define BLACK_KEY 1

uint8_t keyType[] = {
//  C          C#         D          D#         E
    WHITE_KEY, BLACK_KEY, WHITE_KEY, BLACK_KEY, WHITE_KEY,
//  F          F#         G          G#         A          A#         H
    WHITE_KEY, BLACK_KEY, WHITE_KEY, BLACK_KEY, WHITE_KEY, BLACK_KEY, WHITE_KEY
};

enum Meshes noteToMesh[] = {
    C, C_SHARP, D, D_SHARP, E, 
    F, F_SHARP, G, G_SHARP, A, A_SHARP, H,
};

void computeKeyboard(struct Piano *piano, struct NotePitch *s, struct NotePitch *e){
    size_t whiteTrigCount = 0, blackTrigCount = 0;
    size_t linesTrigCount = 0;

    uint8_t start = fromPitch(s);
    uint8_t end = fromPitch(e);
    uint8_t size = end - start + 1;
    debugf("start end: %i %i\n", start, end);

    size_t whiteKeysMeshSize = 0, blackKeysMeshSize = 0;

    for(uint8_t i = 0; i < size; i++){
        uint8_t p = i + start;
        uint8_t note = p % 12;
        
        enum Meshes meshId = noteToMesh[note];
        size_t meshSize = piano->meshesDataSize[meshId];
        debugf("%i %zu\n", meshId, meshSize);
        
        if(keyType[note] == WHITE_KEY){
            whiteKeysMeshSize += meshSize;
        }
        else{
            blackKeysMeshSize += meshSize;
        }
    }
    
    // buffers for white and black keys
    float *whiteKeys = malloc(sizeof(float) * whiteKeysMeshSize);
    float *blackKeys = malloc(sizeof(float) * blackKeysMeshSize);
    debugf("%p %zu\n", whiteKeys, whiteKeysMeshSize);
    debugf("%p %zu\n", blackKeys, blackKeysMeshSize);

    // clear the map
    memset(piano->keyboard.keysDataStart, 0, sizeof(size_t) * KEYBOARD_KEY_SIZE);
    
    float offset = MBB_MIN(D)[0] - MBB_MAX(C)[0];
    float octaveWidth = MBB_MAX(H)[0] + offset;

    float widht = 0;
    size_t counter = 0;
    size_t whiteKeysPointer = 0, blackKeysPointer = 0;
    for(uint8_t i = 0; i < size; i++){
        uint8_t p = i + start;
        uint8_t note = p % 12;
        uint8_t octave = p / 12 - (start / 12);
        
        enum Meshes meshId = noteToMesh[note];
        size_t meshSize = piano->meshesDataSize[meshId];

        float *buffer = (keyType[note] == WHITE_KEY) ? whiteKeys : blackKeys;
        size_t *pointer = (keyType[note] == WHITE_KEY) ? &whiteKeysPointer : &blackKeysPointer;
        size_t *trigCount = (keyType[note] == WHITE_KEY) ? &whiteTrigCount : &blackTrigCount; 

        // copy the keys
        size_t meshIndex = (piano->meshesDataStart[meshId] - piano->meshesDataStart[C]);
        debugf("%zu %p %zu\n", *pointer, buffer, meshIndex);
        memcpy(&buffer[*pointer], &piano->keyboard.keys[meshIndex], sizeof(float) * meshSize);

        piano->keyboard.keysDataStart[p] = counter;
        counter += (meshSize / VERTEX_SIZE);

        // translate the vertices
        if(octave){
            for(size_t v = 0; v < meshSize; v += VERTEX_SIZE){
                buffer[*pointer + v] += octave * octaveWidth;
                widht = MAX(widht, buffer[*pointer + v]);
            }
        }
        
        *pointer += meshSize;
        *trigCount += meshSize;
    }

    uint8_t numberOfOctaves = (size + 12 - 1) / 12;
    debugf("numberOfOctaves: %i\n", numberOfOctaves);
    size_t linesMeshSize = piano->meshesDataSize[LINES];
    size_t linesOffset = (piano->meshesDataStart[LINES] - piano->meshesDataStart[C]);
    float *lines = malloc(sizeof(float) * linesMeshSize * numberOfOctaves);
    for(uint8_t o = 0; o < numberOfOctaves; o++){
        size_t bufferIndex = o * linesMeshSize;
        debugf("bufferIndex: %zu\n", bufferIndex);
        memcpy(&lines[bufferIndex], &piano->keyboard.keys[linesOffset], sizeof(float) * linesMeshSize);
        for(size_t v = 0; v < linesMeshSize; v += VERTEX_SIZE){
            lines[bufferIndex + v] += o * octaveWidth;
        }
        linesTrigCount += linesMeshSize;
    }

    piano->keyboard.keyboardWidth = widht;

    whiteTrigCount /= VERTEX_SIZE;
    blackTrigCount /= VERTEX_SIZE;
    linesTrigCount /= VERTEX_SIZE;

    piano->keyboard.whiteKeysTrigCount = whiteTrigCount;
    piano->keyboard.blackKeysTrigCount = blackTrigCount;

    size_t whiteKeysStart = (VERTEX_BUFFER_COUNT - piano->meshesDataStartOffset) - (whiteTrigCount + blackTrigCount + linesTrigCount);
    size_t blackKeysStart = whiteKeysStart + whiteTrigCount;
    size_t linesStart = blackKeysStart + linesTrigCount;
    piano->keyboard.whiteKeysDataStartOffset = whiteKeysStart;
    piano->keyboard.blackKeysDataStartOffset = blackKeysStart;
    piano->keyboard.linesDataStartOffset = linesStart;

    glBufferSubData(GL_ARRAY_BUFFER, whiteKeysStart * VERTEX_BYTES_SIZE, whiteTrigCount * VERTEX_BYTES_SIZE, whiteKeys);
    glBufferSubData(GL_ARRAY_BUFFER, blackKeysStart * VERTEX_BYTES_SIZE, blackTrigCount * VERTEX_BYTES_SIZE, blackKeys);
    glBufferSubData(GL_ARRAY_BUFFER, linesStart * VERTEX_BYTES_SIZE, linesTrigCount * VERTEX_BYTES_SIZE, lines);
    free(whiteKeys);
    free(blackKeys);
    free(lines);
}

void drawKeyboard(struct Piano *piano){
    mat4 mat = {};
    glm_mat4_identity(mat);

    size_t linesStartIndex = interface->piano->keyboard.linesDataStartOffset;
    size_t whiteStartIndex = interface->piano->keyboard.whiteKeysDataStartOffset;
    size_t blackStartIndex = interface->piano->keyboard.blackKeysDataStartOffset;

    float scale = 2.0f / piano->keyboard.keyboardWidth;

    float lastLine = piano->sheet->staffOffsets[piano->sheet->staffNumber - 1] + (piano->sheet->staffNumber-1) * 8.f;

    float sscale = 0.008f;
    float yScale = sscale * interface->g->screenRatio;
    float height = (lastLine * yScale) + 0.2f + MBB_MAX(C)[1] * scale * interface->g->screenRatio;
    // drawRectangle(-1, start, 1, start - height);
    float scaleY = (2.0f - height) / MBB_MAX(LINES)[1];
    vec3 scaleLineVec = {scale, scaleY, scale};
    glm_translate(mat, (vec3){-1, -1, 0});
    glm_translate_y(mat, MBB_MAX(C)[1] * scale * interface->g->screenRatio);
    glm_scale(mat, scaleLineVec);

    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mat);

    // SET_COLOR(colorUniform, WHITE);
    // glDrawArrays(GL_TRIANGLES, linesStartIndex, piano->keyboard.linesDataStartOffset);

    glm_mat4_identity(mat);

    vec3 scaleVec = {scale, scale * interface->g->screenRatio, scale};
    glm_translate(mat, (vec3){-1, -1, 0});
    glm_scale(mat, scaleVec);

    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mat);

    SET_COLOR(colorUniform, WHITE);
    glDrawArrays(GL_TRIANGLES, whiteStartIndex, piano->keyboard.whiteKeysTrigCount);
    SET_COLOR(colorUniform, GRAY);
    glDrawArrays(GL_TRIANGLES, blackStartIndex, piano->keyboard.blackKeysTrigCount);
}

// void drawNotes(struct Piano *piano){
//     // piano->pressedNotesVector
//     size_t m = 0;
//     while(true){
//         size_t p = piano->currMeasure;
//         struct Measure *measure = piano->measures[m];
//         for(size_t s = 0; s < piano->sheet->staffNumber; s++){
//             for(Division d = 0; d < measure->measureSize; d++){
//                 struct Notes *notes = measure->staffs[s][d];
//                 for(size_t i = 0; i < notes->chordSize; i++){
//                     struct Note *note = notes->chord[i];
//                     fromPitch(note->pitch);
//                     note->duration
//                 }
//             }
//         }
//     }
// }
