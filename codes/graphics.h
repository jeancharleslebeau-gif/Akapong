#pragma once
#include <cstdint>
#include "lib/graphics_basic.h" 

extern graphics_basic graphics;

// Maj graphique du jeu
void draw_game();

// Affiche l'image d'accueil (intro)
void show_intro_screen();

void intro_image_task(void *pvParameters);
