#include "xmlParser.h"
#include <stdlib.h>

// https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/note/
struct Note *parseNote(xmlNodePtr part, StaffNumber *staveIndex, bool *isChord){
    struct Note *note = calloc(1, sizeof(struct Note));

    *staveIndex = 0;
    xmlNodePtr children = part->xmlChildrenNode;
    while(children){
        if(xmlStrcmp(children->name, XML_CHAR"rest") == 0){
            SET_BIT(note->flags, NOTE_FLAG_REST);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"chord") == 0){
            *isChord = true;
        }
        else if(xmlStrcmp(children->name, XML_CHAR"duration") == 0){
            note->duration = parseBody(children);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"type") == 0){
            note->noteType = parseNoteType(children);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"pitch") == 0){
            parsePitch(children, note);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"accidental") == 0){
            parseAccidental(children, note);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"notations") == 0){
            parseNotations(children, note);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"staff") == 0){
            *staveIndex = parseBody(children) - 1;
        }
        else if(xmlStrcmp(children->name, XML_CHAR"time-modification") == 0){
            parseTimeModification(children, note);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"beam") == 0){
            parseBeam(children, &note->beams);
        }
        children = children->next;
    }

    // if(GET_BIT(note->flags, NOTE_FLAG_REST)){
    //     debugf("new rest\n");
    // }
    // else{
    //     debugf("new note (%i:%i:%c)\n", note->octave, note->step, note->stepChar);
    // }
    
    return note;
}

void notesMagazinePrint(struct NoteVectorPVector *notesVectorMagazine){
    debugf("==== magazine ====\n");
    for(size_t i = 0; i < notesVectorMagazine->size; i++){
        struct NotePVector *currNoteVector = notesVectorMagazine->data[i];
        debugf("%p [%zu]: ", currNoteVector, currNoteVector->size);
        for(size_t j = 0; j < currNoteVector->size; j++){
            debugf("%p ", currNoteVector->data[j]);
        }
        debugf("\n");
    }
    debugf("===== mag end ====\n");
}

void flushNotes(Staff staff, struct NoteVectorPVector *notesVectorMagazine, size_t measureNoteSize){
    // notesMagazinePrint(notesVectorMagazine);
    for(size_t i = 0; i < measureNoteSize; i++){
        // if the magazine is smaller then the buffer or the its empty
        // then the buffer with this index will be set to NULL
        if(notesVectorMagazine->size <= i || notesVectorMagazine->data[i]->size == 0){
            staff[i] = NULL;
        }
        else if(i < notesVectorMagazine->size && 0 < notesVectorMagazine->data[i]->size){
            // debugf("%i m p %p\n", i, notesVectorMagazine->data[i]);
            struct NotePVector *notesVector = notesVectorMagazine->data[i];
            // fprintf(stderr, "%li %zu\n", i, notesVector->size);
            size_t noteCounter = 0;
            
            struct Note *lastNote = NULL;
            struct Note *lastRest = NULL;
            for(size_t j = 0; j < notesVector->size; j++){
                struct Note *note = notesVector->data[j];
                // fprintf(stderr, "%p\n", note);
                if(GET_BIT(note->flags, NOTE_FLAG_REST) == 0){
                    lastNote = note;
                    noteCounter++;
                }
                else{
                    lastRest = note;
                }
            }

            struct Notes *notes = calloc(1, sizeof(struct Notes));

            if(noteCounter == 0){
                struct Note **chord = malloc(sizeof(struct Note*));
                chord[0] = lastRest;
                notes->chord = chord;

                // printf("last rest %p\n", lastRest);
                // notes->chord = &lastRest;
                notes->chordSize = 1;
            }
            else if(noteCounter == 1){
                struct Note **chord = malloc(sizeof(struct Note*));
                chord[0] = lastNote;
                notes->chord = chord;
                // debugf("last note %p %p\n", lastNote, notes->chord);
                notes->chordSize = 1;
                notes->beams = lastNote->beams;
            }
            else{
                struct Note **chord = malloc(sizeof(struct Note*) * noteCounter);
                // debugf("new chord %p\n", chord);

                for(size_t j = 0; j < notesVector->size; j++){
                    struct Note *note = notesVector->data[j];
                    if(GET_BIT(note->flags, NOTE_FLAG_REST) == 0){
                        chord[j] = note;
                    }
                    else{
                        fprintf(stderr, "note in chord is a rest");
                        exit(1);
                    }
                }

                notes->chord = chord;
                notes->chordSize = noteCounter;
                notes->beams = chord[0]->beams;
            }

            // debugf("chord p: %p\n", notes->chord);
            staff[i] = notes;

            // debugf("flushed note's beams %i\n", notes->beams);

            notesVector->size = 0;
        }
    }
}

Pitch parseStep(char c){
    switch(c){
        case 'A':
            return 5;
        case 'B':
            return 6;
        case 'C':
            return 0;
        case 'D':
            return 1;
        case 'E':
            return 2;
        case 'F':
            return 3;
        case 'G':
            return 4;
    }

    return 0;
}

#define MATCH(str, enum){\
    if(strcmp(content, str) == 0){\
        free(content);\
        return enum;\
    }\
}

