#include "midiPiano.h"

float timer = 0, speed = 1.2;
bool paused = false;

const uint8_t notations[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
const uint8_t whiteKeyPostion[12] = {0, 0, 1, 0, 2, 3, 0, 4, 0, 5, 0, 6};
float sizes[12];

uint8_t firstNote = 21, lastNote = 108;

size_t leftIndex = 0;
uint8_t pressedNotesSize = 0;
struct PressedNote pressedNotes[127];

float notePositions[127];
uint8_t whiteKeys = 52, blackKeys = 33;
float whiteKey, blackKey;

float noteCanvasScale = 0.6f;
float durationsDrawCount = 4;

GLint colorUniform;
struct Shader *shader;

struct Graphics *g = NULL;
struct Text *text = NULL;

#define UNUSED(x) (void)(x)

void scrollCallback(GLFWwindow *w, double x, double y){
    UNUSED(w);
    UNUSED(x);
    noteCanvasScale += (float)(y) * 0.1f * noteCanvasScale;
    if(noteCanvasScale < 0.05){
        noteCanvasScale = 0.05;
    }

    durationsDrawCount = 1.5f / noteCanvasScale;
}

void pianoInit(){
    g = graphicsInit();
    glfwSetScrollCallback(g->window, scrollCallback);
    text = textInit(&g->screenRatio);

    midiPrecalculateNotes();

    shader = shaderInit(VERTEX_SHADER, FRAGMENT_SHADER);
    GLint posAttrib = glGetAttribLocation(shader->program, "position");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);

    colorUniform = glGetUniformLocation(shader->program, "color");
}

void pianoExit(){
    graphicsExit();
}

void reCountNotes(){
    whiteKeys = 0;
    blackKeys = 0;
    for(uint8_t i = firstNote; i <= lastNote; i++){
        uint8_t note = i % 12;
        if(notations[note]){
            blackKeys++;
        }
        else{
            whiteKeys++;
        }
    }
}

void midiSelectRegion(struct MidiSong *song, uint8_t trackMask){
    uint8_t minNote = 0xff, maxNote = 0;
    for(size_t s = 0; s < MAX_TRACK_SIZE; s++){
        if(trackMask & (1 << s)){
            if(maxNote < song->tracksInfo[s]->maxNote){
                maxNote = song->tracksInfo[s]->maxNote;
            }
            if(song->tracksInfo[s]->minNote < minNote){
                minNote = song->tracksInfo[s]->minNote;
            }
        }
    }

    firstNote = minNote;
    lastNote = maxNote;
    printf("piano range: [%i %i]\n", firstNote, lastNote);
}

uint8_t notePosition(const char *noteName, int8_t octave){
    octave += 1;
    if(9 < octave){
        fprintf(stderr, "wrong number for octave %i\n", octave - 1);
    }

    uint8_t noteNameIndex = 0;
    for(; noteNameIndex < NUMBER_OF_NOTES; noteNameIndex++){
        if(strcmp(noteName, noteNames[noteNameIndex]) == 0){
            break;
        }
        if(noteNameIndex - 1 == NUMBER_OF_NOTES){
            fprintf(stderr, "note %s doesn't exist", noteName);
        }
    }

    return octave * 12 + noteNameIndex;
}

