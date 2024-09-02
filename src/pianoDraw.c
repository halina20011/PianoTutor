#include "piano.h"

#include "interface.h"
extern struct Interface *interface;

extern GLuint elementArrayBuffer;

extern GLint globalMatUniform;
extern GLint localMatUniform;
extern GLint viewMatUniform;
extern GLint colorUniform;

extern struct MeshBoundingBox *meshBoundingBoxes;

void draw(struct Piano *piano, double percentage, enum KeyboardMode keyboardMode){
    glClear(GL_COLOR_BUFFER_BIT);
    
    viewReset();
    
    mat4 globMatrix = {};
    vec3 globScale = {interface->scale, interface->scale, interface->scale};
    vec3 globPos = {interface->xPos, interface->yPos, 0};
    glm_mat4_identity(globMatrix);
    glm_scale(globMatrix, globScale);
    glm_translate(globMatrix, globPos);
    glUniformMatrix4fv(globalMatUniform, 1, GL_FALSE, (float*)globMatrix);

    drawNotes(piano, percentage);
    drawKeyboard(piano, keyboardMode);

    drawSheet(piano);

    glfwSwapBuffers(interface->g->window);
    processPollEvents();
    glfwPollEvents();
}

void drawSheet(struct Piano *piano){
    float clearance = 0.7f;

    float offset = 0 + clearance;
    struct Sheet *sheet = piano->sheet;

    // float scale = 0.05f;
    float scale = 1.0f / sheet->height;
    // glEnable(GL_SCISSOR_TEST);
    // // if(interface->g->xPos < interface->g->width
    // glScissor(interface->xPos, -interface->yPos + interface->g->height - 100, interface->g->width, 120);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glDisable(GL_SCISSOR_TEST);

    mat4 clearMat = {};
    glm_mat4_identity(clearMat);
    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)clearMat);

    viewUse(&piano->view, VIEW_ITEM_TYPE_SHEET);

    float yOffset = sheet->height / 2.0f;
    vec3 cursor = {-0.8f, yOffset * scale, 0};
    float lastStaffOffset = piano->sheet->staffOffsets[piano->sheet->staffNumber - 1];
    float barHeight = lastStaffOffset + (piano->sheet->staffNumber - 1) * 4.f;

    vec3 scaleVec = {scale, scale * interface->g->screenRatio, scale};
    mat4 cleanMat = {};
    glm_mat4_identity(cleanMat);
    
    // float __stretch = 2.0f;
    // glm_scale(cleanMat, (vec3){__stretch, 1, 1}); // scale to the needed stretch
    glm_translate(cleanMat, cursor);
    // glm_scale(cleanMat, (vec3){1.0f / __stretch, 1, 1}); // scale back to not affect width
    // glm_scale(cleanMat, (vec3){__stretch, 1, 1}); // scale to the needed stretch

    glm_scale(cleanMat, scaleVec);
    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)cleanMat);

    // for(size_t m = 0; m < 0; m++){
    // TODO: exit the loop base on offset
    for(size_t m = piano->sheet->currMeasure; m < sheet->measuresSize && m < piano->sheet->currMeasure + 5; m++){
        struct Measure *measure = piano->measures[m];
        struct ItemMeasure *itemMeasure = sheet->measures[m];
        size_t itemsSize = itemMeasure->size;
        
        // mat4 measureMat = {};
        // glm_mat4_copy(cleanMat, measureMat);
        // glm_translate(measureMat, (vec3){offset, 0, 0});
        // glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)measureMat);
        // meshBoundingBoxDraw(&measure->boundingBox, PURPLE);
        
        SET_COLOR(colorUniform, GREEN);
        for(StaffNumber s = 0; s < measure->stavesNumber; s++){
            mat4 mMat = {};
            glm_mat4_copy(cleanMat, mMat);
            float staffOffset = sheet->staffOffsets[s];
            vec3 pos = {offset, -staffOffset, 0};
            glm_translate(mMat, pos);
            glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mMat);
            
            // meshBoundingBoxPrint(&measure->boundingBox);
            meshBoundingBoxDraw(&measure->boundingBoxes[s], GREEN);
        }

        for(size_t i = 0; i < itemsSize; i++){
            struct Item *item = itemMeasure->items[i];
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
                
                struct ItemMesh *iM = item->data;

                size_t trigCount = piano->meshesDataSize[meshId] / 3;
                GLint index = piano->meshesDataStart[meshId] / 3 + vertexBufferGetPosition(VERTEX_BUFFER_MESHES);
                
                mat4 mMat = {};
                glm_mat4_copy(cleanMat, mMat);
                vec3 pos = {offset + iM->xPosition, iM->yPosition - staffOffset, 0};
                glm_translate(mMat, pos);

                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mMat);

                // debugf("mesh id: %i %i %zu\n", meshId, index, trigCount);
                glDrawArrays(GL_TRIANGLES, index, trigCount);

                if(playedNote == true){
                    SET_COLOR(colorUniform, WHITE);
                }

                meshBoundingBoxDraw(&meshBoundingBoxes[item->meshId], GREEN);
            }
            else if(item->type == ITEM_BAR){

            }
            else if(item->type == ITEM_STEM){
                //useShader(interface->shader);
                // staffOffset
                struct ItemStem *itemStem = item->data;
                // itemStem->
                float x = (itemStem->xStart + offset) + itemStem->noteOffset;

                float y1 = itemStem->y1 - staffOffset;
                float y2 = itemStem->y2 - staffOffset;
                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)cleanMat);

                drawLine(x, y1, 0, x, y2, 0);
            }
            else if(item->type == ITEM_BEAM){
                //useShader(interface->shader);

                struct ItemBeam *beam = item->data;
                float x1 = (beam->xStart + offset);
                float x2 = (beam->xEnd   + offset);
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
                mat4 mMat = {};
                glm_mat4_copy(cleanMat, mMat);
                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mMat);

                struct ItemLine *line = item->data;
                float x1 = (line->x + offset);
                float x2 = ((line->x + line->width) + offset);
                float y = line->y - staffOffset;

                glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)cleanMat);
                drawLine(x1, y, 0, x2, y, 0);
            }
            else if(item->type == ITEM_FLAG){
                SET_COLOR(colorUniform, WHITE);

                size_t trigCount = piano->meshesDataSize[meshId] / 3;
                GLint index = piano->meshesDataStart[meshId] / 3;
                
                struct ItemFlag *flag = item->data;

                mat4 mMat = {};
                glm_mat4_copy(cleanMat, mMat);
                vec3 pos = {(offset + flag->xPosition) + flag->width, flag->yPosition - staffOffset, 0};
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

        offset += itemMeasure->width * SHEET_STRETCH;
        float x = offset;
        drawLine(x, 4.f, 0, x, - barHeight, 1);
        offset += clearance;
    }

    // draw staffs lines
    for(StaffNumber s = 0; s < sheet->staffNumber; s++){
        float staffOffset = sheet->staffOffsets[s];
        for(int8_t yOffset = -2; yOffset <= 2; yOffset++){
            float y = yOffset * 2.0f + staffOffset;
            drawLine(0, -y, 0, offset, -y, 0);
        }
    }
}

