#include "piano.h"

#include "interface.h"
extern struct Interface *interface;

extern GLuint elementArrayBuffer;

extern GLint globalMatUniform;
extern GLint localMatUniform;
extern GLint colorUniform;

extern struct MeshBoundingBox *meshBoundingBoxes;

void draw(struct Piano *piano, double percentage, enum KeyboardMode keyboardMode){
    glClear(GL_COLOR_BUFFER_BIT);
    
    drawNotes(piano, percentage);
    drawKeyboard(piano, keyboardMode);

    drawSheet(piano);

    glfwSwapBuffers(interface->g->window);
    processPollEvents();
    glfwPollEvents();
}

void drawSheet(struct Piano *piano){
    float stretch = 1.1f;
    float clearance = 0.7f;

    float offset = 0 + clearance;
    struct Sheet *sheet = piano->sheet;

    float scale = 0.008f;
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
    // TODO: exit the loop base on offset
    for(size_t m = piano->sheet->currMeasure; m < sheet->measuresSize && m < piano->sheet->currMeasure + 5; m++){
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
                    for(size_t p = 0; p < piano->playedNotesVector->size; p++){
                        // debugf("pn: %p\n", pressedNotes->data[p]);
                        struct Note *note = piano->playedNotesVector->data[p]->note;
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
}


void drawNotes(struct Piano *piano, double percentage){
    SET_COLOR(colorUniform, RED);
    mat4 globMatrix = {};
    vec3 globScale = {interface->scale, interface->scale, interface->scale};
    vec3 globPos = {interface->xPos, interface->yPos, 0};
    glm_mat4_identity(globMatrix);
    glm_scale(globMatrix, globScale);
    glm_translate(globMatrix, globPos);
    glUniformMatrix4fv(globalMatUniform, 1, GL_FALSE, (float*)globMatrix);

    float scale = 2.f / piano->keyboard.keyboardWidth;
    mat4 mat = {};
    glm_mat4_identity(mat);
    vec3 scaleVec = {scale, scale * interface->g->screenRatio, scale};
    glm_translate(mat, (vec3){-1, -1, 0});
    glm_translate_y(mat, MBB_MAX(C)[1] * scale * interface->g->screenRatio);
    glm_scale(mat, (vec3){1, 50, 1});
    glm_scale(mat, scaleVec);

    // piano->playedNotesVector
    float offset = 0;
    // float yScale = 0;
    for(size_t measureIndex = piano->sheet->currMeasure; measureIndex < piano->measureSize; measureIndex++){
        // size_t p = piano->sheet->currMeasure;
        struct Measure *measure = piano->measures[measureIndex];
        for(size_t s = 0; s < piano->sheet->staffNumber; s++){
            for(Division d = 0; d < measure->measureSize; d++){
                struct Notes *notes = measure->staffs[s][d];
                if(!notes){
                    continue;
                }

                for(size_t i = 0; i < notes->chordSize; i++){
                    struct Note *note = notes->chord[i];
                    if(GET_BIT(note->flags, NOTE_FLAG_REST)){
                        continue;
                    }
                    Pitch p = notePitchToPitch(&note->pitch);
                    enum Meshes meshId = NOTE_START + pitchToNote(p) - C;
                    float height = (float)note->duration / (float)measure->measureSize;
                    float y = (float)d / (float)measure->measureSize + offset - percentage;

                    float octaveOffset = (int)(p / 12) - piano->keyboard.firstOctave;
                    mat4 matrix = {};
                    glm_mat4_copy(mat, matrix);
                    glm_translate(matrix, (vec3){octaveOffset * piano->keyboard.octaveWidth, y, 0});
                    glm_scale(matrix, (vec3){1, height, 1});
                    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)matrix);
                    
                    GLint index = piano->meshesDataStart[meshId] / 3 + piano->meshesDataStartOffset;
                    size_t trigCount = piano->meshesDataSize[meshId] / 3;

                    glDrawArrays(GL_TRIANGLES, index, trigCount);
                    // enum Meshes note = ;
                    // measure->measureSize
                    // note->duration
                }
            }
        }
        offset += 1;
        if(15 + piano->sheet->currMeasure < measureIndex){
            return;
        }
    }
}

void drawKeyboard(struct Piano *piano, enum KeyboardMode keyboardMode){
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

    SET_COLOR(colorUniform, WHITE);
    glDrawArrays(GL_TRIANGLES, linesStartIndex, piano->keyboard.linesDataStartOffset);

    glm_mat4_identity(mat);

    vec3 scaleVec = {scale, scale * interface->g->screenRatio, scale};
    glm_translate(mat, (vec3){-1, -1, 0});
    glm_scale(mat, scaleVec);

    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mat);

    SET_COLOR(colorUniform, WHITE);
    glDrawArrays(GL_TRIANGLES, whiteStartIndex, piano->keyboard.whiteKeysTrigCount);
    SET_COLOR(colorUniform, GRAY);
    glDrawArrays(GL_TRIANGLES, blackStartIndex, piano->keyboard.blackKeysTrigCount);

    size_t size = (keyboardMode == KEYBOARD_PIANO_MODE) ? piano->pressedNotesVector->size : piano->playedNotesVector->size;
    for(size_t i = 0; i < size; i++){
        Pitch p = (keyboardMode == KEYBOARD_PIANO_MODE) ? 
            piano->pressedNotesVector->data[i].pitch : 
            notePitchToPitch(&piano->playedNotesVector->data[i]->note->pitch);

        size_t colorOffset = (getKeyType(p % 12) == WHITE_KEY) ? 
            piano->keyboard.whiteKeysDataStartOffset :
            piano->keyboard.blackKeysDataStartOffset;
        
        enum Meshes note = pitchToNote(p);
        size_t index = piano->keyboard.keysDataStart[p] + colorOffset;
        SET_COLOR(colorUniform, RED);
        glDrawArrays(GL_TRIANGLES, index, piano->meshesDataSize[note] / 3);
    }
}