void midiPrecalculateNotes(){
    printf("first note name %i %s\n", firstNote, noteNames[firstNote % 12]);
    if(notations[firstNote % 12]){
        firstNote--;
    }
    printf("first note name %i\n", firstNote);
    reCountNotes();

    whiteKey = 1.0 / (float)whiteKeys;
    blackKey = whiteKey / 2.0;

    float blackKeyThird = blackKey / 3.0;

    for(int i = 0; i < NUMBER_OF_NOTES; i++){
        if(!notations[i]){
            if(i == C_NOTE || i == D_NOTE || i == E_NOTE || i == F_NOTE || i == H_NOTE){
                sizes[i] = whiteKey - blackKeyThird * 2.0;
            }
            else{
                sizes[i] = whiteKey - blackKeyThird - blackKey / 2.0;
            }
        }
        else{
            sizes[i] = blackKey;
        }
    }

    float prevPos = 0;
    notePositions[firstNote] = 0.0;
    uint8_t n = firstNote % 12;
    if(n == C_NOTE || n == F_NOTE){
        prevPos = whiteKey - blackKeyThird * 2.0;
    }
    else if(n == G_NOTE){
        prevPos = whiteKey - blackKey / 2.0;
    }
    else if(n == E_NOTE || n == H_NOTE){
        prevPos = whiteKey;
    }
    else if(n == D_NOTE || n == A_NOTE){
        prevPos = whiteKey - blackKeyThird;
    }

    for(uint8_t i = firstNote + 1; i <= lastNote; i++){
        uint8_t note = i % 12;
        
        notePositions[i] = prevPos;
        prevPos = notePositions[i] + sizes[note];
        // uint8_t octave = i / 12;
        // printf("%i %s %f\n", octave, noteNames[note], notePositions[i]);
    }
}

void drawPiano(uint8_t *noteBuffer, uint8_t notesNameMask){
    setWindow(-1.0f, -1.0f, 2.0f, 0.5f);
    SET_COLOR(colorUniform, WHITE);
    drawRectangle(FORMAT(0, 0, 1.0f, 1.0f));
    
    uint8_t blackNotes[127];
    uint8_t blackNotesSize = 0;
    SET_COLOR(colorUniform, GREEN);
    uint8_t noteIndex = 0;
    for(size_t i = firstNote; i <= lastNote; i++){
        uint8_t note = i % 12;

        if(notations[note]){
            blackNotes[blackNotesSize++] = i;
            continue;
        }
        if(!noteBuffer[i]){
            noteIndex++;
            continue;
        }
        
        float pos = (float)noteIndex * whiteKey;
        float w = whiteKey;
        
        drawRectangle(FORMAT(pos, 0, w, 1));
        noteIndex++;
    }

    uint8_t whiteKeyIndex = 0;
    useShader(text->shader);
    textColor(text, (struct TextColor){0, 0, 0});
    for(uint8_t n = firstNote; n <= lastNote; n++){
        uint8_t note = n % 12;
        if(notations[note]){
            continue;
        }
        if((1 << whiteKeyPostion[note]) & notesNameMask){
            float size = whiteKey * 0.5;
            textDraw(text, "C", (float)whiteKeyIndex * whiteKey - 1.0f, -1, size);
        }
        whiteKeyIndex++;
    }

    useShader(shader);
    SET_COLOR(colorUniform, BLACK);
    // draw notes edges on keyboard
    for(size_t i = 0; i < whiteKeys; i++){
        float pos = i * whiteKey;
        drawLine(FORMAT_2P(pos, 0.0f, pos, 1.0f));
    }

    for(uint8_t n = 0; n < blackNotesSize; n++){
        uint8_t i = blackNotes[n];

        if(noteBuffer[i]){
            SET_COLOR(colorUniform, RED);
        }

        float pos = notePositions[i];
        drawRectangle(FORMAT(pos, 0.4f, blackKey, 0.6f));

        if(noteBuffer[i]){
            SET_COLOR(colorUniform, BLACK);
        }
    }
}

void drawBars(float t){
    int measure = (int)t;
    float y = t - measure;
    for(int i = 0; i < (int)durationsDrawCount; i++){
        float yPos = ((float)i - y) * noteCanvasScale;
        useShader(text->shader);
        textColor(text, (struct TextColor){1, 1, 1});
        T_PRINTF(windowX(0.05f), windowY(yPos), windowWidth(0.02), "%i", measure + i);

        useShader(shader);
        SET_COLOR(colorUniform, WHITE);
        drawLine(FORMAT_2P(0, yPos, 2, yPos));
    }
}

