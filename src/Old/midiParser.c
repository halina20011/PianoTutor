#include "midiParser.h"

uint8_t size1[SIZE_OF_SIZE1] = {
    EVENT_PROGRAM_CHANGE, 
    EVENT_CHANNEL_PREASURE,
    EVENT_SONG_SELECT
};

uint8_t size2[SIZE_OF_SIZE2] = {
    EVENT_NOTE_OFF,
    EVENT_NOTE_ON,
    EVENT_POLYPHONIC_KEY_PREASSURE,
    EVENT_CONTROL_CHANGE,
    EVENT_PITCH_WHEEL_CHANGE,
    EVENT_CHANNEL_MODE_MESSAGE
};

uint8_t sizeTable[0xff] = {};

// VLV
uint32_t variableLengthValue(uint8_t *data, uint32_t *endOffset){
    uint32_t size = 0;
    size_t i = 0;
    char byte = 0;
    do{
        byte = data[i] & 0xFF;
        size = size << 7 | (byte & BIT_MASK_7) ;
        i++;
    } while(byte & (1 << 7));
    
    *endOffset += i;

    return size;
}

char *toBuffer(uint8_t *data, uint32_t size){
    char *buffer = malloc(sizeof(char) * size);
    char *bufferP = buffer;
    for(uint32_t i = 0; i < size; i++){
        *bufferP++ = data[i];
    }

    return buffer;
}

void formatBin(uint8_t val, char *buffer){
    size_t size = 0;
    while(val){
        buffer[size++] = (val & 1) ? '1' : '0';
        val >>= 1;
    }

    for(size_t i = size; i < 8; i++){
        buffer[size++] = '0';
    }

    for(uint8_t i = 0; i < 8/2; i++){
        uint8_t temp = buffer[i];
        buffer[i] = buffer[size - 1 - i];
        buffer[size - 1 - i] = temp;
    }

    buffer[8] = '\0';
}

void printData(char *data, size_t s){
    for(size_t i = 0; i < s; i++){
        printf("%02X ", data[i] & 0xff);
    }
    printf("\n");
}

// TODO: use binary search
// uint8_t getNote(float duration){
//     float minError = fabs(duration - noteDurations[0].duration);
//     uint8_t index = 0;
//     for(int i = 0; i < NUMBER_OF_NOTES * 2; i++){
//         if(duration == noteDurations[i].duration){
//             return index;
//         }
//
//         float error = fabs(duration - noteDurations[i].duration);
//         if(error < minError){
//             minError = error;
//             index = i;
//             printf("error: %f\n", error);
//         }
//     }
//
//     return index;
// }

struct MidiNotePressGroup *noteInit(uint8_t type, uint32_t start, float duration, uint8_t trackIndex){
    struct MidiNotePressGroup *note = malloc(sizeof(struct MidiNotePressGroup));
    note->type = type;
    note->start = start;
    note->duration = duration;

    note->trackIndex = trackIndex;

    return note;
}

struct MidiNotesPressGroup *notesPressGroupInit(struct MidiNotePressGroup **n, size_t size, float timer, uint32_t BPM, uint8_t numerator, uint8_t denominator){
    struct MidiNotesPressGroup *npg = malloc(sizeof(struct MidiNotesPressGroup));
    
    size_t bufferSize = sizeof(struct MidiNotePressGroup) * size;
    struct MidiNotePressGroup **note = malloc(bufferSize);
    // float maxDuration = 0;
    // for(uint8_t i = 0; i < size; i++){
    //     if(maxDuration < n[i]->duration){
    //         maxDuration = n[i]->duration;
    //     }
    //     note[i] = n[i];
    // }
    memcpy(note, n, bufferSize);

    npg->notes = note;
    npg->size = size;
    npg->timer = timer;

    npg->BPM = BPM;
    npg->numerator = numerator;
    npg->denominator = denominator;

    return npg;
}

