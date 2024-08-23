#ifndef NOTE_TYPES
#define NOTE_TYPES

#include <stdint.h>

typedef uint16_t Division;
typedef int8_t KeySignature;
typedef uint8_t TimeSignature;
typedef uint8_t StaffNumber;

#define BEAM_BIT_SIZE 4
#define BEAM_BIT_MASK ((1 << (BEAM_BIT_SIZE + 1)) - 1)

#define BEAM_ENABLED 0
#define BEAM_OFF 1
#define BEAM_HOOK_ENABLED 2
#define BEAM_HOOK_BACKWARD 3

#define SET_BEAM(variable, beamNumber, value) (variable |= (value << (beamNumber * BEAM_BIT_SIZE)))

#define GET_BEAM(variable, beamNumber) ((variable >> ((beamNumber) * BEAM_BIT_SIZE)) & BEAM_BIT_MASK)
// https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/beam/
/*
 * type of beam that note has
 *
 * beam is groupt by 4 bits
 * 1 - beam enabled | beam dissabled
 * 2 - beam ends here | beam start/continue
 * 3 - hook enabled | beam does not have a beam
 * 4 - backward hook | forward hook
 */

typedef uint8_t Beam;
typedef uint32_t Beams;

typedef uint8_t Pitch;

#define PITCH_MIN 0
#define PITCH_MAX UINT8_MAX

typedef uint8_t NoteFlags;
typedef uint16_t NoteDuration;
typedef uint8_t Tuplet;
typedef uint16_t MeasureSize;

typedef float NotePositionExtreme[2];

typedef uint8_t ChordSize;

#endif
