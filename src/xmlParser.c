#include "xmlParser.h"

#define ENCODING_CHUNK_SIZE 5
#define CHUNK_SIZE 16384

#define MIN(a, b) ((a < b) ? a : b)

VECTOR_TYPE_FUNCTIONS(struct Measure*, MeasurePVector, "%p");
VECTOR_TYPE_FUNCTIONS(struct Note*, NotePVector, "%p");
VECTOR_TYPE_FUNCTIONS(struct NotePVector*, NoteVectorPVector, "%p");

// file has to be in dir 0 and to have extension .xml
bool matchFile(const char *name){
    for(uint8_t i = 0; name[i]; i++){
        if(name[i] == '/'){
            return false;
        }
    }

    char *dot = strchr(name, '.');
    if(dot && strcmp(dot, ".xml") == 0){
        return true;
    }

    return false;
}

struct Measure **readNotes(char filePath[], size_t *measureSize){
    struct zip *z = zip_open(filePath, 0, NULL);
    if(!z){
        fprintf(stderr, "failed to open zip file: %s\n", filePath);
        exit(1);
    }

    long count = zip_get_num_entries(z, 0);
    debugf("number of files %lu\n", count);
    if(count < 0){
        fprintf(stderr, "unable to get number of files in zip\n");
        exit(1);
    }

    xmlDocPtr doc;
    for(int i = 0; i < count; i++){
        struct zip_stat st;
        if(zip_stat_index(z, i, 0, &st) != 0){
            fprintf(stderr, "unable to get stats for file %d in zip\n", i);
            zip_close(z);
            exit(1);
        }

        const char *fileName = zip_get_name(z, i, 0);
        // debugf("file %s\n", fileName);
        if(matchFile(fileName) != true){
        // if(strcmp((char*)fileName, "score.xml") != 0){
            continue;
        }
        // debugf("matched file %s\n", fileName);
        
        struct zip_file *zf = zip_fopen_index(z, i, 0);
        if(!zf){
            fprintf(stderr, "unable to open zip file: %s\n", fileName);
            zip_close(z);
            exit(1);
        }

        // char encodingBuffer[ENCODING_CHUNK_SIZE] = {};
        // int encodingBufferSize = zip_fread(zf, encodingBuffer, MIN(ENCODING_CHUNK_SIZE, st.size));
        // xmlParserCtxtPtr ctxt = xmlCreatePushParserCtxt(NULL, NULL, encodingBuffer, encodingBufferSize, NULL);
        xmlParserCtxtPtr ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);

        char buffer[CHUNK_SIZE + 1] = {};
        zip_int64_t bytesWritten = ENCODING_CHUNK_SIZE;
        zip_int64_t bytesLeft = st.size;

        // FILE *notes = fopen("/tmp/notes", "w+");

        // debugf("size %i\n", bytesLeft);
        while(0 < bytesLeft){
            zip_int64_t bytesToRead = MIN(CHUNK_SIZE, bytesLeft);
            zip_int64_t bytesRead = zip_fread(zf, buffer, bytesToRead);
            if(bytesRead < 0){
                debugf("failed to read data from file %s in zip\n", fileName);
                exit(1);
            }
            bytesWritten += bytesRead;
            bytesLeft -= bytesRead;
            // debugf("bytes read: %i left: %i\n", bytesRead, bytesLeft);
            
            // fwrite(buffer, sizeof(char), bytesRead, notes);

            buffer[bytesRead] = 0;
            xmlParseChunk(ctxt, buffer, bytesRead, 0);
        }
        // fclose(notes);

        xmlParseChunk(ctxt, NULL, 0, 1);

        doc = ctxt->myDoc;
        int res = ctxt->wellFormed;
        xmlFreeParserCtxt(ctxt);
        if(!res){
            fprintf(stderr, "failed to parse %s\n", fileName);
            exit(1);
        }

