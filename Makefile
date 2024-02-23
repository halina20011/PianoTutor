CC = gcc

FILES = $(wildcard ./src/*.c)
OBJECTS = $(patsubst ./src/%.c, ./build/%.o, $(FILES))
# LDFLAGS = -lm -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lGLEW -lEGL -lGL -lGLU -lOpenGL
LDFLAGS = $(shell pkg-config --static --libs glew) $(shell pkg-config --libs glfw3)

CPPFLAGS =
CFLAGS = -Wall -Wextra -Wshadow

.PHONY: update urun

db: 
	@echo $(FILES)
	@echo $(OBJECTS)
	@echo $(LDFLAGS)

build/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<;

main: $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o ./build/$@

run: main
	./build/main

bitmap: bitmapFont.c
	gcc $(shell pkg-config --cflags --libs freetype2) bitmapFont.c -o ./build/bitmapFont && ./build/bitmapFont

viewBitmap: bitmap
	magick -size 128x2176 -depth 8 gray:src/bitmap.raw build/preview.png
	feh ./build/preview.png

shaders: makeShaders.c
	gcc makeShaders.c -o ./build/makeShaders && ./build/makeShaders | tee src/shaders.h

update:
	rsync -avz --exclude "build/*" ${USER}@192.168.0.248:/home/${USER}/Programming/C/PianoTutor/ ./

urun: update run

debug: CFLAGS += -O0 -ggdb
debug: main
	@echo ${DARGS}
	gdb -ex "set debuginfod enabled off" -ex "set args $(DARGS)" -ex run ./build/main
