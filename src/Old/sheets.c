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

VECTOR_TYPE_FUNCTIONS(struct MidiNoteDuration, MidiNoteDurationVector, "");

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
    const int x = (*(struct MidiNoteDuration*)a).duration;
    const int y = (*(struct MidiNoteDuration*)b).duration;
    
    return (y < x) - (x < y);
}

struct MidiNoteDuration *generateNoteTable(uint32_t PPQ){

    struct MidiNoteDuration notes[] = {
        NOTE_VALUE(MAXIMA, 4 * 8 * PPQ, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(MAXIMA, 4 * 12 * PPQ, DOTTED, NOT_TUPLET),

        NOTE_VALUE(LONG, 4 * 4 * PPQ, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(DOUBLE_WHOLE_NOTE, 4 * 2 * PPQ, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(WHOLE_NOTE, 4 * 1 * PPQ, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(HALF_NOTE, 2 * PPQ, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(QUATER_NOTE, PPQ, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(EIGHT_NOTE, PPQ / 2, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(SIXTEENTH_NOTE, PPQ / 4, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(THIRTY_SECOND_NOTE, PPQ / 8, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(SIXTEENTH_NOTE, PPQ / 16, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(HUNDRED_TWENTY_EIGHTH_NOTE, PPQ / 32, NO_DOT, NOT_TUPLET),
        NOTE_VALUE(TWO_HUNDRED_FIFTY_SIXTH_NOTE, PPQ / 64, NO_DOT, NOT_TUPLET),
    };

    return NULL;
}

struct MidiNoteDuration *generateValidNotes(uint32_t percision, size_t *rSize){
    struct MidiNoteDurationVector *durationsVector = MidiNoteDurationVectorInit();

    for(uint8_t t = 0; t < 4; t++){
        for(uint8_t i = 0; i < NOTES_TYPE_COUNT; i++){
            double full = 8 / pow(2, i);
            double half = full / 2;
            double dot = full * 1.5;
            double triplet = (full * 2) / 3;

            double arr[] = {full, half, dot, triplet};
            float dur = arr[t];
            float duration = NOTE_DURATION(dur, percision);

            struct MidiNoteDuration *target = NULL;
            for(size_t j = 0; j < durationsVector->size; j++){
                if(durationsVector->data[j].duration == duration){
                    target = &durationsVector->data[j];
                    break;
                }
            }

            if(target == NULL){
                MidiNoteDurationVectorPush(durationsVector, (struct MidiNoteDuration){0, duration, i, 1 << t});
            }
        }
    }

    struct MidiNoteDuration *durationsTable = MidiNoteDurationVectorDuplicate(durationsVector, rSize);
    MidiNoteDurationVectorFree(durationsVector);

    qsort(durationsTable, *rSize, sizeof(struct MidiNoteDuration), noteDurationComp);

    return durationsTable;
}

void validNotesPrint(struct MidiNoteDuration *noteDurations, size_t size){
    for(size_t i = 0; i < size; i++){
        if(noteDurations[i].duration){
            char buffer[10];
            // format8(noteDurations[i].flags, buffer);
            // format8(0b1, buffer);
            printf("// %s\n", noteTypes[noteDurations[i].noteType]);
            printf("%f %i %s\n", noteDurations[i].duration, noteDurations[i].noteType, buffer);
        }
    }
}

// TODO: use hashset or ordered set for faster search
void songsNoteDurations(struct MidiSong *song, struct MidiSongInfo *songInfo, struct MidiSheet *sheet, bool debug){
    struct MidiNoteDurationVector *diffDurations = MidiNoteDurationVectorInit();

    for(size_t i = 0; i < song->notesArraySize; i++){
        struct MidiNotesPressGroup *n = song->notesArray[i];
        for(size_t j = 0; j < n->size; j++){
            struct MidiNotePressGroup *note = n->notes[j];
            float duration = NOTE_DURATION(note->duration, songInfo->percision);

            struct MidiNoteDuration *target = NULL;
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
                struct MidiNoteDuration nI = {duration, 0, n->timer, 0};
                MidiNoteDurationVectorPush(diffDurations, nI);
            }
        }
    }
    // printf("number of durations %zu\n", diffDurations->size);

    size_t durationsSize = 0;
    struct MidiNoteDuration *durations = MidiNoteDurationVectorDuplicate(diffDurations, &durationsSize);
    MidiNoteDurationVectorFree(diffDurations);

    qsort(durations, durationsSize, sizeof(struct MidiNoteDuration), noteDurationComp);

    sheet->songNotes = durations;
    sheet->songNotesSize = durationsSize;
}

void correctDurations(char *correctionFilePath, struct MidiSongInfo *songInfo, struct MidiSheet *sheet){
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
            struct MidiNoteDuration *target = NULL;
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
            struct MidiNoteDuration *correctNote = NULL;
            for(size_t i = 0; i < sheet->validNotesSize; i++){
                float e = sheet->validNotes[i].correctDuration - target->duration;
                if(e < error){
                    e = error;
                    correctNote = &sheet->validNotes[i];
                }
            }
            
            target->correctDuration = correctNote->correctDuration;
            target->noteType = correctNote->noteType;
            // target->flags = correctNote->flags;

            printf("%f corrected to %f\n", dur1, dur2);
        }
        else{
            printf("%f doesn't have a value\n", dur1);
        }
        // printf("%i %s", i++, line);
    }
}

void flushMeasure(struct MidiMeasureVector *m, uint8_t trackSize, struct MidiNoteVector *stack, size_t *eachTrackNoteCount, struct MidiNotesPressGroup *npg){
    // printf("measure has %zu notes\n", *stackSize);
    size_t eachTrackNoteIndex[MAX_TRACK_SIZE] = {};

    // printf("track size %i\n", trackSize);
    struct MidiMeasure measure = {};
    
    measure.numerator = npg->numerator;
    measure.denominator = npg->denominator;
    measure.BPM = npg->BPM;

    measure.tracks = malloc(sizeof(struct MidiNote*) * trackSize);
    measure.trackSize = trackSize;

    for(uint8_t i = 0; i < trackSize; i++){
        struct MidiNote *track = malloc(sizeof(struct MidiNote) * eachTrackNoteCount[i]);
        measure.tracks[i] = track;
        eachTrackNoteCount[i] = 0;
    }

    for(size_t i = 0; i < stack->size; i++){
        struct MidiNote note = stack->data[i];
        measure.tracks[note.trackIndex][eachTrackNoteIndex[note.trackIndex]++] = note;
    }
    stack->size = 0;

    MidiMeasureVectorPush(m, measure);
}

struct MidiNote toNote(struct MidiSongInfo *songInfo, struct MidiSheet *sheet, struct MidiNotePressGroup *notePressGroup){
    struct MidiNote note = {notePressGroup->type, notePressGroup->trackIndex, NULL};
    
    float duration = NOTE_DURATION(notePressGroup->duration, songInfo->percision);
    for(size_t i = 0; i < sheet->songNotesSize; i++){
        if(duration == sheet->songNotes[i].correctDuration){
            note.duration = &sheet->songNotes[i];
        }
    }

    return note;
}

float measureEnd(struct MidiNotesPressGroup *npg){
    uint8_t n = npg->numerator;
    // uint8_t d = npg->denominator;
    
    // TODO: correct measure calculation
    float duration = n;
    return npg->timer + duration;
}

void generateMeasure(struct MidiSong *song, struct MidiSongInfo *songInfo, struct MidiSheet *sheet){
    // printf("lastTimer %zu\n", measureSize);
    // printf("songInfo, %i %f\n", song->trackSize, songInfo.upbeat);
    struct MidiMeasureVector *measureVector = MidiMeasureVectorInit();
    
    size_t eachTrackNoteCount[MAX_TRACK_SIZE] = {};

    struct MidiNoteVector *stack = MidiNoteVectorInit();

    float end = measureEnd(song->notesArray[0]);

    if(songInfo->upbeat != 0){
        end = songInfo->upbeat;
    }

    size_t i = 0;
    while(i < song->notesArraySize){
        if(end < song->notesArray[i]->timer){
            struct MidiNotesPressGroup *npg = song->notesArray[i];
            // add notes to measure
            for(size_t j = 0; j < npg->size; j++){
                struct MidiNotePressGroup *note = npg->notes[j];
                eachTrackNoteCount[note->trackIndex]++;
                struct MidiNote n = toNote(songInfo, sheet, note);
                MidiNoteVectorPush(stack, n);
            }
        }

        i++;

        if(i == song->notesArraySize || end <= song->notesArray[i]->timer){
            flushMeasure(measureVector, song->trackSize, stack, eachTrackNoteCount, song->notesArray[i - 1]);
            printf("measure %zu end %f\n", measureVector->size, end);
            if(i < song->notesArraySize){
                struct MidiNotesPressGroup *npg = song->notesArray[i];
                end = measureEnd(npg);
            }
        }
    }
}

void generateSheet(struct MidiSong *song, struct MidiSongInfo songInfo){
    return;
}
