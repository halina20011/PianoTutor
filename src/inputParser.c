#include "inputParser.h"


extern struct InputParser inputParser;

const int INPUT_TYPES_SIZES[] = {0, 0, 1, 1, 1, 1};

bool onlyCountArguments = true;
size_t inputNumberOfArguments = 0;

void inputInit(void (*funcArguments)()){
    onlyCountArguments = true;
    inputNumberOfArguments = 0;
    (*funcArguments)();

    onlyCountArguments = false;
    inputParser.inputsSize = 0;
    inputParser.inputs = malloc(sizeof(struct Input*) * inputNumberOfArguments);
    (*funcArguments)();
}


size_t inputOptionSize(enum InputType type){
    switch(type){
        case INPUT_TYPE_NONE:
            return 0;
        case INPUT_TYPE_STR:
            return sizeof(char*);
        case INPUT_TYPE_INT:
            return sizeof(int);
        case INPUT_TYPE_UINT8_T:
            return sizeof(uint8_t);
        case INPUT_TYPE_FLOAT:
            return sizeof(float);
        case INPUT_TYPE_SWITCH:
        case INPUT_TYPE_ARRAY:
            break;
    }
    return 0;
}

void *inputCreateOptions(enum InputType type, size_t size, ...){
    if(onlyCountArguments){
        return NULL;
    }

    void *options = malloc(inputOptionSize(type) * size);

    va_list args;
    va_start(args, size);

    for(size_t i = 0; i < size; i++){
        // if(type == INPUT_TYPE_INT){
        //     ((int*)options)[i] = va_arg(args, int);
        // }
        if(type == INPUT_TYPE_STR){
            ((char**)options)[i] = strdup(va_arg(args, char*));
        }
        else{
            fprintf(stderr, "format array not suported\n");
            exit(1);
        }
    }
    
    va_end(args);

    return options;
}

void inputAddInput(void *value, enum InputType inputType, const char* message, void *options, size_t optionsSize, size_t size, ...){
    inputNumberOfArguments++;
    if(onlyCountArguments){
        return;
    }

    struct Input *input = malloc(sizeof(struct Input));
    input->dataType = inputType;
    input->value = value;
    input->message = strdup(message);
    
    va_list args;
    va_start(args, size);

    char **flags = malloc(sizeof(char*) * size);

    for(size_t i = 0; i < size; i++){
        char *str = va_arg(args, char*);
        flags[i] = strdup(str);
        printf("added %s\n", flags[i]);
    }
    input->flags = flags;
    input->flagsSize = size;

    va_end(args);

    inputParser.inputs[inputParser.inputsSize++] = input;
}

void inputFree(){
    for(size_t i = 0; inputParser.inputsSize; i++){
        struct Input *input = inputParser.inputs[i];
        for(size_t j = 0; j < input->flagsSize; j++){
            free(input->flags[j]);
        }
        free(input->options);
        free(input->flags);
        free(input);
    }
    free(inputParser.inputs);
}

char *joinFlags(struct Input *input, int before, int wholeSize){
    char buffer[JOIN_FLAGS_BUFFER + 1];
    int size = 0;
    for(int i = 0; i < before; i++){
        buffer[size++] = ' ';
    }
    
    // join the flags together
    for(uint8_t f = 0; f < input->flagsSize; f++){
        size_t s = strlen(input->flags[f]);
        memcpy(buffer + size, input->flags[f], s);
        size += s;
        // is another flags succeed add ", "
        if(f + 1 < input->flagsSize){
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
        struct Input *input = inputParser.inputs[i];
        char *str = joinFlags(input, inputParser.flagsOffset, inputParser.flagsColSize);
        printf("%s", str);
        free(str);

        if(messageSize == 0){
            printf("%s\n", input->message);
        }
        else{
            char *mPtr = input->message;
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

void parseInputType(struct Input *input, int i, char **argv){
    switch(input->dataType){
        case INPUT_TYPE_NONE:
            break;
        case INPUT_TYPE_SWITCH:
            *((bool*)input->value) = true;
            break;
        case INPUT_TYPE_STR:
            *((char**)input->value) = argv[i + 1];
            break;
        case INPUT_TYPE_INT:
            *((int*)input->value) = strtol(argv[i + 1], NULL, 10);
            break;
        case INPUT_TYPE_UINT8_T:
            *((uint8_t*)input->value) = strtol(argv[i + 1], NULL, 10);
            break;
        case INPUT_TYPE_FLOAT:
            printf("float: %s\n", argv[i + 1]);
            *((float*)input->value) = strtof(argv[i + 1], NULL);
            break;
        default:
            fprintf(stderr, "this input type has not yet been implemented\n");
            exit(1);
    }
}

void parseInput(int argc, char **argv){
    for(int i = 1; i < argc; i++){
        bool matches = false;
        printf("input size: %zu\n", inputParser.inputsSize);
        for(size_t j = 0; j < inputParser.inputsSize && !matches; j++){
            // printf("j %i\n", j);
            struct Input *input = inputParser.inputs[j];
            size_t indexMatch = 0;
            for(size_t f = 0; f < input->flagsSize && !matches; f++){
                printf("%s == %s\n", argv[i], input->flags[f]);
                if(strcmp(argv[i], input->flags[f]) == 0){
                    matches = true;
                    indexMatch = f;
                }
            }

            if(matches){
                int dataSize = INPUT_TYPES_SIZES[input->dataType];
                if(argc <= i + dataSize){
                    fprintf(stderr, "wrong numer of arguments for flag %s\n", input->flags[indexMatch]);
                    exit(1);
                }
                printf("match \"%s\"\n", input->flags[indexMatch]);
                if(input->dataType & INPUT_TYPE_ARRAY){
                    input->dataType = ~INPUT_TYPE_ARRAY;
                    fprintf(stderr, "not implemented\n");
                    exit(1);
                    // for(size_t o = 0; 0 < /
                }
                else{
                    parseInputType(input, i, argv);
                }
                i += dataSize;
            }
        }

        if(!matches){
            fprintf(stderr, "unknown flag \"%s\"\n", argv[i]);
            exit(1);
        }
    }

    // inputFree();
}