// clear
// draw notes + draw bars
//
// clear piano area
// draw white pressed notes
// draw notes lines
// 
// draw all black notes
void midiDrawVisibleNotes(struct MidiSong *song, uint8_t *noteBuffer, uint8_t trackMask, uint8_t notesNameMask){
    setWindow(-1.f, -.5f, 2.0f, 1.5f);
    drawBars(timer);
    
    useShader(shader);
    SET_COLOR(colorUniform, WHITE);
    // draw notes edges on note canvas
    for(uint8_t noteIndex = firstNote; noteIndex <= lastNote; noteIndex++){
        uint8_t note = noteIndex % 12;
        if(note == C_NOTE || note == E_NOTE){
            float pos = notePositions[noteIndex];
            // drawLine(pos, 1.5, pos, 0);
            drawLine(FORMAT_2P(pos, 0, pos, 1));
        }
    }

    size_t i = leftIndex;
    while(i < song->notesArraySize){
        struct MidiNotesPressGroup *notes = song->notesArray[i];
        if(timer + durationsDrawCount < notes->timer){
            break;
        }
        
        for(size_t n = 0; n < notes->size; n++){
            uint8_t noteTrackMask = 1 << notes->notes[n]->trackIndex;
            if(!(noteTrackMask & trackMask)){
                continue;
            }

            uint8_t note = notes->notes[n]->type;
            float duration = notes->notes[n]->duration * noteCanvasScale;
            uint8_t noteName = note % 12;

            float pos = notePositions[note];
            float posY = (notes->timer - timer) * noteCanvasScale;
            float w = sizes[noteName];
            
            drawRectangle(FORMAT(pos, posY, w, duration));
        }

        i++;
    }

    drawPiano(noteBuffer, notesNameMask);
}

void turnOnNotes(struct MidiSong *song, int midiDevice, uint8_t *noteBuffer, uint8_t trackMask){
    size_t currIndex = leftIndex;
    while(currIndex < song->notesArraySize){
        struct MidiNotesPressGroup *notes = song->notesArray[currIndex];
        if(timer < notes->timer){
            break;
        }
        for(uint8_t i = 0; i < notes->size; i++){
            uint8_t noteTrackMask = 1 << notes->notes[i]->trackIndex;
            uint8_t note = notes->notes[i]->type;
            float end = notes->timer + notes->notes[i]->duration;
            // printf("%i %i %f %f\n", i, note, notes->timer, timer);
            if((noteTrackMask & trackMask) && !noteBuffer[note] && notes->timer < timer && timer < end){
                noteBuffer[note] = 1;
                pressedNotes[pressedNotesSize++] = (struct PressedNote){note, end};
                if(midiDevice){
                    sendNoteEvent(midiDevice, NOTE_ON, note, 60);
                }
            }
        }

        currIndex++;
    }
}

void turnOffNotes(int midiDevice, uint8_t *noteBuffer){
    for(uint8_t i = 0; i < pressedNotesSize; i++){
        if(pressedNotes[i].end < timer){
            struct PressedNote temp = pressedNotes[i];
            pressedNotes[i] = pressedNotes[pressedNotesSize - 1];
            pressedNotes[pressedNotesSize - 1] = temp;
            
            uint8_t note = pressedNotes[--pressedNotesSize].type;
            if(midiDevice){
                sendNoteEvent(midiDevice, NOTE_OFF, note, 0);
            }

            noteBuffer[note] = 0;
        }
    }
}

#define PREV_NOTE (1 << 15)
#define NEXT_NOTE (1 << 14)

uint16_t setTypeBit(int8_t j){
    switch(j){
        case -1:
            return PREV_NOTE;
        case 1:
            return NEXT_NOTE;
        default:
            return 0;
    }
}