// int comp(const void *a, const void *b){
//     const float x = (*(struct NoteDuration*)a).duration;
//     const float y = (*(struct NoteDuration*)b).duration;
//     return (y < x) - (x < y);
// }
//
// void calculateNoteDurations(){
//     int offset = 2;
//     for(int i = 0; i < NUMBER_OF_NOTES; i++){
//         float duration = pow(2, - i + offset);
//         float durationDot = duration * 3/2;
//         printf("%i %f %f\n", i, duration, durationDot);
//         noteDurations[i * 2 + 0] = (struct NoteDuration){duration, i * 2};
//         noteDurations[i * 2 + 1] = (struct NoteDuration){durationDot, i * 2 + 1};
//     }
//
//     qsort(noteDurations, NUMBER_OF_NOTES * 2, sizeof(struct NoteDuration), comp);
//     // for(int i = 0; i < NUMBER_OF_NOTES * 2; i++){
//     //     printf("%u %f\n", noteDurations[i].noteValue, noteDurations[i].duration);
//     // }
// }

// create a singly linked list for the sequence of notes
// they are grooped by the time when they are pressed
struct List *parseTrack(uint8_t *data, uint32_t dataSize, uint32_t PPQ, uint8_t trackIndex, struct MidiTrackInfo **returnTrackInfo){
    uint32_t dataIndex = 0;
    uint8_t prevStatus = 0;
    uint8_t n = 4, d = 4, c = 24, b = 8;
    
    uint32_t timer = 0;
    uint32_t BPM = BMP_UNSET;

    struct MidiNotePressGroup *noteTable[NOTE_BUFFER_SIZE];

    struct MidiNotePressGroup *notesMagazine[NOTE_BUFFER_SIZE];
    size_t notesMagazineSize = 0;

    struct List *notesPressGroupsList = newSinglyLinkedList();

    struct MidiTrackInfo *trackInfo = malloc(sizeof(struct MidiTrackInfo));
    
    trackInfo->minNote = 0xff;
    trackInfo->maxNote = 0;

     // uint32_t prevDelta = 0;
    while(dataIndex < dataSize){
        uint32_t deltaTime = variableLengthValue(data + dataIndex, &dataIndex);
        // if(deltaTime != 0){
        //     printf("deltaTime %u\n", deltaTime);
        // }
        
        uint8_t trackType = data[dataIndex++] & 0xFF;
        timer += deltaTime;

        // handle Meta events
        if(trackType == 0xFF){
            uint8_t metaType = data[dataIndex++];
            uint32_t metaDataLength = variableLengthValue(data + dataIndex, &dataIndex);
            printf("meta[%i]: %02X => ", metaDataLength, metaType & 0xff);
            char *dataBuffer = toBuffer(data + dataIndex, metaDataLength);
            printData(dataBuffer, metaDataLength);
            dataIndex += metaDataLength;
            
            if(metaType == META_TRACK_NAME){
                printf("meta track name[%i]\n", metaDataLength);
            }
            else if(metaType == META_PREFIX_PORT){
                printf("meta prefix port %i\n", 0);
            }
            else if(metaType == META_MIDI_PORT){
                printf("using midi port: %i\n", 0);
            }
            else if(metaType == META_SET_TEMPO){
                CHECK_META_DATA_SIZE(META_SET_TEMPO_SIZE);
                // in microsecconds per quater note
                uint32_t t = BUFFER_TO_24(dataBuffer);
                BPM = (double)(60 * 1000000) / (double)t;
                printf("new BPM: %u\n", BPM);
            }
            else if(metaType == META_TIME_SIGNATURE){
                CHECK_META_DATA_SIZE(META_TIME_SIGNATURE_SIZE);
                n = dataBuffer[0]; // number of beats per measure
                d = 1 << dataBuffer[1]; // represents beat units as power of 2
                c = dataBuffer[2]; // midi clocks per metronome click
                b = dataBuffer[3]; // number of 32nd note in 4nd note
                float fTimer = (float)timer / (float)PPQ;
                printf("%f %u/%u c:%u b:%u\n", fTimer, n, d, c, b);
                // calculateNoteDurations()
            }
            else if(metaType == META_KEY_SIGNATURE){
                CHECK_META_DATA_SIZE(META_KEY_SIGNATURE_SIZE);
                uint8_t sf = dataBuffer[0];
                uint8_t mi = dataBuffer[1];
                printf("meta key signature: %i %i\n", sf, mi);
            }
            else if(metaType == META_KEY_END_OF_TRACK){
                printf("end of track\n");
            }
            else{
                printf("not found meta type %02X\n", metaType);
            }
            free(dataBuffer);
        }
        else{
            uint8_t messageStatus = trackType;
            uint8_t dataByte[2];
            
            // detected running status
            if((messageStatus & (1 << 7)) == 0){
                dataIndex--;
                messageStatus = prevStatus;
            }

            uint32_t dataByteSize = sizeTable[messageStatus];
            for(uint32_t i = 0; i < dataByteSize; i++){
                dataByte[i] = data[dataIndex++] & BIT_MASK_7;
            }
            
            uint8_t messageType = messageStatus & (BIT_MASK_4 << 4);

            if(messageType == EVENT_NOTE_ON || messageType == EVENT_NOTE_OFF){
                uint8_t noteType = dataByte[0];
                uint8_t velocity = (messageType == EVENT_NOTE_ON) ? dataByte[1] : 0;
                // int8_t octave = noteType / 12;
                // uint8_t noteName = noteType % 12;
                if(!velocity){
                    messageType = EVENT_NOTE_OFF;
                }
                
                // if deltaTime is not 0 and notesMagazine is not empty
                // group the notes into one notePress 
                if(deltaTime && notesMagazineSize){
                    // printf("magazine: ");
                    // for(size_t i = 0; i < notesMagazineSize; i++){
                    //     printf("%i ", notesMagazine[i]->type);
                    // }
                    // printf("\n");
                    
                    if(noteType < trackInfo->minNote){
                        trackInfo->minNote = noteType;
                    }
                    if(trackInfo->maxNote < noteType){
                        trackInfo->maxNote = noteType;
                    }

                    float notesTimer = (float)notesMagazine[0]->start / (float)PPQ;
                    struct MidiNotesPressGroup *notesPressGroup = notesPressGroupInit(notesMagazine, notesMagazineSize, notesTimer, BPM, n, d);
                    notesMagazineSize = 0;
                    listInsert(notesPressGroupsList, notesPressGroup);
                }

                // printf("note[%i] %i %s vel: %i ch %i\n", noteType, octave, noteNames[noteName], velocity, chanel);

                if(messageType == EVENT_NOTE_OFF){
                    struct MidiNotePressGroup *note = noteTable[noteType];
                    note->duration = (timer - note->start) / (float)PPQ;
                    note->ticks = timer - note->start;
                    printf("%i\n", note->ticks);
                }
                else{
                    struct MidiNotePressGroup *note = noteInit(noteType, timer, -1, trackIndex);
                    noteTable[noteType] = note;

                    notesMagazine[notesMagazineSize++] = note;
                    // printf("magazine %zu\n", notesMagazineSize);
                }
                
                // addEvent(list, velocity, timer, PPQ);
            }

            // char *dataBuffer = toBuffer(data + i, offset);
            // printData(dataBuffer, offset);
            // free(dataBuffer);
            prevStatus = messageStatus;
            // prevDelta = deltaTime;
        }
    }

