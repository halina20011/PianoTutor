#include "inputParser.h"

const int INPUT_TYPES_SIZES[] = {0, 0, 1, 1, 1, 1};

char *joinFlags(struct Input input, int before, int wholeSize){
    char buffer[JOIN_FLAGS_BUFFER + 1];
    int size = 0;
    for(int i = 0; i < before; i++){
        buffer[size++] = ' ';
    }
    
    // join the flags together
    for(uint8_t f = 0; f < input.flagsSize; f++){
        size_t s = strlen(input.flags[f]);
        memcpy(buffer + size, input.flags[f], s);
        size += s;
        // is another flags succeed add ", "
        if(f + 1 < input.flagsSize){
            buffer[size++] = ',';
            buffer[size++] = ' ';
        }
    }
    
    // add spaces so the text after will be aligned
    memset(buffer + size, ' ', wholeSize - size);
    buffer[wholeSize] = 0;

    char *con = malloc(sizeof(char) * wholeSize);
    memcpy(con, buffer, sizeof(char) * wholeSize);
    return con;
}

void printHelpMessage(struct InputParser inputParser){
    char *columnStr = getenv("COLUMNS");
    int windowSize = 0;
    if(columnStr){
        windowSize = atoi(columnStr);
    }

    int flagsSize = inputParser.flagsColSize;
    char empty[inputParser.flagsColSize + 1];
    memset(empty, ' ', inputParser.flagsColSize);
    empty[inputParser.flagsColSize] = 0;

    int messageSize = (windowSize) ? windowSize - flagsSize : 0;

    printf("usage: [flag] [option]\n");
    for(size_t i = 0; i < inputParser.inputsSize; i++){
        struct Input input = inputParser.inputs[i];
        char *str = joinFlags(input, inputParser.flagsOffset, inputParser.flagsColSize);
        printf("%s", str);
        free(str);

        if(messageSize == 0){
            printf("%s\n", input.message);
        }
        else{
            char *mPtr = input.message;
            int size = 0;
            while(*mPtr){
                while(*mPtr == ' ' && size == 0){
                    mPtr++;
                }

                if(*mPtr == '\n'){
                    printf("\n%s", empty);
                    mPtr++;
                    size = 0;
                }
                else if(messageSize <= size){
                    printf("%s", empty);
                    size = 0;
                }
                else{
                    putchar(*mPtr++);
                    size++;
                }
            }
        }
        printf("\n");
    }
}

void parseInput(struct InputParser inputParser, int argc, char **argv){
    for(int i = 1; i < argc; i++){
        bool matches = false;
        for(size_t j = 0; j < inputParser.inputsSize && !matches; j++){
            // printf("j %i\n", j);
            struct Input input = inputParser.inputs[j];
            uint8_t indexMatch = 0;
            for(int f = 0; f < input.flagsSize && !matches; f++){
                if(strcmp(argv[i], input.flags[f]) == 0){
                    matches = true;
                    indexMatch = f;
                }
            }
            if(matches){
                int dataSize = INPUT_TYPES_SIZES[input.dataType];
                if(argc <= i + dataSize){
                    fprintf(stderr, "wrong numer of arguments for flag %s\n", input.flags[indexMatch]);
                    exit(1);
                }
                printf("match \"%s\"\n", input.flags[indexMatch]);
                switch(input.dataType){
                    case INPUT_TYPE_NONE:
                        break;
                    case INPUT_TYPE_SWITCH:
                        *((bool*)input.variable) = true;
                        break;
                    case INPUT_TYPE_STR:
                        *((char**)input.variable) = argv[i + 1];
                        break;
                    case INPUT_TYPE_INT:
                        *((int*)input.variable) = strtol(argv[i + 1], NULL, 10);
                        break;
                    case INPUT_TYPE_UINT8_T:
                        *((uint8_t*)input.variable) = strtol(argv[i + 1], NULL, 10);
                        break;
                    case INPUT_TYPE_FLOAT:
                        printf("float: %s\n", argv[i + 1]);
                        *((float*)input.variable) = strtof(argv[i + 1], NULL);
                        break;
                    default:
                        fprintf(stderr, "this input type has not yet been implemented\n");
                        exit(1);
                }
                i += dataSize;
            }
        }
        if(!matches){
            fprintf(stderr, "unknown flag \"%s\"\n", argv[i]);
            exit(1);
        }
    }
}
