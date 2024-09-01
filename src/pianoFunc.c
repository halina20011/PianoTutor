#include "piano.h"

#include "interface.h"
extern struct Interface *interface;

extern GLint globalMatUniform;
extern GLint localMatUniform;
extern GLint colorUniform;

extern struct MeshBoundingBox *meshBoundingBoxes;

#define MATCH(MATCH_NAME, MATCH_ID){ \
    if(strcmp(name, MATCH_NAME) == 0){ \
        return MATCH_ID; \
    } \
}

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
        return name[4] - 'A' + NOTE_START;
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

void meshBoundingBoxClear(struct MeshBoundingBox *bb){
    bb->min[0] = FLT_MAX;
    bb->min[1] = FLT_MAX;
    bb->min[2] = FLT_MAX;

    bb->max[0] = FLT_MIN;
    bb->max[1] = FLT_MIN;
    bb->max[2] = FLT_MIN;
}

void meshBoundingBox(struct MeshBoundingBox *bb, float *data, uint32_t size){
    meshBoundingBoxClear(bb);

    for(uint32_t i = 0; i < size; i += 3){
        bb->min[0] = MIN(data[i + 0], bb->min[0]);
        bb->min[1] = MIN(data[i + 1], bb->min[1]);
        bb->min[2] = MIN(data[i + 2], bb->min[2]);
        
        bb->max[0] = MAX(data[i + 0], bb->max[0]);
        bb->max[1] = MAX(data[i + 1], bb->max[1]);
        bb->max[2] = MAX(data[i + 2], bb->max[2]);
    }
}

void meshBoundingBoxUpdate(struct MeshBoundingBox *bbToUpdate, struct MeshBoundingBox *itemUpdatingBb, vec3 offset){
    bbToUpdate->min[0] = MIN(bbToUpdate->min[0], itemUpdatingBb->min[0] + offset[0]);
    bbToUpdate->min[1] = MIN(bbToUpdate->min[1], itemUpdatingBb->min[1] + offset[1]);
    bbToUpdate->min[2] = MIN(bbToUpdate->min[2], itemUpdatingBb->min[2] + offset[2]);
                                                                       
    bbToUpdate->max[0] = MAX(bbToUpdate->max[0], itemUpdatingBb->max[0] + offset[0]);
    bbToUpdate->max[1] = MAX(bbToUpdate->max[1], itemUpdatingBb->max[1] + offset[1]);
    bbToUpdate->max[2] = MAX(bbToUpdate->max[2], itemUpdatingBb->max[2] + offset[2]);
}

void meshBoundingBoxPrint(struct MeshBoundingBox *bb){
    debugf("bounding box: [%f %f %f] [%f %f %f]\n", bb->min[0], bb->min[1], bb->min[2], bb->max[0], bb->max[1], bb->max[2]);
}

// void meshBoundingBoxDraw(enum Meshes id){
void meshBoundingBoxDraw(struct MeshBoundingBox *bb, struct Color color){
    if(!interface->showBoudningBox){
        return;
    }

    SET_COLOR(colorUniform, color);

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

uint8_t keyType[] = {
//  C          C#         D          D#         E
    WHITE_KEY, BLACK_KEY, WHITE_KEY, BLACK_KEY, WHITE_KEY,
//  F          F#         G          G#         A          A#         H
    WHITE_KEY, BLACK_KEY, WHITE_KEY, BLACK_KEY, WHITE_KEY, BLACK_KEY, WHITE_KEY
};

uint8_t getKeyType(uint8_t note){
    return keyType[note];
}

enum Meshes noteToMesh[] = {
    C, C_SHARP, D, D_SHARP, E, 
    F, F_SHARP, G, G_SHARP, A, A_SHARP, H,
};

enum Meshes pitchToNote(Pitch p){
    return noteToMesh[p % 12];
}

enum PianoNotes meshesNote[] = {
    C_NOTE, D_NOTE, E_NOTE, F_NOTE, G_NOTE, A_NOTE, H_NOTE,
    C_SHARP_NOTE, D_SHARP_NOTE, F_SHARP_NOTE, G_SHARP_NOTE, A_SHARP_NOTE
};

enum PianoNotes mesheNoteToNote(enum Meshes note){
    return meshesNote[note - C];
}

// TODO: keyboard to start and stop from any key
void computeKeyboard(struct Piano *piano, Pitch start, Pitch end){
    size_t whiteTrigCount = 0, blackTrigCount = 0;
    size_t linesTrigCount = 0;

    // TODO: keyboard, max height
    start -= (start % 12) - C_NOTE;
    // end += H - (;

    // uint8_t start = notePitchToPitch(s);
    // uint8_t end = notePitchToPitch(e);
    piano->keyboard.firstOctave = start / 12;
    uint8_t size = end - start + 1;
    debugf("start end: %i %i size %i\n", start, end, size);

    size_t whiteKeysMeshSize = 0, blackKeysMeshSize = 0;

    for(uint8_t i = 0; i < size; i++){
        uint8_t p = i + start;
        uint8_t note = p % 12;
        
        enum Meshes meshId = noteToMesh[note];
        size_t meshSize = piano->meshesDataSize[meshId];
        // debugf("%i %zu\n", meshId, meshSize);
        
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
    // debugf("%p %zu\n", whiteKeys, whiteKeysMeshSize);
    // debugf("%p %zu\n", blackKeys, blackKeysMeshSize);

    // clear the map
    memset(piano->keyboard.keysDataStart, 0, sizeof(size_t) * KEYBOARD_KEY_SIZE);
    
    float offset = MBB_MIN(D)[0] - MBB_MAX(C)[0];
    float octaveWidth = MBB_MAX(H)[0] + offset;
    piano->keyboard.octaveWidth = octaveWidth;

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
        // debugf("%zu %p %zu\n", *pointer, buffer, meshIndex);
        memcpy(&buffer[*pointer], &piano->keyboard.keys[meshIndex], sizeof(float) * meshSize);

        piano->keyboard.keysDataStart[p] = ((*pointer) / VERTEX_SIZE);

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
        // debugf("bufferIndex: %zu\n", bufferIndex);
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
    
    float scale = 2.0f / piano->keyboard.keyboardWidth;
    float keyboardHeight = MBB_MAX(C)[1] * scale * interface->g->screenRatio;
    float notesHeight = 2.0f - (piano->view.items[VIEW_ITEM_TYPE_SHEET].height + keyboardHeight);
    viewSet(&piano->view, VIEW_ITEM_TYPE_KEYBOARD, keyboardHeight);
    viewSet(&piano->view, VIEW_ITEM_TYPE_NOTES, notesHeight);
    debugf("keyboardHeight: %f notesHeight %f\n", keyboardHeight, notesHeight);
    viewRecalc(&piano->view);
}
