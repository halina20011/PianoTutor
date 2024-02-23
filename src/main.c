#include <stdio.h>

#include "midiParser.h"
#include "piano.h"

// void test(uint8_t *data){
//     uint32_t i = 0;
//     uint32_t r = variableLengthValue(data, &i);
//     printf("%X %u\n", r, r);
// }
//

// TODO: parse input arguments
// TODO: correct track numbers
// TODO: use correct tempto from Song struct
// TODO: create note sheet window

int main(){
    // test((uint8_t[]){0x00});
    // test((uint8_t[]){0x40, 0x00});
    // test((uint8_t[]){0x7F, 0x00});
    // test((uint8_t[]){0x81, 0x00});
    // test((uint8_t[]){0xC0, 0x00});
    // test((uint8_t[]){0xFF, 0x7F});
    // test((uint8_t[]){0x81, 0x80,0x80,0x00});
    // test((uint8_t[]){0x85, 0x2b, 0x3f, 0x00});
    // calculateNoteDurations();
    // struct Song *song = midiParser("/home/mario/Documents/Notes/Thomas_the_tank_engine_theme.mid");
    // struct Song *song = midiParser("/home/mario/Documents/Notes/houseOfWolves.mid");
    // struct Song *song = midiParser("/home/mario/Documents/Notes/King_For_a_Day_-_Pierce_the_Veil.mid");
    // struct Song *song = midiParser("/home/mario/Documents/Notes/Mr_Doctor_Man_-_Palaye_Royale.mid");
    // struct Song *song = midiParser("/home/mario/Documents/Notes/Year_Zero.mid");
    // struct Song *song = midiParser("/home/mario/Documents/Notes/Rats.mid");
    // struct Song *song = midiParser("/home/mario/Documents/Notes/Life_Eternal__Ghost_Piano_advanced.mid");
    // struct Song *song = midiParser("/home/mario/Documents/Notes/YOASOBI_marasy_full_ver..mid");
    // struct Song *song = midiParser("/home/mario/Documents/Notes/The_World_-_Death_Note.mscz.mid");
    // struct Song *song = midiParser("/home/mario/Downloads/chopin_prelude_28_4.mid");
    struct Song *song = midiParser("/home/mario/Documents/Notes/Prlude_Opus_28_No._4_in_E_Minor__Chopin.mid");
    analyse(song, 1000);
    // struct Song *song = midiParser("/home/mario/Documents/Notes/Fantaisie-Impromptu_in_C_Minor__Chopin.mid");
    // struct Song *song = generateSong();
    // struct Song *song = midiParser("/home/mario/Downloads/test.mid");
    // struct Song *song = midiParser("/home/mario/Documents/Notes/Giornos_Theme.mid");
    uint8_t trackMask = 1;
    uint8_t noteMask = (1 << C);
    // uint8_t noteMask = 0xff;
    // selectRegion(song, trackMask);
    pianoInit();
    playSong(song, noteMask);
    // learnSong(song, TUTORIAL, trackMask, noteMask);
    pianoExit();
    // midiParser("./build/twinkle.mid");
    return 0;
}
