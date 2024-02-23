#ifndef PIANO
#define PIANO

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "graphics.h"
#include "midiDevice.h"

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

void pianoInit();
void pianoExit();

void precalculateNotes();
void selectRegion(struct Song *song, uint8_t trackMask);

void drawVisibleNotes(struct Song *song, uint8_t *noteBuffer, uint8_t trackMask, uint8_t noteMask);
void drawSong(struct Song *song, uint8_t *noteBuffer, int midiDevice);
void learnSong(struct Song *song, uint8_t mode, uint8_t trackMask, uint8_t noteMask);
void playSong(struct Song *song, uint8_t noteMask);

#endif