void drawNotes(struct Piano *piano, double percentage){
    mat4 mat = {};
    glm_mat4_identity(mat);
    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mat);

    viewUse(&piano->view, VIEW_ITEM_TYPE_NOTES);

    float scale = 2.0f / piano->keyboard.keyboardWidth;

    size_t linesStartIndex = vertexBufferGetPosition(VERTEX_BUFFER_LINES);

    float scaleY = 2.0f / MBB_MAX(LINES)[1];
    float scaleX = 1.0f / (piano->view.items[VIEW_ITEM_TYPE_NOTES].height / 2.0f);
    vec3 scaleLineVec = {scaleX * scale, scaleY, scale};
    glm_translate(mat, (vec3){-1, -1, 0});
    glm_scale(mat, scaleLineVec);

    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mat);

    SET_COLOR(colorUniform, WHITE);
    glDrawArrays(GL_TRIANGLES, linesStartIndex, vertexBufferGetSize(VERTEX_BUFFER_LINES));

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
                    
                    // GLint index = piano->meshesDataStart[meshId] / 3 + piano->meshesDataStartOffset;
                    GLint index = piano->meshesDataStart[meshId] / 3 + vertexBufferGetPosition(VERTEX_BUFFER_MESHES);
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
    GLint whiteStartIndex = vertexBufferGetPosition(VERTEX_BUFFER_WHITE_KEYS);
    GLint blackStartIndex = vertexBufferGetPosition(VERTEX_BUFFER_BLACK_KEYS);

    mat4 mat = {};
    glm_mat4_identity(mat);
    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mat);

    viewUse(&piano->view, VIEW_ITEM_TYPE_KEYBOARD);

    float scale = 2.0f / piano->keyboard.keyboardWidth;

    float scaleY = 2.0f / (MBB_MAX(C)[1]);
    float scaleX = 1.0f / (piano->view.items[VIEW_ITEM_TYPE_KEYBOARD].height / 2.0f);
    vec3 scaleLineVec = {scaleX * scale, scaleY, scale};
    glm_translate(mat, (vec3){-1, -1, 0});
    glm_scale(mat, scaleLineVec);

    glUniformMatrix4fv(localMatUniform, 1, GL_FALSE, (float*)mat);

    SET_COLOR(colorUniform, WHITE);
    glDrawArrays(GL_TRIANGLES, whiteStartIndex, (GLint)vertexBufferGetSize(VERTEX_BUFFER_WHITE_KEYS));
    SET_COLOR(colorUniform, GRAY);
    glDrawArrays(GL_TRIANGLES, blackStartIndex, (GLint)vertexBufferGetSize(VERTEX_BUFFER_BLACK_KEYS));

    size_t size = (keyboardMode == KEYBOARD_PIANO_MODE) ? piano->pressedNotesVector->size : piano->playedNotesVector->size;
    for(size_t i = 0; i < size; i++){
        Pitch p = (keyboardMode == KEYBOARD_PIANO_MODE) ? 
            piano->pressedNotesVector->data[i].pitch : 
            notePitchToPitch(&piano->playedNotesVector->data[i]->note->pitch);

        GLint colorOffset = (getKeyType(p % 12) == WHITE_KEY) ? 
            vertexBufferGetPosition(VERTEX_BUFFER_WHITE_KEYS) :
            vertexBufferGetPosition(VERTEX_BUFFER_BLACK_KEYS);
        
        enum Meshes note = pitchToNote(p);
        GLint index = piano->keyboard.keysDataStart[p] + colorOffset;
        SET_COLOR(colorUniform, RED);
        glDrawArrays(GL_TRIANGLES, index, piano->meshesDataSize[note] / 3);
    }
}
