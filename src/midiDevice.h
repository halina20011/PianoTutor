#ifndef MIDI_DEVICE
#define MIDI_DEVICE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <stdint.h>

#include "helpers.h"

#include "xmlParser.h"

#define NOTE_ON  0x90
#define NOTE_OFF 0x80

int midiDeviceInit(char *path);
void midiRead(struct Piano *piano);
void sendNoteEvent(int midiDevice, uint8_t eventType, struct NotePitch *pitch, uint8_t velocity);

#endif
