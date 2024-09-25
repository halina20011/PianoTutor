flags = -f ~/Documents/Notes/Megalovania.mxl
# flags = -f ~/Documents/Notes/Prlude_Opus_28_No._4_in_E_Minor__Chopin.mxl
# flags = -f ~/Documents/Notes/Mr_Doctor_Man_-_Palaye_Royale.mxl
# flags = -f ~/Documents/Notes/ainekuraine.mxl
# flags = -f ~/Documents/Notes/Bad_Apple_Original_ver.mxl
# flags = -f ~/Documents/Notes/Giornos_Theme.mxl
# flags = -f ~/Documents/Notes/HOUSE_OF_WOLVES_-_MY_CHEMICAL_ROMANCE_-_PIANO_ARRANGEMENT.mxl
# flags = -f ~/Documents/Notes/Life_Eternal__Ghost_Piano_advanced.mxl
# flags = -f ~/Documents/Notes/Rats.mxl
# flags = -f ~/Documents/Notes/Pirates_of_The_Caribbean_Medley.mxl
# flags = -f ~/Documents/Notes/Rats.mxl
# flags = -f ~/Documents/Notes/Shreksophone__Shrek_and_Donkey.mxl
# flags = -f ~/Documents/MuseScore4/Scores/test.mxl

# flags += --hide-keyboard --hide-notes

run: make
	./build/main ${flags}

plot: flags += --plot
plot: make
	# ./build/main ${flags} | tee /dev/tty | python livePlot.py
	./build/main ${flags} | python livePlot.py
# plot:
# 	gcc ./build/test.c -lm -o ./build/main
# 	./build/main ${flags} | tee /dev/tty | grep "PLOT: " | sed -n 's/PLOT: \[\(.*\)\] \(.*\)/\2/p' | tee /dev/tty | gnuplot -persist livePlot.gp

make:
	cd build && make

shaders: makeShaders.c
	gcc makeShaders.c -o ./build/makeShaders && ./build/makeShaders | tee src/shaders.h
	touch ./src/graphics.c

debug: make
	gdb -ex "set debuginfod enabled off" -ex "set args $(flags)" -ex run ./build/main

init:
	mkdir build;
	cd build && cmake ..

sync:
	rsync -avz --exclude 'build' --exclude '.git' 192.168.11.245:/${HOME}/Programming/C/PianoTutor/ ./