    trackInfo->n = n;
    trackInfo->d = d; 
    trackInfo->c = c;
    trackInfo->b = b;

    *returnTrackInfo = trackInfo;
    printf("track info: %i %i\n", trackInfo->minNote, trackInfo->maxNote);
    
    // struct Node *curr = song->head;
    // while(curr){
    //     struct MidiNotesPressGroup *notes = curr->val;
    //     printf("%i %zu => ", notes->timer, notes->size);
    //     for(size_t i = 0; i < notes->size; i++){
    //         printf("%i ", notes->notes[i]->type);
    //     }
    //     curr = curr->next;
    //     printf("\n");
    // }

    // for(uint8_t i = 0; i < NOTE_BUFFER_SIZE; i++){
    //     struct Vector *v = notePresses[i]->presses;
    //     if(!v->size){
    //         continue;
    //     }
    //     printf("%s[%i] :", noteNames[i % 12], i / 12);
    //     for(size_t j = 0; j < v->size; j++){
    //         struct NotePress *notePress = v->data[j];
    //         printf("[%i:%i] ", notePress->start, notePress->end);
    //     }
    //     printf("\n");
    // }
    
    return notesPressGroupsList;
}

struct MidiNotesPressGroup **mergeTracks(struct List **tracks, size_t tracksSize, size_t *nSize){
    struct Node **currs = malloc(sizeof(struct Node*) * tracksSize);
    size_t notesSize = 0;
    for(size_t i = 0; i < tracksSize; i++){
        currs[i] = tracks[i]->head;
        notesSize += tracks[i]->size;
    }

    struct MidiNotesPressGroup **allNotes = malloc(sizeof(struct MidiNotesPressGroup*) * notesSize);

