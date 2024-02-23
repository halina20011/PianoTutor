// gcc $(pkg-config --cflags --libs freetype2 ) bitmapFont.c -o ./build/bitmapFont

#include <stdio.h>
#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define TEXT_SIZE 170
#define TEXTURE_WIDTH 128
#define TEXTURE_HEIGHT 128

#define BITMAP_WIDTH TEXTURE_WIDTH
#define BITMAP_HEIGHT TEXTURE_HEIGHT * 17

int main(){
    FT_Library ft;
    if(FT_Init_FreeType(&ft)){
        fprintf(stderr, "could not init freetype library\n");
        return 1;
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
    
    uint8_t bitmap[BITMAP_HEIGHT][BITMAP_WIDTH] = {};

    int wX = 0, wY = 0;
    const char *text = "0123456789CDEFGAH";
    for(int i = 0; text[i]; i++){
        if(FT_Load_Char(face, text[i], FT_LOAD_RENDER)){
            fprintf(stderr, "cound not open char %c\n", text[i]);
            return 1;
        }

        FT_Bitmap *bmp = &g->bitmap;

        for(int y = 0; y < bmp->rows; y++){
            for(int x = 0; x < bmp->width; x++){
                uint8_t pixel = bmp->buffer[y * bmp->width + x];
                bitmap[wY + y][wX + x] = pixel;
            }
        }
        
        // printf("%i %i\n", g->advance.x >> 6, g->advance.y >> 6);
        // printf("%i, %i ", bmp->rows, bmp->width);
        wY += TEXTURE_HEIGHT;
    }

    FILE *file = fopen("src/bitmap.raw", "w");
    if(!file){
        return 1;
    }

    fwrite(bitmap, 1, BITMAP_HEIGHT * BITMAP_WIDTH, file);
    fclose(file);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return 0;
}
