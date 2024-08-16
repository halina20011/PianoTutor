CC = gcc

FILES = $(wildcard ./src/*.c)
OBJECTS = $(patsubst ./src/%.c, ./build/%.o, $(FILES))
# LDFLAGS = -lm -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lGLEW -lEGL -lGL -lGLU -lOpenGL
# zlib
LDFLAGS = -lc -lm \
	$(shell pkg-config --libs cglm) \
	$(shell pkg-config --static --libs glew) \
	$(shell pkg-config --libs glfw3) \
	$(shell xml2-config --libs) \
	$(shell pkg-config --libs libzip)

CPPFLAGS = $(shell xml2-config --cflags)
CFLAGS = -Wall -Wextra -Wshadow

# CFLAGS += -D'FUNCTION_CHECKER=' -finstrument-functions

.PHONY: update urun

db: 
	@echo $(FILES)
	@echo $(OBJECTS)
	@echo $(LDFLAGS)

build/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<;

main: $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o ./build/$@

# flags = -m /home/mario/Documents/Notes/houseOfWolves.mid
# flags = -m /home/mario/Documents/Notes/Life_Eternal__Ghost_Piano_advanced.mid --upbeat 2 
# flags = -m /home/mario/Documents/Notes/houseOfWolves.mid --percision 5 --analyse
# flags = -m /home/mario/Documents/Notes/The_World_-_Death_Note.mscz.mid
# flags = -m /home/mario/Documents/Notes/aineKleine.mid
# flags = -m /home/mario/Documents/Notes/aineKleine.mid
# flags = -m /home/mario/Documents/Notes/Beggin.mid
# flags = -m /home/mario/Documents/Notes/Gossip.mid
# flags = -m /home/mario/Documents/Notes/famousLastWords.mid
# flags = -m /home/mario/Documents/Notes/Fantaisie-Impromptu_in_C_Minor__Chopin.mid
# flags = -m /home/mario/Documents/Notes/Pirates_of_The_Caribbean_Medley.mid
# flags = -m /home/mario/Documents/Notes/Prlude_Opus_28_No._4_in_E_Minor__Chopin.mid --percision 5 --analyse
# flags = -m /home/mario/Documents/Notes/Prlude_Opus_28_No._4_in_E_Minor__Chopin.mid

# MXL
flags = -f /home/mario/Documents/Notes/Megalovania.mxl
# flags = -f /home/mario/Documents/Notes/Mr_Doctor_Man_-_Palaye_Royale.mxl
# flags = -f /home/mario/Documents/Notes/ainekuraine.mxl
# flags = -f /home/mario/Documents/Notes/Bad_Apple_Original_ver.mxl
# flags = -f /home/mario/Documents/Notes/Giornos_Theme.mxl
# flags = -f /home/mario/Documents/Notes/HOUSE_OF_WOLVES_-_MY_CHEMICAL_ROMANCE_-_PIANO_ARRANGEMENT.mxl
# flags = -f /home/mario/Documents/Notes/Life_Eternal__Ghost_Piano_advanced.mxl
# flags = -f /home/mario/Documents/Notes/Rats.mxl
# flags = -f /home/mario/Documents/Notes/Pirates_of_The_Caribbean_Medley.mxl
# flags = -f /home/mario/Documents/Notes/Rats.mxl
# flags = -f /home/mario/Documents/Notes/Shreksophone__Shrek_and_Donkey.mxl
# flags = -f /home/mario/Documents/Notes/Prlude_Opus_28_No._4_in_E_Minor__Chopin.mxl
# flags = -f /home/mario/Documents/MuseScore4/Scores/test.mxl
# flags = --help
# flags += --durations
# flags += --upbeat 1 -c preludeCF.txt
# flags += --percision 4

run: main
	./build/main $(flags)

bitmap: bitmapFont.c
	gcc $(shell pkg-config --cflags --libs freetype2) bitmapFont.c -o ./build/bitmapFont && ./build/bitmapFont

viewBitmap: bitmap
	magick -size 128x2176 -depth 8 gray:src/bitmap.raw build/preview.png
	feh ./build/preview.png

shaders: makeShaders.c
	gcc makeShaders.c -o ./build/makeShaders && ./build/makeShaders | tee src/shaders.h
	touch ./src/graphics.c

update:
	rsync -avz --exclude "build/*" ${USER}@192.168.0.248:/home/${USER}/Programming/C/PianoTutor/ ./

urun: update run

debug: CFLAGS += -O0 -ggdb -fstack-protector-all -fsanitize=address
debug: main
	gdb -ex "set debuginfod enabled off" -ex "set args $(flags)" -ex run ./build/main

debugMan: CFLAGS += -O0 -ggdb
debugMan: main
	gdb -ex "set debuginfod enabled off" -ex "set args $(flags)" ./build/main
