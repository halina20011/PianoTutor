#include "sheets.h"

// #define NTH_BYTE(num, p) (((num & (1 << p)) != 0))
//
// #define PRINT_8(n) (printf("%i %i %i %i %i %i %i %i"),\
//         NTH_BYTE(n, 7),\
//         NTH_BYTE(n, 6),\
//         NTH_BYTE(n, 5),\
//         NTH_BYTE(n, 4),\
//         NTH_BYTE(n, 3),\
//         NTH_BYTE(n, 2),\
//         NTH_BYTE(n, 1),\
//         NTH_BYTE(n, 0)) 

void format8(uint8_t n, char *buffer){
    uint8_t i = 0;
    for(int i = 0; i < 8; i++){
        buffer[i] = '0';
    }

    while(n){
        buffer[7 - i++] = (n & 1) + '0';
        n >>= 1;
    }

    buffer[8] = 0;
}

void printNoteDurations(struct NoteDuration *noteDurations, size_t size){
    for(size_t i = 0; i < size; i++){
        if(noteDurations[i].duration){
            char buffer[10];
            format8(noteDurations[i].flags, buffer);
            // format8(0b1, buffer);
            printf("%f %i %s\n", noteDurations[i].duration, noteDurations[i].note, buffer);
        }
    }
}

struct NoteDuration *generateNotes(int percision, size_t *rSize){
    size_t size = ((8 * 1.5 + 1) * percision);
    *rSize = size;
    struct NoteDuration *table = calloc(size, sizeof(struct NoteDuration));

    for(uint8_t t = 0; t < 4; t++){
        for(uint8_t i = 0; i < NOTES_TYPE_COUNT; i++){
            double full = 8 / pow(2, i);
            double half = full / 2;
            double dot = full * 1.5;
            double triplet = (full * 2) / 3;

            double arr[] = {full, half, dot, triplet};

            float duration = arr[t];
            int hash = (duration * (float)percision);
            if(!table[hash].duration){
                table[hash] = (struct NoteDuration){duration, i, 1 << t};
            }
        }
    }

    return table;
}

void correctDurations(char *correctionFilePath, struct SongInfo *songInfo){
    FILE *file = fopen(correctionFilePath, "r");
    if(!file){
        fprintf(stderr, "failed to open correction file \"%s\"\n", correctionFilePath);
    }
    
    char line[MAX_CORRECTION_FILE_LINE_SIZE + 1];

    // get numer of lines
    while(fgets(line, MAX_CORRECTION_FILE_LINE_SIZE, file)){
        bool comment = false;
        uint8_t match = 0;
        for(int i = 0; line[i]; i++){
            if(line[i] == '/'){
                if(++match == 2){
                    comment = true;
                }
            }
            else if(line[i] != ' '){
                break;
            }
        }

        if(comment){
            continue;
        }

        float dur1, dur2;
        if(sscanf(line, "%f,%f", &dur1, &dur2) == 2){
            int hash = (dur1 * (float)songInfo->percision);
            songInfo->noteInfo[hash].correctDuration = dur2;
            // printf("%f corrected to %f\n", dur1, dur2);
        }
        // else{
        //     printf("%f doesn't have correct value\n", dur1);
        // }
        // printf("%i %s", i++, line);
    }
}

void flushMeasure(struct Measure *m, uint8_t trackSize, struct Note **stack, size_t *stackSize, size_t *eachTrackNoteCount){
    // printf("measure has %zu notes\n", *stackSize);
    size_t eachTrackNoteIndex[MAX_TRACK_SIZE] = {};

    // allocate for eatch track the count of notes that it will have
    // printf("track size %i\n", trackSize);
    struct NotesPressGroup **tracks = malloc(sizeof(struct NotesPressGroup*) * trackSize);
    for(uint8_t i = 0; i < trackSize; i++){
        // printf("track %i size: %zu\n", i, eachTrackNoteCount[i]);
        tracks[i] = malloc(sizeof(struct NotesPressGroup) * eachTrackNoteCount[i]);
        eachTrackNoteCount[i] = 0;
    }

    while(*stackSize){
        struct NotesPressGroup *note = stack[--*stackSize];
        // size_t trackIndex = note->trackIndex;
        // size_t trackWIndex = eachTrackNoteIndex[trackIndex]++;
        // // printf("%p %zu %zu\n", note, trackIndex, trackWIndex);
        // tracks[trackIndex] = note;
    }

    m->tracks = tracks;
    m->trackSize = trackSize;
}

void addToMeasure(struct NotesPressGroup *npg, struct Note **stack, size_t *stackSize, size_t *eachTrackNoteCount){
    // printf("adding: ");
    for(size_t j = 0; j < npg->size; j++){
        struct Note *note = npg->notes[j];
        eachTrackNoteCount[note->trackIndex]++;
        // printf("%s, ", noteNames[note->type % 12]);
        stack[(*stackSize)++] = note;
    }
    // printf("\n");
}

void generateMeasure(struct Song *song, struct SongInfo songInfo){
    struct NotesPressGroup *last = song->notesArray[song->notesArraySize - 1];
    // TODO: song can have different time signatures
    size_t measureSize = (last->timerEnd - songInfo.upbeat) / 4 + (songInfo.upbeat != 0);
    // printf("lastTimer %zu\n", measureSize);
    // printf("songInfo, %i %f\n", song->trackSize, songInfo.upbeat);
    struct Measure *measures = malloc(sizeof(struct Measure) * measureSize);
    size_t measureIndex = 0;
    
    size_t eachTrackNoteCount[MAX_TRACK_SIZE] = {};

    // TODO: ensure the stack never overflows
    // for now we have demisemiquater as the minimum and max MAX_TRACK_SIZE
    const size_t maxStackSize = MIN_NOTE_COUNT * MAX_TRACK_SIZE;
    struct Note *stack[maxStackSize];
    size_t stackSize = 0; 

    // create upbeat
    size_t i = 0;
    if(songInfo.upbeat != 0){
        // printf("upbeat set %f\n", songInfo.upbeat);
        for(; i < song->notesArraySize;){
            if(songInfo.upbeat <= song->notesArray[i]->timer){
                flushMeasure(&measures[measureIndex], song->trackSize, stack, &stackSize, eachTrackNoteCount);
                measureIndex++;
                break;
            }
            else{
                struct NotesPressGroup *npg = song->notesArray[i];
                addToMeasure(npg, stack, &stackSize, eachTrackNoteCount);
            }
            i++;
        }
    }

    size_t currMeasureIndex = 0;
    for(;i < song->notesArraySize;){
        if((currMeasureIndex + 1) * 4 <= song->notesArray[i]->timer - songInfo.upbeat){
            // printf("measure ends on timer %f\n", song->notesArray[i]->timer);
            flushMeasure(&measures[measureIndex++], song->trackSize, stack, &stackSize, eachTrackNoteCount);
            currMeasureIndex++;
        }
        else{
            struct NotesPressGroup *npg = song->notesArray[i];
            addToMeasure(npg, stack, &stackSize, eachTrackNoteCount);
            i++;
        }
    }
}

void generateSheet(struct Song *song, struct SongInfo songInfo){
    return;
}