// https://www.w3.org/2021/06/musicxml40/musicxml-reference/data-types/note-type-value/
enum NoteType parseNoteType(xmlNodePtr node){
    char *content = (char*)xmlNodeGetContent(node);

    MATCH("maxima",    NOTE_TYPE_MAXIMA);
    MATCH("long",      NOTE_TYPE_LONG);
    MATCH("breve",     NOTE_TYPE_BREVE);
    MATCH("whole",     NOTE_TYPE_WHOLE);
    MATCH("half",      NOTE_TYPE_HALF);
    MATCH("quarter",   NOTE_TYPE_QUARTER);
    MATCH("eighth",    NOTE_TYPE_EIGHTH);
    MATCH("16th",      NOTE_TYPE_16TH);
    MATCH("32nd",      NOTE_TYPE_32ND);
    MATCH("64th",      NOTE_TYPE_64TH);
    MATCH("128th",     NOTE_TYPE_128TH);
    MATCH("256th",     NOTE_TYPE_256TH);
    MATCH("512th",     NOTE_TYPE_512TH);
    MATCH("1024th",    NOTE_TYPE_1024TH);

    fprintf(stderr, "note type '%s', not found\n", content);
    free(content);
    exit(1);
}

// https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/pitch/
void parsePitch(xmlNodePtr node, struct Note *note){
    xmlNodePtr children = node->xmlChildrenNode;
    while(children){
        // https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/step/
        if(xmlStrcmp(children->name, XML_CHAR"step") == 0){
            char *content = (char*)xmlNodeGetContent(children);
            char c = content[0];
            note->pitch.step = parseStep(c);
            note->pitch.stepChar = c;
            free(content);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"alter") == 0){
            long alter = parseBody(children);
            // sharp == 1 #
            // flat == -1 b
            note->pitch.alter = alter;
        }
        // https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/octave/
        else if(xmlStrcmp(children->name, XML_CHAR"octave") == 0){
            note->pitch.octave = parseBody(children);
            if(9 < note->pitch.octave ){
                fprintf(stderr, "note's octave must be in range <0;9> you have %i\n", note->pitch.octave);
            }
        }
        children = children->next;
    }

    Pitch pitch = notePitchToPitch(&note->pitch);
    debugf("step: %i alter %i octave %i => %i\n", note->pitch.step, note->pitch.alter, note->pitch.octave, pitch);
}

// <accidental>
// https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/accidental/
void parseAccidental(xmlNodePtr parent, struct Note *note){
    xmlChar *body = xmlNodeGetContent(parent);
    SET_BIT(note->flags, NOTE_FLAG_ACCIDENTAL);
    if(xmlStrcmp(body, XML_CHAR"sharp") == 0){
        SET_BIT(note->flags, NOTE_FLAG_SHARP);
    }
    else if(xmlStrcmp(body, XML_CHAR"natural") == 0){
        SET_BIT(note->flags, NOTE_FLAG_NATURAL);
    }
    else if(xmlStrcmp(body, XML_CHAR"flat") == 0){
        SET_BIT(note->flags, NOTE_FLAG_FLAT);
    }
    else{
        SET_BIT(note->flags, NOTE_FLAG_SHARP);
        // fprintf(stderr, "accidental '%s' not implemented\n", body);
        // exit(1);
    }
    free(body);
    // debugf("accidental: %i\n", note->flags);
}

void parseNotations(xmlNodePtr parent, struct Note *note){
    xmlNodePtr element = parent->xmlChildrenNode;
    while(element){
        // https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/tied/
        if(xmlStrcmp(element->name, XML_CHAR"tied") == 0){
            SET_BIT(note->flags, TIE_FLAG);
        }
        // TODO articulations -> staccato
        element = element->next;
    }
}

// https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/time-modification/
void parseTimeModification(xmlNodePtr parent, struct Note *note){
    xmlNodePtr element = parent->xmlChildrenNode;
    while(element){
        // https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/actual-notes/
        if(xmlStrcmp(element->name, XML_CHAR"actual-notes") == 0){
            note->tuplet = parseBody(element);
        }

        element = element->next;
    }
}

// <beam>
// https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/beam/
// https://www.w3.org/2021/06/musicxml40/musicxml-reference/data-types/beam-value/
void parseBeam(xmlNodePtr parent, Beams *rBeams){
    int number = parseProp(parent, "number");
    char *body = (char*)xmlNodeGetContent(parent);
    if(number){
        number--;
    }
    
    Beam beam = 0;
    SET_BIT(beam, BEAM_ENABLED);

    if(strcmp(body, "end") == 0){
        SET_BIT(beam, BEAM_OFF);
    }

    bool fHook = (strcmp(body, "forward hook") == 0);
    bool bHook = (strcmp(body, "backward hook") == 0);
    if(fHook || bHook){
        SET_BIT(beam, BEAM_OFF);
        
        SET_BIT(beam, BEAM_HOOK_ENABLED);
        if(bHook){
            SET_BIT(beam, BEAM_HOOK_BACKWARD);
        }
    }

    SET_BEAM(*rBeams, number, beam);
    free(body);
    // debugf("beam[%i] val: %i => %i\n", number, beam, *rBeams);
}
