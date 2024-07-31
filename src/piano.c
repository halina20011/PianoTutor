#include "piano.h"

#include "interface.h"
#include <string.h>
extern struct Interface *interface;

extern GLuint elementArrayBuffer;

extern GLint shaderGlobalMatUniform;
extern GLint modelShaderGlobalMatUniform;

extern GLint shaderMatUniform;
extern GLint modelShaderMatUniform;

extern GLint shaderColorUniform;
extern GLint modelShaderColorUniform;

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

uint8_t assignMeshId(char *name){
    if(strlen(name) == 1){
        return name[0] - '0' + TEXT_START;
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

    SET_COLOR(modelShaderColorUniform, GREEN);
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

    SET_COLOR(modelShaderColorUniform, WHITE);
}

void loadPianoMeshs(struct Piano *piano){
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

    for(size_t i = 0; i < numberOfMeshes; i++){
        // debugf("file pos: %i\n", ftell(file));
        MeshStrIdSize idSize;
        fread(&idSize, sizeof(MeshStrIdSize), 1, file);

        char modelStrId[MAX_MODEL_ID_SIZE];
        fread(modelStrId, sizeof(char), idSize, file);
        modelStrId[idSize] = 0;
        MeshId id = assignMeshId(modelStrId);

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
    }

    float *data = malloc(sizeof(float) * dataSize);
    // debugf("dataSize: %zu\n", dataSize);
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
    useShader(interface->modelShader);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSize, data, GL_STATIC_DRAW);
    free(data);

    piano->meshesDataStart = meshesDataStart;
    piano->meshesDataSize = meshesDataSize;
    meshBoundingBoxes = mbbs;
}

struct Piano *pianoInit(struct Measure **measures, size_t measureSize){
    graphicsInit();
    struct Piano *piano = calloc(1, sizeof(struct Piano));
    interface->piano = piano;
    loadPianoMeshs(piano);
    piano->cursor = 0;
    piano->currMeasure = 0;

    piano->measures = measures;
    piano->measureSize = measureSize;

    return piano;
}

