#pragma once
#include <cstdint> 

typedef struct {
    int x, y;
    int dx, dy;
    int size;
} ball_t;

typedef struct {
    int x, y;
    int w, h;
} paddle_t;

// Variables globales
extern ball_t ball;
extern paddle_t leftPaddle, rightPaddle;
extern int scoreP1, scoreP2;
extern int gameMode, difficulty;
extern int menu_selection;
extern int ai_delay_counter;

// Fonction delay en millisecondes (style Arduino)
void delay(int ms);

// Fonctions utilitaires
void wait_for_key_release(uint16_t key);
void show_message_and_wait(const char* line1, const char* line2);
void reset_scores_and_menu();

// Teste la collision entre la balle et une raquette
bool check_paddle_collision(paddle_t &p, bool isLeft);

// Fonctions principales du jeu
void update_game();
void reset_ball();
void clamp_paddle(paddle_t &p);
void main_loop();