        debugf("notes parsed successfully\n");
    }

    xmlNode *root = xmlDocGetRootElement(doc);
    struct Measure **measures = NULL;
    *measureSize = 0;
    // TODO: missing <score-timewise>
    // https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/measure-partwise/
    if(xmlStrcmp(root->name, XML_CHAR"score-partwise") == 0){
        xmlNodePtr children = root->xmlChildrenNode;
        while(children){
            if(xmlStrcmp(children->name, XML_CHAR"part") == 0){
                measures = parseMeasures(children, measureSize);
            }
            children = children->next;
        }
    }
    xmlFreeDoc(doc);

    return measures;
}

long parseBody(xmlNodePtr part){
    // debugf("parsing body\n");
    xmlChar *content = xmlNodeGetContent(part);
    long r = strtol((char*)content, NULL, 10);
    // debugf("end parsing body\n");
    return r;
}

long parseProp(xmlNodePtr note, char *name){
    char *prop = (char*)xmlGetProp(note, XML_CHAR name);
    if(!prop){
        return 0;
    }

    return strtol(prop, NULL, 10);
}

uint16_t getMeasureNotesSize(xmlNodePtr note, struct Attributes *currAtrributes, MeasureSize *measureSize, StaffNumber *staffNumber){
    xmlNodePtr children = note->xmlChildrenNode;
    long division = 0;
    TimeSignature denominator = 0, numerator = 0;
    long staves = 0;
    bool set = false;
    while(children){
        if(xmlStrcmp(children->name, XML_CHAR"divisions") == 0){
            set = true;
            division = parseBody(children);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"time") == 0){
            set = true;
            parseTime(children, &denominator, &numerator);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"staves") == 0){
            staves = parseBody(children);
        }

        children = children->next;
    }
    
    if(set){
        if(division == 0){
            division = currAtrributes->division;
        }
        if(denominator == 0){
            denominator = currAtrributes->denominator;
        }
        
        debugf("division: %li, denominator: %i\n", division, denominator);
        *measureSize = division * denominator;
    }

    if(staves == 0){
        staves = currAtrributes->stavesNumber;
    }

    *staffNumber = staves;
    return 0;
}

void printMeasure(struct Measure *measure){
    // for every staff in the measure
    for(StaffNumber staffIndex = 0; staffIndex < measure->stavesNumber; staffIndex++){
        Staff staff = measure->staffs[staffIndex];
        debugf("\n");
        printf("staff[%i]\n", staffIndex);
        // for every buffer place in staff
        for(Division d = 0; d < measure->measureSize; d++){
            struct Notes *notes = staff[d];
            // check if place is not empty
            if(notes){
                printf("  [%i]: ", d);
                for(ChordSize chordIndex = 0; chordIndex < notes->chordSize; chordIndex++){
                    struct Note *n = notes->chord[chordIndex];
                    
                    if(GET_BIT(n->flags, NOTE_FLAG_REST)){
                        printf("rest \n");
                    }
                    else{
                        printf("(%i:%i:%c)\n", n->pitch.octave, n->pitch.step, n->pitch.stepChar);
                    }
                }
                printf("\n");
            }
        }
    }
    debugf("\n");
}

void printMeasures(struct MeasurePVector *measuresVector){
    // for every measure
    for(size_t i = 0; i < measuresVector->size; i++){
        debugf("measure[%zu]\n", i);
        struct Measure *measure = measuresVector->data[i];
        printMeasure(measure);
    }
}

struct Measure **parseMeasures(xmlNodePtr part, size_t *measureSize){
    xmlNodePtr children = part->xmlChildrenNode;
    
    struct MeasurePVector *measuresVector = MeasurePVectorInit();

    // TODO: repeats
    // used for repeats
    // struct Measure *measureStart = NULL;

    struct NoteVectorPVector *notesVectorMagazine = NoteVectorPVectorInit();

    struct Attributes *currAtrributes = attributesInit();
    // while(children && !measuresVector->size){
    while(children){
        if(xmlStrcmp(children->name, XML_CHAR"measure") == 0){
            struct Measure *m = parseMeasure(children, notesVectorMagazine, currAtrributes);
            MeasurePVectorPush(measuresVector, m);
        }
        
        children = children->next;
    }

    struct Measure **measures = MeasurePVectorDuplicate(measuresVector, measureSize);

    // printMeasures(measuresVector);
    MeasurePVectorFree(measuresVector);

    return measures;
}

