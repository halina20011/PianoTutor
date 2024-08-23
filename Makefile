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

run: make
	./build/main ${flags}

make:
	cd build && make

debug: make
	gdb -ex "set debuginfod enabled off" -ex "set args $(flags)" -ex run ./build/main

init:
	mkdir build;
	cd build && cmake ..