void drawSheet(struct Piano *piano, struct PressedNotePVector *pressedNotes){
    glClear(GL_COLOR_BUFFER_BIT);

    float stretch = 1.1f;
    float clearance = 0.7f;

    float offset = 0 + clearance;
    struct Sheet *sheet = piano->sheet;

    mat4 globMatrix = {};
    vec3 globScale = {interface->scale, interface->scale, interface->scale};
    vec3 globPos = {interface->xPos, interface->yPos, 0};
    glm_mat4_identity(globMatrix);
    // glm_translate(globMatrix, globPos);
    glm_scale(globMatrix, globScale);
    glm_translate(globMatrix, globPos);
    useShader(interface->modelShader);
    glUniformMatrix4fv(modelShaderGlobalMatUniform, 1, GL_FALSE, (float*)globMatrix);

    useShader(interface->shader);
    glUniformMatrix4fv(shaderGlobalMatUniform, 1, GL_FALSE, (float*)globMatrix);

    // size_t s = 0;
    float scale = 0.008f;
    vec3 scaleVec = {scale, scale * interface->g->screenRatio, scale};
    vec3 cursor = {-0.8f, 0.8f, 0};
    float lastStaffOffset = piano->sheet->staffOffsets[piano->sheet->staffNumber - 1];
    float barHeight = lastStaffOffset + (piano->sheet->staffNumber - 1) * 4.f;

    mat4 cleanMat = {};
    glm_mat4_identity(cleanMat);
    glm_translate(cleanMat, cursor);
    glm_scale(cleanMat, scaleVec);
    glUniformMatrix4fv(shaderMatUniform, 1, GL_FALSE, (float*)cleanMat);
    
    // for(size_t m = 0; m < 1; m++){
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
                useShader(interface->modelShader);
                SET_COLOR(modelShaderColorUniform, WHITE);

                bool playedNote = false;
                if(item->typeSubGroup == ITEM_GROUP_NOTE_HEAD){
                    for(size_t p = 0; p < pressedNotes->size; p++){
                        // debugf("pn: %p\n", pressedNotes->data[p]);
                        struct Note *note = pressedNotes->data[p]->note;
                        if(note->item == item){
                            // debugf("%p == %p\n", note->item, item);
                            playedNote = true;
                            SET_COLOR(modelShaderColorUniform, RED);
                            break;
                        }
                    }
                }

                size_t trigCount = piano->meshesDataSize[meshId] / 3;
                GLint index = piano->meshesDataStart[meshId] / 3;
                mat4 mMat = {};
                glm_mat4_identity(mMat);
                struct ItemMesh *iM = item->data;
                glm_translate(mMat, cursor);
                glm_scale(mMat, scaleVec);
                // vec3 pos = {(offset + iM->xPosition) * 1.5, 50, 0};
                vec3 pos = {(offset + iM->xPosition) * stretch, iM->yPosition - staffOffset, 0};
                glm_translate(mMat, pos);

                glUniformMatrix4fv(modelShaderMatUniform, 1, GL_FALSE, (float*)mMat);

                // debugf("mesh id: %i %i %zu\n", meshId, index, trigCount);
                glDrawArrays(GL_TRIANGLES, index, trigCount);

                if(playedNote == true){
                    SET_COLOR(modelShaderColorUniform, WHITE);
                }

                if(interface->showBoudningBox){
                    useShader(interface->shader);
                    glUniformMatrix4fv(shaderMatUniform, 1, GL_FALSE, (float*)mMat);
                    meshBoundingBoxPrint(item->meshId);
                    glUniformMatrix4fv(shaderMatUniform, 1, GL_FALSE, (float*)cleanMat);
                }
            }
            else if(item->type == ITEM_BAR){

            }
            else if(item->type == ITEM_STEM){
                useShader(interface->shader);
                // staffOffset
                struct ItemStem *itemStem = item->data;
                // itemStem->
                float x = (itemStem->xStart + offset) * stretch + itemStem->noteOffset;

                float y1 = itemStem->y1 - staffOffset;
                float y2 = itemStem->y2 - staffOffset;

                drawLine(x, y1, 0, x, y2, 0);
            }
            else if(item->type == ITEM_BEAM){
                useShader(interface->shader);

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
                drawLineWeight(p1, p2, cursor, scaleVec, BEAM_HEIGHT, shaderMatUniform);
            }
            else if(item->type == ITEM_LINE){
                useShader(interface->shader);

                mat4 mMat = {};
                glm_mat4_identity(mMat);
                glm_translate(mMat, cursor);
                glm_scale(mMat, scaleVec);
                glUniformMatrix4fv(shaderMatUniform, 1, GL_FALSE, (float*)mMat);

                struct ItemLine *line = item->data;
                float x1 = (line->x + offset) * stretch;
                float x2 = ((line->x + line->width) + offset) * stretch;
                float y = line->y - staffOffset;

                drawLine(x1, y, 0, x2, y, 0);
                // drawLine(x1, y, 0, x1, y + 1, 0);
            }
            else{
                fprintf(stderr, "item type '%i' not implemented\n", item->type);
                exit(1);
            }
        }

        offset += measure->width;
        useShader(interface->shader);
        // mat4 mMat = {};
        // glm_mat4_identity(mMat);
        // glm_translate(mMat, cursor);
        // glm_scale(mMat, scaleVec);
        // glUniformMatrix4fv(shaderMatUniform, 1, GL_FALSE, (float*)mMat);
        // float x = (float)(offset + m) * stretch;
        float x = offset * stretch;
        drawLine(x, 4.f, 0, x, - barHeight, 1);
        offset += clearance;

        SET_COLOR(shaderColorUniform, WHITE);
    }

    // draw staffs
    useShader(interface->shader);
    // mat4 mMat = {};
    // glm_mat4_identity(mMat);
    // glm_translate(mMat, cursor);
    // glm_scale(mMat, scaleVec);
    // glUniformMatrix4fv(shaderMatUniform, 1, GL_FALSE, (float*)mMat);
    for(StaffNumber s = 0; s < sheet->staffNumber; s++){
        float staffOffset = sheet->staffOffsets[s];
        for(int8_t yOffset = -2; yOffset <= 2; yOffset++){
            float y = yOffset * 2.0f + staffOffset;
            drawLine(0, -y, 0, offset * stretch, -y, 0);
        }
    }

    // debugf("%zu\n", s);

    glfwSwapBuffers(interface->g->window);
    processPollEvents();
    glfwPollEvents();
}

void pressNote(struct Note *note, struct PressedNotePVector *pressedNotes, Division divisionCounter, int midiDevice){
    sendNoteEvent(midiDevice, NOTE_ON, &note->pitch, 100);
    struct PressedNote *pressedNote = malloc(sizeof(struct PressedNote));
    pressedNote->note = note;
    pressedNote->endDivision = note->duration + divisionCounter;
    PressedNotePVectorPush(pressedNotes, pressedNote);
    // debugf("adding note %p at %zu\n", pressedNote, pressedNotes->size - 1);
}

void pianoPlaySong(struct Piano *piano, int midiDevice){
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

    struct PressedNotePVector *pressedNotes = PressedNotePVectorInit();

    updateAttributes(measures[0]->attributes[0], &currAttributes);
    size_t divisionCounter = 0;
    while(!glfwWindowShouldClose(interface->g->window) && measureIndex < piano->measureSize){
        drawSheet(piano, pressedNotes);

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
        for(size_t i = 0; i < pressedNotes->size; ){
            struct PressedNote *pressedNote = pressedNotes->data[i];
            if(pressedNote->endDivision <= divisionCounter){
                sendNoteEvent(midiDevice, NOTE_OFF, &pressedNote->note->pitch, 0);

                // debugf("removing %zu note %p\n", i, pressedNote);
                if(pressedNotes->size - 1 != i){
                    struct PressedNote *lastNote = pressedNotes->data[pressedNotes->size - 1], *temp = NULL;
                    // debugf("swap {%p} %p\n", pressedNote, lastNote);
                    temp = pressedNote;
                    pressedNote = lastNote;
                    lastNote = temp;
                    // debugf("free %p\n", lastNote);
                    free(lastNote);
                    pressedNotes->data[i] = pressedNote;
                }
                pressedNotes->size--;
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
                    pressNote(note, pressedNotes, divisionCounter, midiDevice);
                }
            }
        }
    }
}

void drawKeyboard(){

}
