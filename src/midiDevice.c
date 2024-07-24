#include "midiDevice.h"

#define MIDI_MATCH_PATH "/dev/snd/midi*"

int midiDeviceInit(char *path){
    char *midiDevicePath = NULL;
    if(strcmp(path, "auto") == 0){
        // man 3 glob
        glob_t globResult;
        int globStatus = glob(MIDI_MATCH_PATH, 0, NULL, &globResult);
        if(globStatus){
            fprintf(stderr, "no file that matches %s\n", MIDI_MATCH_PATH);
            exit(1);
        }

        char *firstMatch = globResult.gl_pathv[0];
        if(firstMatch){
            debugf("matches %s\n", firstMatch);
            midiDevicePath = firstMatch;
        }
    }

    if(midiDevicePath == NULL){
        fprintf(stderr, "no midi device\n");
        exit(1);
    }
    
    int f = open(midiDevicePath, O_RDWR | O_NONBLOCK);
    if(f < 0){
        fprintf(stderr, "failed to open midi device\n");
        exit(1);
    }

    return f;
}

int isStatusByte(uint8_t byte) {
    return (byte & 0x80) != 0;
}

int getNumDataBytes(uint8_t statusByte) {
    switch (statusByte & 0xF0) {
        case 0x80:  // Note Off
        case 0x90:  // Note On
        case 0xA0:  // Polyphonic Aftertouch
        case 0xB0:  // Control Change
        case 0xE0:  // Pitch Bend
            return 2;
        case 0xC0:  // Program Change
        case 0xD0:  // Channel Aftertouch
            return 1;
        default:
            return 0;  // Invalid status byte
    }
}

uint8_t stepArray[] = {0, 2, 4, 5, 7, 9, 11};

uint8_t toPitch(struct NotePitch *pitch){
    uint8_t step = stepArray[pitch->step];
    uint8_t r = (pitch->octave) * 12 + step + pitch->alter;
    // debugf("%i %i %i\n", step, pitch->stepChar, r);
    return r;
}

void sendNoteEvent(int midiDevice, uint8_t eventType, struct NotePitch *notePitch, uint8_t velocity){
    if(midiDevice == -1){
        return;
    }
    uint8_t pitch = toPitch(notePitch);
    // debugf("[%i:%i] => %i\n", notePitch->step, notePitch->octave, pitch);
    // debugf("MIDI message: %02X %02X %02X\n", eventType, pitch, velocity);
    // debugf("MIDI message: %03i %03i %03i\n", eventType, pitch, velocity);
    uint8_t message[] = {eventType, pitch, velocity};
    write(midiDevice, message, sizeof(message));
}

void midiRead(int midiDevice, uint8_t *noteBuffer){
    if(midiDevice == -1){
        return;
    }
    uint8_t statusByte = 0;
    uint8_t dataBytes[2];
    uint8_t byte;
    int numDataBytes = 0;

    while(1){
        ssize_t bytesRead = read(midiDevice, &byte, sizeof(byte));
        if(bytesRead < 0){
            return;
        }

        if(isStatusByte(byte)) {
            statusByte = byte;
            numDataBytes = getNumDataBytes(statusByte);
            // if(numDataBytes == 0){
            //     debugf("Invalid status byte: 0x%02X\n", statusByte);
            // }
        }
        else{
            if(0 < numDataBytes){
                dataBytes[numDataBytes - 1] = byte;
                numDataBytes--;

                if (numDataBytes == 0){
                    debugf("MIDI Event: Status=%i, Data1=%i, Data2=%i\n", statusByte, dataBytes[0], dataBytes[1]);
                    uint8_t note = dataBytes[1];
                    if(statusByte == NOTE_ON){
                        noteBuffer[note] = 1;
                    }
                    else if(statusByte == NOTE_OFF){
                        noteBuffer[note] = 0;
                    }
                    return;
                }
            } 
            else{
                debugf("Invalid MIDI data byte: 0x%02X\n", byte);
                return;
            }
        }
    }
}
