#ifndef MIDI_DEVICE
#define MIDI_DEVICE

#include <fcntl.h>
#include <unistd.h>
#include <glob.h>

#include "midiParser.h"

#define NOTE_ON  0x90
#define NOTE_OFF 0x80

int midiDeviceInit(char *path);
void midiRead(int fd, uint8_t *noteBuffer);
void sendNoteEvent(int midiDevice, uint8_t eventType, uint8_t noteNumber, uint8_t velocity);

#endif