    size_t size = 0;
    while(1){
        // find all tracks where curr head (MidiNotesPressGroup) has maximum timer
        float maxVal = -1;
        size_t minIndex;
        
        size_t tracksStack[MAX_TRACK_SIZE];
        size_t sameSize = 0;
        size_t mergedNotesSize = 0;
        uint32_t BMP = BMP_UNSET;
        for(size_t i = 0; i < tracksSize; i++){
            if(currs[i]){
                struct MidiNotesPressGroup *npg = currs[i]->val;
                float timer = npg->timer;
                if(maxVal < timer){
                    minIndex = i;
                    maxVal = timer;
                    sameSize = 0;
                    mergedNotesSize = 0;
                }
                if(timer == maxVal){
                    tracksStack[sameSize++] = i;
                    mergedNotesSize += npg->size;
                    if(BMP == BMP_UNSET && npg->BPM != BMP_UNSET){
                        BMP = npg->BPM;
                    }
                }
            }
        }

        if(maxVal == -1){
            break;
        }
        
        // if there are more NotesPressGroups then merged them together
        struct MidiNotesPressGroup *curr = currs[minIndex]->val;
        curr->BPM = BMP;
        // printf("BMP: %i\n", BMP);
        if(1 < sameSize){
            struct MidiNotePressGroup **mergedNotes = malloc(sizeof(struct MidiNotePressGroup) * mergedNotesSize);
            size_t noteIndex = 0;
            for(size_t i = 0; i < sameSize; i++){
                minIndex = tracksStack[i];
                struct MidiNotesPressGroup *notes = currs[minIndex]->val;
                struct MidiNotePressGroup **note = notes->notes;
                for(size_t n = 0; n < notes->size; n++){
                    mergedNotes[noteIndex++] = note[n];
                }
            }
            
            curr->notes = mergedNotes;
            curr->size = mergedNotesSize;
        }

        // calculate the maxDuration
        float maxDuration = 0;
        for(uint8_t i = 0; i < curr->size; i++){
            if(maxDuration < curr->notes[i]->duration){
                maxDuration = curr->notes[i]->duration;
            }
        }
        
        curr->timerEnd = curr->timer + maxDuration;
        allNotes[size++] = curr;
        
        // move all the heads with maximum timer to next item
        for(uint8_t i = 0; i < sameSize; i++){
            currs[tracksStack[i]] = currs[tracksStack[i]]->next;
        }
    }
    // printf("%zu %zu\n", size, notesSize);

    for(size_t i = 0; i < size / 2; i++){
        struct MidiNotesPressGroup *temp = allNotes[i];
        allNotes[i] = allNotes[size - i - 1];
        allNotes[size - i - 1] = temp;
    }

    // for(size_t i = 0; i < size; i++){
    //     struct MidiNotePressGroup **nn = allNotes[i]->notes;
    //     printf("%zu %f => ", i, allNotes[i]->timer);
    //     for(size_t j = 0; j < allNotes[i]->size; j++){
    //         printf("%s ", noteNames[nn[j]->type %12]);
    //     }
    //     printf("\n");
    // }

    *nSize = size;

    return allNotes;
}

void songPrint(struct MidiSong *song){
    printf("song size: %zu\n", song->notesArraySize);
    for(size_t i = 0; i < song->notesArraySize; i++){
        struct MidiNotesPressGroup *n = song->notesArray[i];
        printf("measure[%zu] timer: %f\n", i, n->timer);
        for(size_t j = 0; j < n->size; j++){
            struct MidiNotePressGroup *note = n->notes[j];
            printf("%f ", note->duration);
        }
        printf("\n");
    }
}

void printTrack(struct List *track){
    struct Node *n = track->head;
    while(n){
        struct MidiNotesPressGroup *npg = n->val;
        printf("timer: %f\n", npg->timer);

        for(size_t s = 0; s < npg->size; s++){
            struct MidiNotePressGroup *note = npg->notes[s];
            if(note->duration < 0.0001){
                printf("exit\n");
                return;
            }
            printf("%s %f\n", noteNames[note->type % 12], note->duration);
        }
        // printf("\n");

        n = n->next;
    }
}

void printTracks(struct List **tracks, size_t size){
    for(size_t i = 0; i < size; i++){
        printTrack(tracks[i]);
    }
}

