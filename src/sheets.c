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

VECTOR_TYPE_FUNCTIONS(struct NoteDuration, NoteDurationVector, "");

void format8(uint8_t n, char *buffer){
    uint8_t i = 0;
    for(i = 0; i < 8; i++){
        buffer[i] = '0';
    }

    while(n){
        buffer[7 - i++] = (n & 1) + '0';
        n >>= 1;
    }

    buffer[8] = 0;
}

char *noteTypes[] = {
    "FULL",
    "HALF",
    "DOT",
    "TRIPLET"
};

int noteDurationComp(const void *a, const void *b){
    const int x = (*(struct NoteDuration*)a).duration;
    const int y = (*(struct NoteDuration*)b).duration;
    
    return (y < x) - (x < y);
}

struct NoteDuration *generateValidNotes(uint32_t percision, size_t *rSize){
    struct NoteDurationVector *durationsVector = NoteDurationVectorInit();

    for(uint8_t t = 0; t < 4; t++){
        for(uint8_t i = 0; i < NOTES_TYPE_COUNT; i++){
            double full = 8 / pow(2, i);
            double half = full / 2;
            double dot = full * 1.5;
            double triplet = (full * 2) / 3;

            double arr[] = {full, half, dot, triplet};
            float dur = arr[t];
            float duration = NOTE_DURATION(dur, percision);

            struct NoteDuration *target = NULL;
            for(size_t j = 0; j < durationsVector->size; j++){
                if(durationsVector->data[j].duration == duration){
                    target = &durationsVector->data[j];
                    break;
                }
            }

            if(target == NULL){
                NoteDurationVectorPush(durationsVector, (struct NoteDuration){0, duration, i, 1 << t});
            }
        }
    }

    struct NoteDuration *durationsTable = NoteDurationVectorDuplicate(durationsVector, rSize);
    NoteDurationVectorFree(durationsVector);

    qsort(durationsTable, *rSize, sizeof(struct NoteDuration), noteDurationComp);

    return durationsTable;
}

void validNotesPrint(struct NoteDuration *noteDurations, size_t size){
    for(size_t i = 0; i < size; i++){
        if(noteDurations[i].duration){
            char buffer[10];
            format8(noteDurations[i].flags, buffer);
            // format8(0b1, buffer);
            printf("// %s\n", noteTypes[noteDurations[i].noteType]);
            printf("%f %i %s\n", noteDurations[i].duration, noteDurations[i].noteType, buffer);
        }
    }
}

// TODO: use hashset or ordered set for faster search
void songsNoteDurations(struct Song *song, struct SongInfo *songInfo, struct Sheet *sheet, bool debug){
    struct NoteDurationVector *diffDurations = NoteDurationVectorInit();

    for(size_t i = 0; i < song->notesArraySize; i++){
        struct NotesPressGroup *n = song->notesArray[i];
        for(size_t j = 0; j < n->size; j++){
            struct NotePressGroup *note = n->notes[j];
            float duration = NOTE_DURATION(note->duration, songInfo->percision);

            struct NoteDuration *target = NULL;
            for(size_t j = 0; j < diffDurations->size; j++){
                if(diffDurations->data[j].duration == duration){
                    target = &diffDurations->data[j];
                    break;
                }
            }

            if(target == NULL){
                if(debug){
                    printf("// timer %f\n", n->timer);
                    printf("%f,\n", duration);
                }
                struct NoteDuration nI = {duration, 0, n->timer, 0};
                NoteDurationVectorPush(diffDurations, nI);
            }
        }
    }
    // printf("number of durations %zu\n", diffDurations->size);

    size_t durationsSize = 0;
    struct NoteDuration *durations = NoteDurationVectorDuplicate(diffDurations, &durationsSize);
    NoteDurationVectorFree(diffDurations);

    qsort(durations, durationsSize, sizeof(struct NoteDuration), noteDurationComp);

    sheet->songNotes = durations;
    sheet->songNotesSize = durationsSize;
}

