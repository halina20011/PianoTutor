// gcc $(pkg-config --cflags --libs freetype2 ) bitmapFont.c -o ./build/bitmapFont

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define TEXT_SIZE 120
#define TEXTURE_WIDTH 128
#define TEXTURE_HEIGHT 128

int main(){
    FT_Library ft;
    if(FT_Init_FreeType(&ft)){
        fprintf(stderr, "could not init freetype library\n");
        return 1;
    }

    char stack[128];
    int stackSize = 0;
    int wX = 0, wY = 0;
    for(uint8_t i = 0; i < 128; i++){
        if(isprint(i)){
            stack[stackSize++] = i;
        }
    }

    FT_Face face;
    // const char *fontFile = "/usr/share/fonts/gnu-free/FreeMono.otf";
    // const char *fontFile = "/usr/share/fonts/noto/NotoSans-Black.ttf";
    const char *fontFile = "/usr/share/fonts/RobotoMonoNerdFont/RobotoMonoNerdFont-Regular.ttf";
    if(FT_New_Face(ft, fontFile, 0, &face)){
        fprintf(stderr, "could not open font\n");
        return 1;
    }

    FT_Set_Pixel_Sizes(face, 0, TEXT_SIZE);
    FT_GlyphSlot g = face->glyph;
    size_t width = TEXTURE_WIDTH;
    size_t height = TEXTURE_HEIGHT * stackSize;
    uint8_t bitmap[height][width];
    int max = 0;
    for(int i = 0; i < stackSize; i++){
        char c = stack[i];
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)){
            fprintf(stderr, "cound not open char %c\n", c);
            return 1;
        }
        
        FT_Bitmap *bmp = &g->bitmap;
        int bmpHeight = bmp->rows;
        int bmpWidth = bmp->width;

        if(max < bmpHeight){
            max = bmpHeight;
        }

        // add y offset so every char will be on text line
        for(int y = 0; y < bmpHeight; y++){
            for(int x = 0; x < bmpWidth; x++){
                uint8_t pixel = bmp->buffer[y * bmpWidth + x];
                bitmap[wY + y][wX + x] = pixel;
            }
        }
        
        int lastRow = wY + TEXTURE_HEIGHT - 1;
        int lastColumn = wX + TEXTURE_WIDTH - 1;
        // printf("%i,%i\n", lastRow, lastColumn);
        int8_t diffY = bmpHeight - g->bitmap_top;
        uint8_t offsetY = 0;
        if(offsetY < 0){
            offsetY = -diffY;
        }

        // printf("%c %i %i %i\n", c, g->bitmap_left, g->bitmap_top, );
        // bitmap[lastRow][lastColumn - 1] = g->bitmap_left;
        // bitmap[lastRow][lastColumn] = g->bitmap_top;
        bitmap[lastRow][lastColumn - 1] = bmpWidth;
        bitmap[lastRow][lastColumn] = diffY;
        // printf("%c [x, y] [%i, %i] [%i, %i]\n", c, bmpWidth, bmpWidth, g->bitmap_left, g->bitmap_top);
        // printf("%c %i %i\n", c, bmpWidth, bmpHeight);
        wY += TEXTURE_HEIGHT;
    }
    
    printf("count %i\n", stackSize);
    printf("max height %i\n", max);

    FILE *file = fopen("src/bitmap.raw", "w");
    if(!file){
        return 1;
    }

    fwrite(bitmap, 1, height * width, file);
    fclose(file);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    printf("%ix%i\n", width, height);
    return 0;
}
