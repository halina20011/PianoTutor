#include "xmlParser.h"

#define MIN(a, b) ((a < b) ? a : b)

struct Attributes *attributesInit(){
    struct Attributes *a = malloc(sizeof(struct Attributes));
    
    a->division = 0;
    a->keySignature = 0;

    a->numerator = 0;
    a->denominator = 0;

    a->stavesNumber = 0;
    a->clefs = NULL;    

    return a;
}

void updateAttributes(struct Attributes *attributes, struct Attributes *currAttributes){
    if(attributes->division){
        currAttributes->division = attributes->division;
    }

    if(attributes->keySignature){
        currAttributes->keySignature = attributes->keySignature;
    }

    if(attributes->numerator){
        currAttributes->numerator = attributes->numerator;
        currAttributes->denominator = attributes->denominator;
    }

    if(attributes->stavesNumber){
        // if curr attribute has
        StaffNumber size = MIN(currAttributes->stavesNumber, attributes->stavesNumber);
        currAttributes->stavesNumber = attributes->stavesNumber;
        debugf("staffNumbers copy %i\n", size);
        enum Clef *clefs = currAttributes->clefs;
        currAttributes->clefs = calloc(currAttributes->stavesNumber, sizeof(enum Clef));
        if(clefs){
            memcpy(currAttributes->clefs, clefs, size * sizeof(enum Clef));
            free(clefs);
        }
    }
    StaffNumber staffNumber = currAttributes->stavesNumber;

    if(attributes->clefs){
        for(StaffNumber s = 0; s < staffNumber; s++){
            if(attributes->clefs[s]){
                currAttributes->clefs[s] = attributes->clefs[s];
            }
            debugf("clef %i = %i\n", s, attributes->clefs[s]);
        }
    }
}

// https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/attributes/
struct Attributes *parseAttributes(xmlNodePtr part, struct Attributes *currAtrributes){
    struct Attributes *a = attributesInit();
    xmlNodePtr children = part->xmlChildrenNode;
    while(children){
        if(xmlStrcmp(children->name, XML_CHAR"divisions") == 0){
            long division = parseBody(children);
            a->division = division;

            currAtrributes->division = division;
        }
        else if(xmlStrcmp(children->name, XML_CHAR"key") == 0){
            KeySignature keySignature = 0;
            parseKey(children, &keySignature);

            a->keySignature = keySignature;
            currAtrributes->keySignature = keySignature;
        }
        else if(xmlStrcmp(children->name, XML_CHAR"time") == 0){
            // both denominator and numerator are required so there is no need for checking
            TimeSignature denominator = 0, numerator = 0;
            parseTime(children, &denominator, &numerator);

            a->denominator = denominator;
            a->numerator = numerator;

            currAtrributes->denominator = denominator;
            currAtrributes->numerator = numerator;
        }
        else if(xmlStrcmp(children->name, XML_CHAR"staves") == 0){
            long numberOfStaves = parseBody(children);
            a->stavesNumber = numberOfStaves;
            a->clefs = calloc(numberOfStaves, sizeof(enum Clef));
            currAtrributes->stavesNumber = numberOfStaves;
        }
        else if(xmlStrcmp(children->name, XML_CHAR"clef") == 0){
            parseClef(children, a, currAtrributes);
        }
        children = children->next;
    }
    
    a->stavesNumber = currAtrributes->stavesNumber;
    return a;
}

// <key>
// https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/key/
void parseKey(xmlNodePtr part, KeySignature *keySignature){
    xmlNodePtr children = part->xmlChildrenNode;
    while(children){
        // https://w3c.github.io/musicxml/musicxml-reference/elements/fifths/
        if(xmlStrcmp(children->name, XML_CHAR"fifths") == 0){
            long r = parseBody(children);
            if(r == 0){
                *keySignature = 0;
                continue;
            }

            *keySignature = r;
        }
        else if(xmlStrcmp(children->name, XML_CHAR"cancel") == 0){
            fprintf(stderr, "cancel element\n");
            exit(1);
        }
        children = children->next;
    }
}

// <time>
// https://w3c.github.io/musicxml/musicxml-reference/elements/time/
void parseTime(xmlNodePtr part, TimeSignature *numerator, TimeSignature *denominator){
    xmlNodePtr children = part->xmlChildrenNode;
    while(children){
        if(xmlStrcmp(children->name, XML_CHAR"beats") == 0){
            *numerator = parseBody(children);
        }
        else if(xmlStrcmp(children->name, XML_CHAR"beat-type") == 0){
            *denominator = parseBody(children);
        }

        children = children->next;
    }
}
    // xmlNodePtr children = part->xmlChildrenNode;
    // while(children){
    //     if(xmlStrcmp(children->name, XML_CHAR"divisions") == 0){
    //         parseKey(children, a);
    //     }
    //     else if(xmlStrcmp(children->name, XML_CHAR"key") == 0){
    //
    //     }
    //     children = children->next;
    // }
    //

void parseClef(xmlNodePtr part, struct Attributes *a, struct Attributes *currAtrributes){
    uint8_t number = 0;
    xmlChar *attValue = xmlGetProp(part, XML_CHAR"number");
    if(attValue){
        number = strtol((char*)attValue, NULL, 10) - 1;
    }

    xmlNodePtr children = part->xmlChildrenNode;

    int8_t change = 0;
    int8_t clef = 0;
    while(children){
        if(xmlStrcmp(children->name, XML_CHAR"sign") == 0){
            char *content = (char*)xmlNodeGetContent(children);
            // debugf("content >%s<\n", content);
            
            if(strcmp(content, "G") == 0){
                clef = CLEF_G;
            }
            else if(strcmp(content, "F") == 0){
                clef = CLEF_F;
            }
            else if(strcmp(content, "C") == 0){
                clef = CLEF_C;
            }
            else{
                fprintf(stderr, "missing feature 'clef => sign => %s'\n", content);
                exit(1);
            }
        }
        else if(xmlStrcmp(children->name, XML_CHAR"clef-octave-change") == 0){
            change = (uint8_t)parseBody(children);
        }
        children = children->next;
    }

    if(!a->clefs){
        a->clefs = calloc(currAtrributes->stavesNumber, sizeof(enum Clef));
    }
    a->clefs[number] = clef - change;
}