struct Measure *parseMeasure(xmlNodePtr measure, struct NoteVectorPVector *notesVectorMagazine, struct Attributes *currAtrributes){
    long measureNumber = parseProp(measure, "number");
    debugf("measure number: %li\n", measureNumber);
    struct Measure *m = malloc(sizeof(struct Measure));
    m->attributes = NULL;
    m->staffs = NULL;
    m->sheetMeasureIndex = measureNumber;

    xmlNodePtr children = measure->xmlChildrenNode;

    MeasureSize measureSize = 0;
    long numberOfAttributes = 0;
    StaffNumber staffNumber = 0;
    // find 
    // - the smallest note duration for the notes array
    while(children){
        if(xmlStrcmp(children->name, XML_CHAR"attributes") == 0){
            getMeasureNotesSize(children, currAtrributes, &measureSize, &staffNumber);
            numberOfAttributes++;
        }
        children = children->next;
    }

    if(!measureSize){
        long division = currAtrributes->division;
        long denominator = currAtrributes->denominator;
        
        measureSize = division * denominator;
    }
    if(!staffNumber){
        staffNumber = currAtrributes->stavesNumber;
    }

    children = measure->xmlChildrenNode;
    debugf("staffNumber: %i measureSize: %i attributes: %li\n", staffNumber, measureSize, numberOfAttributes);

    struct Attributes **attributes = NULL;
    Staff *staffs = malloc(sizeof(Staff*) * staffNumber);

    for(StaffNumber i = 0; i < staffNumber; i++){
        size_t s = sizeof(Staff) * measureSize;
        staffs[i] = malloc(s);
    }

    if(numberOfAttributes != 0){
        attributes = calloc(measureSize, sizeof(struct Attributes*));
    }

    size_t currTime = 0;

    StaffNumber staveIndex = 0, prevStaveIndex = 0;
    while(children){
        // fprintf(stderr, "curr time: %li\n", currTime);
        if(xmlStrcmp(children->name, XML_CHAR"attributes") == 0){
            attributes[currTime] = parseAttributes(children, currAtrributes);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"note") == 0){
            bool isChord = false;
            
            struct Note *note = parseNote(children, &staveIndex, &isChord);
            
            if(isChord){
                if(currTime < note->duration){
                    fprintf(stderr, "error currTime is negative\n");
                    exit(1);
                }
                
                currTime -= note->duration;
            }

            while(notesVectorMagazine->size <= currTime){
                struct NotePVector *newNotePVector = NotePVectorInit();
                NoteVectorPVectorPush(notesVectorMagazine, newNotePVector);
            }

            if(prevStaveIndex != staveIndex){
                // debugf("stave %i %i\n", prevStaveIndex, staveIndex);
                // debugf("staff %p\n", staffs[prevStaveIndex]);
                flushNotes(staffs[prevStaveIndex], notesVectorMagazine, measureSize);
                prevStaveIndex = staveIndex;
            }

            struct NotePVector *noteVector = notesVectorMagazine->data[currTime];

            NotePVectorPush(noteVector, note);

            currTime += note->duration;
        }
        else if(xmlStrcmp(children->name, XML_CHAR"backup") == 0){
            size_t backup = parseBody(children);

            if(currTime < backup){
                fprintf(stderr, "error currTime is negative\n");
                exit(1);
            }

            currTime -= backup;
        }
        children = children->next;
    }

    // debugf("staff %p %i\n", staffs[staveIndex], staveIndex);
    flushNotes(staffs[staveIndex], notesVectorMagazine, measureSize);

    // for(size_t i = 0; i < measureSize; i++){
    //     struct Staff *notes = malloc(sizeof(struct Staff));
    //     notes->chord = 
    // }

    m->staffs = staffs;
    m->measureSize = measureSize;
    m->stavesNumber = currAtrributes->stavesNumber;
    m->attributes = attributes;

    return m;
}

void parse(xmlNodePtr part){
    xmlNodePtr children = part->xmlChildrenNode;
    while(children){
        if(xmlStrcmp(children->name, XML_CHAR"divisions") == 0){
        }
        else if(xmlStrcmp(children->name, XML_CHAR"key") == 0){

        }
        children = children->next;
    }
}
