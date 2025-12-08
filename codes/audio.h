#pragma once

#include <cstdint>
#include "lib/tone.h"
#include "lib/audio_basic.h"   // pour audio_player

typedef struct {
    float freq1; uint16_t dur1;
    float freq2; uint16_t dur2;
    float freq3; uint16_t dur3;
} note_t;

extern audio_track_tone tone1, tone2, tone3;
extern audio_player player;
extern audio_track_tone tone_ball;

// Déclaration du tableau (défini ailleurs)
extern const note_t melody[];
extern const int melody_intro_len;

// Fonctions audio

void play_intro_music();
void intro_music_task(void *pvParameters);