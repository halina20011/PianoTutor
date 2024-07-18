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
            printf("matches %s\n", firstMatch);
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

void sendNoteEvent(int midiDevice, uint8_t eventType, uint8_t noteNumber, uint8_t velocity){
    // printf("MIDI message: %02X %02X %02X\n", eventType, noteNumber, velocity);
    uint8_t message[] = {eventType, noteNumber, velocity};
    write(midiDevice, message, sizeof(message));
}

void midiRead(int fd, uint8_t *noteBuffer){
    uint8_t statusByte = 0;
    uint8_t dataBytes[2];
    uint8_t byte;
    int numDataBytes = 0;

    while(1){
        ssize_t bytesRead = read(fd, &byte, sizeof(byte));
        if(bytesRead < 0){
            return;
        }

        if(isStatusByte(byte)) {
            statusByte = byte;
            numDataBytes = getNumDataBytes(statusByte);
            // if(numDataBytes == 0){
            //     printf("Invalid status byte: 0x%02X\n", statusByte);
            // }
        }
        else{
            if(numDataBytes > 0){
                dataBytes[numDataBytes - 1] = byte;
                numDataBytes--;

                if (numDataBytes == 0){
                    printf("MIDI Event: Status=%i, Data1=%i, Data2=%i\n", statusByte, dataBytes[0], dataBytes[1]);
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
                printf("Invalid MIDI data byte: 0x%02X\n", byte);
                return;
            }
        }
    }
}