void correctDurations(char *correctionFilePath, struct SongInfo *songInfo, struct Sheet *sheet){
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
            struct NoteDuration *target = NULL;
            for(size_t i = 0; i < sheet->songNotesSize; i++){
                if(dur1 == sheet->songNotes[i].duration){
                    target = &sheet->songNotes[i];
                }
            }
            if(!target){
                fprintf(stderr, "no matched fouond for duration %f, make sure to use same percision\n", dur1);
                exit(1);
            }

            float error = FLT_MAX;
            struct NoteDuration *correctNote = NULL;
            for(size_t i = 0; i < sheet->validNotesSize; i++){
                float e = sheet->validNotes[i].correctDuration - target->duration;
                if(e < error){
                    e = error;
                    correctNote = &sheet->validNotes[i];
                }
            }
            
            target->correctDuration = correctNote->correctDuration;
            target->noteType = correctNote->noteType;
            target->flags = correctNote->flags;

            printf("%f corrected to %f\n", dur1, dur2);
        }
        else{
            printf("%f doesn't have a value\n", dur1);
        }
        // printf("%i %s", i++, line);
    }
}

void flushMeasure(struct MeasureVector *m, uint8_t trackSize, struct NoteVector *stack, size_t *eachTrackNoteCount, struct NotesPressGroup *npg){
    // printf("measure has %zu notes\n", *stackSize);
    size_t eachTrackNoteIndex[MAX_TRACK_SIZE] = {};

    // printf("track size %i\n", trackSize);
    struct Measure measure = {};
    
    measure.numerator = npg->numerator;
    measure.denominator = npg->denominator;
    measure.BPM = npg->BPM;

    measure.tracks = malloc(sizeof(struct Note*) * trackSize);
    measure.trackSize = trackSize;

    for(uint8_t i = 0; i < trackSize; i++){
        struct Note *track = malloc(sizeof(struct Note) * eachTrackNoteCount[i]);
        measure.tracks[i] = track;
        eachTrackNoteCount[i] = 0;
    }

    for(size_t i = 0; i < stack->size; i++){
        struct Note note = stack->data[i];
        measure.tracks[note.trackIndex][eachTrackNoteIndex[note.trackIndex]++] = note;
    }
    stack->size = 0;

    MeasureVectorPush(m, measure);
}

struct Note toNote(struct SongInfo *songInfo, struct Sheet *sheet, struct NotePressGroup *notePressGroup){
    struct Note note = {notePressGroup->type, notePressGroup->trackIndex, NULL};
    
    float duration = NOTE_DURATION(notePressGroup->duration, songInfo->percision);
    for(size_t i = 0; i < sheet->songNotesSize; i++){
        if(duration == sheet->songNotes[i].correctDuration){
            note.duration = &sheet->songNotes[i];
        }
    }

    return note;
}

float measureEnd(struct NotesPressGroup *npg){
    uint8_t n = npg->numerator;
    // uint8_t d = npg->denominator;
    
    // TODO: correct measure calculation
    float duration = n;
    return npg->timer + duration;
}

void generateMeasure(struct Song *song, struct SongInfo *songInfo, struct Sheet *sheet){
    // printf("lastTimer %zu\n", measureSize);
    // printf("songInfo, %i %f\n", song->trackSize, songInfo.upbeat);
    struct MeasureVector *measureVector = MeasureVectorInit();
    
    size_t eachTrackNoteCount[MAX_TRACK_SIZE] = {};

    struct NoteVector *stack = NoteVectorInit();

    float end = measureEnd(song->notesArray[0]);

    if(songInfo->upbeat != 0){
        end = songInfo->upbeat;
    }

    size_t i = 0;
    while(i < song->notesArraySize){
        if(end < song->notesArray[i]->timer){
            struct NotesPressGroup *npg = song->notesArray[i];
            // add notes to measure
            for(size_t j = 0; j < npg->size; j++){
                struct NotePressGroup *note = npg->notes[j];
                eachTrackNoteCount[note->trackIndex]++;
                struct Note n = toNote(songInfo, sheet, note);
                NoteVectorPush(stack, n);
            }
        }

        i++;

        if(i == song->notesArraySize || end <= song->notesArray[i]->timer){
            flushMeasure(measureVector, song->trackSize, stack, eachTrackNoteCount, song->notesArray[i - 1]);
            printf("measure %zu end %f\n", measureVector->size, end);
            if(i < song->notesArraySize){
                struct NotesPressGroup *npg = song->notesArray[i];
                end = measureEnd(npg);
            }
        }
    }
}

void generateSheet(struct Song *song, struct SongInfo songInfo){
    return;
}