// TODO: calculate error
uint8_t match(struct MidiSong *song, uint8_t *noteBuffer, uint8_t *userNoteBuffer, float *error){
    // uint16_t shouldBePressed[127] = {};
    // if(song->notesArraySize <= leftIndex){
    //     return 0;
    // }
    //
    // struct MidiNotesPressGroup *notes = song->notes[leftIndex];
    // 
    // // for(int8_t j = -1; j < 2; j++){
    //     // size_t nIndex = leftIndex + j;
    //     size_t nIndex = leftIndex;
    //     // uint16_t bit = setTypeBit(j);
    //     // if(0 <= nIndex && nIndex < song->notesArraySize){
    //         for(uint8_t i = 0; i < notes->size; i++){
    //             shouldBePressed[notes->notes[i]->type] = i;// | bit;
    //         }
    //     // }
    // // }
    // 
    uint8_t wrongNotes = 0;
    // for(int8_t i = 0; i < 127; i++){
    //     uint16_t noteState = shouldBePressed[i];
    //     if((0xff & noteState) != noteBuffer[i]){
    //
    //     // if((0xff & noteState) != noteBuffer[i]){
    //         wrongNotes++;
    //     }
    // }
    for(int8_t i = 0; i < 127; i++){
        if(userNoteBuffer[i] != noteBuffer[i]){
            wrongNotes++;
        }
    }

    return wrongNotes;
}

// TODO: add different type of modes
// TODO: add backtracking if enabled
// TODO: make loop mode, start from x to y measure
void midiLearnSong(struct MidiSong *song, int midiDevice, uint8_t mode, uint8_t trackMask, uint8_t notesNameMask){
    uint8_t userNoteBuffer[127] = {};
    uint8_t noteBuffer[127] = {};

    timer = -1.5;
    speed = 1;
    float prevTime = 0;
    float error = 0.0;

    while(isRunning()){
        glfwPollEvents();
        float currentTime = glfwGetTime();
        float alphaTime = (currentTime - prevTime) * speed;
        clear();

        midiRead(midiDevice, userNoteBuffer);
        turnOnNotes(song, 0, noteBuffer, trackMask);
        uint8_t wrongNotes = match(song, noteBuffer, userNoteBuffer, &error);
        // printf("%i\n", wrongNotes);
        if(!paused && !wrongNotes){
            timer += alphaTime;
        }
        // else{
        //     timer -= 1.0;
        //     struct MidiNotesPressGroup *notes = song->notesArray[leftIndex];
        //     while(leftIndex && timer <= notes->timer){
        //         notes = song->notesArray[--leftIndex];
        //     }
        // }
        turnOffNotes(0, noteBuffer);
        midiDrawVisibleNotes(song, noteBuffer, trackMask, notesNameMask);
        swap(g);

        prevTime = currentTime;
    }
}

void midiPlaySong(struct MidiSong *song, int midiDevice, uint8_t notesNameMask){
    const uint8_t trackMask = 0xff;
    uint8_t noteBuffer[127] = {};

    float prevTime = 0;

    while(isRunning()){
        glfwPollEvents();
        float currentTime = glfwGetTime();
        float alphaTime = (currentTime - prevTime) * speed;
        if(!paused){
            timer += alphaTime;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        if(song->notesArraySize <= leftIndex){
            printf("leftIndex to big %zu %zu\n", song->notesArraySize, leftIndex);
            return;
        }

        struct MidiNotesPressGroup *notes = song->notesArray[leftIndex];
        while(notes->timerEnd < timer){
            if(song->notesArraySize <= leftIndex + 1){
                return;
            }
            if(song->notesArray[leftIndex + 1]->timerEnd < timer){
                notes = song->notesArray[leftIndex++];
            }
            else{
                break;
            }
        }

        turnOnNotes(song, midiDevice, noteBuffer, trackMask);
        turnOffNotes(midiDevice, noteBuffer);
        midiDrawVisibleNotes(song, noteBuffer, trackMask, notesNameMask);
        swap(g);

        prevTime = currentTime;
    }
}
