#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#define DIR_PATH "./src/Shaders/"

char *readFile(const char *fileName){
    FILE *file = fopen(fileName, "rb");
    if(!file){
        fprintf(stderr, "unable to open file: %s\n", fileName);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(sizeof(char) * (size + 1));
    fread(buffer, 1, size, file);
    fclose(file);

    buffer[size] = 0;
    // printf("%s\n", buffer);

    return buffer;
}

void toMacroFormat(char *str, char *buffer){
    int w = 0;
    for(int i = 0; str[i] && str[i] != '.'; i++){
        if(isupper(str[i])){
            buffer[w++] = '_';
        }
        buffer[w++] = toupper(str[i]);
    }

    buffer[w] = 0;
}

void format(FILE *stream, char *fileName, char *fileBuffer){
    char macroBuffer[100];
    toMacroFormat(fileName, macroBuffer);
    fprintf(stream, "\n#define %s \"", macroBuffer);
    char c;
    int size = 1;
    for(int i = 0; fileBuffer[i]; i++){
        if(!size){
            fprintf(stream, "    \"");
        }

        c = fileBuffer[i];
        if(c == '\n'){
            fprintf(stream, "\\n\"");
            if(fileBuffer[i + 1] != 0){
                fprintf(stream, "\\");

            }
            fprintf(stream, "\n");
            size = 0;
        }
        else{
            fprintf(stream, "%c", c);
            size++;
        }
    }
}

void makeShader(){
    DIR *d;
    struct dirent *dir;

    d = opendir(DIR_PATH);
    // FILE *stream = fopen(stdout, "w");
    FILE *stream = stdout;
    if(d){
        while((dir = readdir(d)) != NULL){
            if(dir->d_type != DT_DIR){
                char filePath[255];
                strcpy(filePath, DIR_PATH);
                strcat(filePath, dir->d_name);
                // printf("%s %i\n", filePath, dir->d_type);
                char *fileBuffer = readFile(filePath);
                format(stream, dir->d_name, fileBuffer);
            }
        }
        closedir(d);
    }
}

int main(){
    makeShader();
    return 0;
}
