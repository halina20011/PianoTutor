#include "piano.h"

#include "interface.h"
extern struct Interface *interface;
extern GLint shaderMatUniform;
extern GLint modelShaderMatUniform;

struct MeshBoundingBox *meshBoundingBoxes;

#define MATCH(MATCH_NAME, MATCH_ID){ \
    if(strcmp(name, MATCH_NAME) == 0){ \
        return MATCH_ID; \
    } \
}

VECTOR_TYPE_FUNCTIONS(size_t, SizeVector, "");
VECTOR_TYPE_FUNCTIONS(struct Item*, ItemPVector, "");
VECTOR_TYPE_FUNCTIONS(struct Item**, ItemPPVector, "");
VECTOR_TYPE_FUNCTIONS(struct ItemMeasure*, ItemMeasurePVector, "");

uint8_t assignMeshId(char *name){
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
    MATCH("flat", FLAT);

    fprintf(stderr, "model str id '%s' not found\n", name);
    exit(1);
}

void meshBoundingBox(struct MeshBoundingBox *bb, float *data, size_t size){
    bb->min[0] = FLT_MAX;
    bb->min[1] = FLT_MAX;
    bb->min[2] = FLT_MAX;

    bb->max[0] = FLT_MIN;
    bb->max[1] = FLT_MIN;
    bb->max[2] = FLT_MIN;

    for(size_t i = 0; i < size; i += 3){
        bb->min[0] = MIN(data[i + 0], bb->min[0]);
        bb->min[1] = MIN(data[i + 1], bb->min[1]);
        bb->min[2] = MIN(data[i + 2], bb->min[2]);
        
        bb->max[0] = MAX(data[i + 0], bb->max[0]);
        bb->max[1] = MAX(data[i + 1], bb->max[1]);
        bb->max[2] = MAX(data[i + 2], bb->max[2]);
    }
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
        // printf("file pos: %i\n", ftell(file));
        MeshStrIdSize idSize;
        fread(&idSize, sizeof(MeshStrIdSize), 1, file);

        char modelStrId[MAX_MODEL_ID_SIZE];
        fread(modelStrId, sizeof(char), idSize, file);
        modelStrId[idSize] = 0;
        MeshId id = assignMeshId(modelStrId);
        printf("%s => %i\n", modelStrId, id);

        uint32_t size = 0;
        fread(&size, sizeof(uint32_t), 1, file);

        float *data = malloc(sizeof(float) * size);
        fread(data, sizeof(float), size, file);
        dataArray[id] = data;
        dataSize += size;

        struct MeshBoundingBox *mbb = &(mbbs[id]);
        meshBoundingBox(mbb, data, size);
        
        meshesDataSize[id] = size;
    }

    float *data = malloc(sizeof(float) * dataSize);
    // printf("dataSize: %zu\n", dataSize);
    size_t dataPointer = 0;

    meshesDataStart[0] = 0;
    for(size_t i = 1; i < MESHES_SIZE; i++){
        if(meshesDataSize[i]){
            // printf("dataSize: %zu %p\n", meshesDataSize[i], dataArray[i]);
            memcpy(&data[dataPointer], dataArray[i], sizeof(float) * meshesDataSize[i]);
            dataPointer += meshesDataSize[i];
            // printf("dataPointer: %zu\n", dataPointer);
        }

        if(i - 1){
            meshesDataStart[i] = meshesDataStart[i - 1] + meshesDataSize[i - 1];
            // printf("data start %zu\n", meshesDataStart[i]);
        }
    }

    // for(size_t i = 0; i < 12; i += 3){
    //     printf("%f %f %f\n", data[i], data[i + 1], data[i + 2]);
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

    computeMeasures(piano);

    return piano;
}

void drawSheet(struct Piano *piano){
    glClear(GL_COLOR_BUFFER_BIT);

    float stretch = 2.0f;
    float clearance = 0.7f;

    float offset = 0 + clearance;
    struct Sheet *sheet = piano->sheet;
    // size_t s = 0;
    float s = 0.008;
    vec3 scale = {s, s * interface->g->screenRatio, s};
    vec3 cursor = {-0.8, 0.8, 0};
    float lastStaffOffset = piano->sheet->staffOffsets[piano->sheet->staffNumber - 1];
    float barHeight = lastStaffOffset + (piano->sheet->staffNumber - 1) * 4.f;
    // for(size_t m = 0; m < 1; m++){
    for(size_t m = piano->currMeasure; m < sheet->measuresSize; m++){
        struct ItemMeasure *measure = sheet->measures[m];
        size_t itemsSize = measure->size;
        for(size_t i = 0; i < itemsSize; i++){
            struct Item *item = measure->items[i];
            float staffOffset = sheet->staffOffsets[item->staffIndex];
            // printf("%i %f\n", item->staffIndex, staffOffset);
            uint8_t meshId = item->meshId;
            // printf("meshId: %i\n", meshId);
            if(item->type == ITEM_MESH){
                useShader(interface->modelShader);

                size_t trigCount = piano->meshesDataSize[meshId] / 3;
                GLint index = piano->meshesDataStart[meshId] / 3;
                mat4 mMat = {};
                glm_mat4_identity(mMat);
                struct ItemMesh *iM = item->data;
                glm_translate(mMat, cursor);
                glm_scale(mMat, scale);
                // vec3 pos = {(offset + iM->xPosition) * 1.5, 50, 0};
                vec3 pos = {(offset + iM->xPosition) * stretch, - iM->yPosition - staffOffset, 0};
                glm_translate(mMat, pos);

                glUniformMatrix4fv(modelShaderMatUniform, 1, GL_FALSE, (float*)mMat);

                // printf("mesh id: %i %i %zu\n", meshId, index, trigCount);
                glDrawArrays(GL_TRIANGLES, index, trigCount);
            }
            else if(item->type == ITEM_BAR){

            }
            else if(item->type == ITEM_STEAM){
                // staffOffset
                struct ItemSteam *itemSteam = item->data;
                // itemSteam->
                float x = (itemSteam->xStart + offset) * stretch;
                float y = itemSteam->yStart;
                useShader(interface->shader);
                drawLine(x, y, 0, x, y + itemSteam->length, 0);
                useShader(interface->modelShader);
            }
            else{
                fprintf(stderr, "item type '%i' not implemented\n", item->type);
                exit(1);
            }
        }

        offset += measure->width;
        useShader(interface->shader);
        mat4 mMat = {};
        glm_mat4_identity(mMat);
        glm_translate(mMat, cursor);
        glm_scale(mMat, scale);
        glUniformMatrix4fv(shaderMatUniform, 1, GL_FALSE, (float*)mMat);
        // float x = (float)(offset + m) * stretch;
        float x = offset * stretch;
        drawLine(x, 4.f, 0, x, - barHeight, 1);
        offset += clearance;
    }

    // draw staffs
    useShader(interface->shader);
    for(StaffNumber s = 0; s < sheet->staffNumber; s++){
        float staffOffset = sheet->staffOffsets[s];
        for(int8_t yOffset = -2; yOffset <= 2; yOffset++){
            float y = yOffset * 2.0f + staffOffset;
            mat4 mMat = {};
            glm_mat4_identity(mMat);
            glm_translate(mMat, cursor);
            glm_scale(mMat, scale);
            glUniformMatrix4fv(shaderMatUniform, 1, GL_FALSE, (float*)mMat);
            drawLine(0, -y, 0, offset * stretch, -y, 0);
        }
    }

    // printf("%zu\n", s);

    glfwSwapBuffers(interface->g->window);
    processPollEvents();
    glfwPollEvents();
}

void drawKeyboard(){

}
