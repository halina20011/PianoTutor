#ifndef __PIANO_MESHES__
#define __PIANO_MESHES__

enum Meshes{
    // CLEFS
    MESH_NULL,
    F_CLEF_15_UP = 1,
    F_CLEF_8_UP,
    F_CLEF,
    F_CLEF_8_DOWN,
    F_CLEF_15_DOWN,

    G_CLEF_15_UP,
    G_CLEF_8_UP,
    G_CLEF,
    G_CLEF_8_DOWN,
    G_CLEF_15_DOWN,
    C_CLEF,

    COMMON_TIME,
    CUT_TIME,
    
    TIME_0,
    _TIME_1,
    TIME_2,
    TIME_3,
    TIME_4,
    _TIME_5,
    TIME_6,
    _TIME_7,
    TIME_8,
    
    WHOLE_REST,
    HALF_REST,
    EIGHT_REST,
    SIXTEENTH_REST,
    THIRTY_SECOND_REST,
    SIXTY_FOURTH_REST,
    HUNDERT_TWENTY_EIGHT_REST,

    DOUBLE_WHOLE_HEAD,
    WHOLE_HEAD,
    HALF_HEAD,
    QUATER_HEAD,

    TIE,
    NOTE_STEAM,
    NOTE_FLAG,
    NOTE_BEAM,

    FLAG1,
    FLAG2,
    FLAG3,
    FLAG4,
    FLAG5,

    SHARP,
    NATURAL,
    FLAT,

    C, D, E, F, G, A, H,
    C_SHARP, D_SHARP, F_SHARP, G_SHARP, A_SHARP,
    LINES,

    NOTE_START,

    TEXT_START = NOTE_START + 12,
    
    MESHES_SIZE = TEXT_START + 10,
};

#endif