void makeTable(uint32_t ){

}

struct MidiSong *midiParser(const char *filePath){
    for(int i = 0; i < SIZE_OF_SIZE1; i++){
        sizeTable[size1[i]] = 1;
    }

    for(int i = 0; i < SIZE_OF_SIZE2; i++){
        sizeTable[size2[i]] = 2;
    }

    FILE *file = fopen(filePath, "rb");
    if(!file){
        fprintf(stderr, "failed to open midi file: \"%s\"\n", filePath);
        exit(1);
    }

    char name[4];
    uint32_t PPQ = 480;
    
    struct List *tracks = newSinglyLinkedList();
    struct MidiSong *song = malloc(sizeof(struct MidiSong));

    uint8_t trackIndex = 0;
    // name(4 bytes):size(4 bytes):data(size bytes)
    while(fread(name, sizeof(char) * 4, 1, file) == 1){
        char buffer[4];
        fread(buffer, sizeof(uint8_t) * 4, 1, file);
        uint32_t dataSize = BUFFER_TO_32(buffer);
        uint8_t *data = malloc(sizeof(uint8_t) * dataSize);
        fread(data, sizeof(uint8_t) * dataSize, 1, file);
        // printf("%s %u\n", name, dataSize);
        if(strcmp(name, "MThd") == 0){
            if(dataSize != 6){
                fprintf(stderr, "header has wrong size\n");
                exit(1);
            }
            uint16_t format =   BUFFER_TO_16((data + 0));
            uint16_t ntrks =    BUFFER_TO_16((data + 2));
            uint16_t division = BUFFER_TO_16((data + 4));
            // printf("header: %i %i %i\n", format, ntrks, division);

            uint8_t bit15 = (division & (1 << 15)) != 0;
            if(bit15 == 0){
                // PPQ ="ticks per quater note" / "parts per quater"
                PPQ = division & ~(1 << 15);
                printf("PPQ: %i\n", PPQ);
            }
            else{
                uint8_t second = (division & ((BIT_MASK_7) << 7));
                uint8_t first = (division & BIT_MASK_7);
                fprintf(stderr, "SMPTE not implemented\n");
                exit(1);
                // printf("%i|%i|%i|\n", bit15, second, first);
            }
        }
        else if(strcmp(name, "MTrk") == 0){
            struct MidiTrackInfo *trackInfo = NULL;
            struct List *track = parseTrack(data, dataSize, PPQ, trackIndex, &trackInfo);
            song->tracksInfo[trackIndex++] = trackInfo;
            // printf("track p: %p\n", track);
            listPush(tracks, track);
        }
        free(data);
    }
    printf("\n");
    song->trackSize = trackIndex;
    
    size_t tSize = tracks->size;
    struct List **tracksArray = malloc(sizeof(struct List*) * tSize);
    struct Node *curr = tracks->head;
    for(size_t i = 0; curr; i++){
        struct List *l = curr->val;
        // printf("size %zu\n", l->size);
        // printf("track p: %p\n", l->head->val);
        tracksArray[i] = l;
        curr = curr->next;
    }

    // printTracks(tracksArray, tSize);

    // song->PPQ = PPQ;
    song->notesArray = mergeTracks(tracksArray, tSize, &song->notesArraySize);
    song->PPQ = PPQ;

    // songPrint(song);

    return song;
}

struct MidiSong *generateSong(){
    struct MidiSong *song = malloc(sizeof(struct MidiSong));

    struct MidiNotesPressGroup *notes = malloc(sizeof(struct MidiNotesPressGroup));
    notes->timer = 0;
    // notes->trackNumber = 0;
    
    uint8_t notations[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
    struct MidiNotePressGroup **n = malloc(sizeof(struct Note*) * 127);
    uint8_t size = 0;
    for(uint8_t i = 0; i < 127; i++){
        uint8_t note = i % 12;
        if(!notations[note]){
            struct MidiNotePressGroup *newNote = malloc(sizeof(struct MidiNotePressGroup));
            newNote->type = i;
            newNote->start = 0;
            newNote->duration = 1.0;
            n[size++] = newNote;
        }
    }
    notes->notes = n;
    notes->size = size;

    song->notesArray = malloc(sizeof(struct MidiNotesPressGroup*));
    song->notesArray[0] = notes;
    song->notesArraySize = 1;

    return song;
}
