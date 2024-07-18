#ifndef MIDI_PIANO
#define MIDI_PIANO

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "color.h"
#include "graphics.h"
#include "midiDevice.h"
#include "text.h"
#include "window.h"

#define C_NOTE           0
#define C_NOTE_SHARP     1
#define D_NOTE           2
#define D_NOTE_SHARP     3
#define E_NOTE           4
#define F_NOTE           5
#define F_NOTE_SHARP     6
#define G_NOTE           7
#define G_NOTE_SHARP     8
#define A_NOTE           9
#define A_NOTE_SHARP     10
#define H_NOTE           11

enum NOTES{
    C,
    D,
    E,
    F,
    G,
    A,
    H
};

#define TUTORIAL 0

struct PressedNote{
    uint8_t type;
    float end;
};

void midiPianoInit();
void midiPianoExit();

void midiPrecalculateNotes();
void midiSelectRegion(struct MidiSong *song, uint8_t trackMask);

void midiDrawVisibleNotes(struct MidiSong *song, uint8_t *noteBuffer, uint8_t trackMask, uint8_t noteMask);

void midiLearnSong(struct MidiSong *song, int midiDevice, uint8_t mode, uint8_t trackMask, uint8_t notesNameMask);
void midiPlaySong(struct MidiSong *song, int midiDevice, uint8_t notesNameMask);

#endif
