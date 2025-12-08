// composants du programme Akapong 
#include "config.h"
#include "game.h"
#include "menu.h"
#include "graphics.h"
#include "audio.h"
#include "image_intro.h" 

// gestion des différents composants de la console
#include "lib/expander.h"
#include "lib/audio.h"
#include "lib/audio_basic.h"
#include "lib/graphics_basic.h"
#include "lib/LCD.h"
#include "lib/tone.h"

// composants systèmes pour delay par exemple
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdlib.h>

ball_t ball = {SCREEN_W/2, SCREEN_H/2, 2, 2, 4};
paddle_t leftPaddle = {PADDLE_SPACE, (SCREEN_H - PADDLE_H)/2, PADDLE_W, PADDLE_H};
paddle_t rightPaddle = {SCREEN_W-PADDLE_SPACE-PADDLE_W, (SCREEN_H - PADDLE_H)/2, PADDLE_W, PADDLE_H};
int scoreP1 = 0, scoreP2 = 0;
int gameMode = 0, difficulty = 0;
int menu_selection = 0;
int ai_delay_counter = 0;

extern graphics_basic graphics;
extern audio_track_tone tone_ball;
extern audio_player player;

// Implémentation de delay en millisecondes
void delay(int ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

// Attente du relâchement d’une touche
void wait_for_key_release(uint16_t key)
{
    while (expander_read() & key)
    {
        delay(50);
    }
}

// Affiche un message sur deux lignes et attend validation par A
void show_message_and_wait(const char* line1, const char* line2)
{
    graphics.setColor(color_black);
    graphics.fillRect(0, 0, SCREEN_W, SCREEN_H);

    lcd_move_cursor(60, 100);
    lcd_printf("%s", line1);

    lcd_move_cursor(60, 140);
    lcd_printf("%s", line2);

    lcd_refresh();
    while (!lcd_refresh_completed());

    while (1)
    {
        uint16_t keys = expander_read();
        if (keys & EXPANDER_KEY_A)
        {
            wait_for_key_release(EXPANDER_KEY_A);
            break;
        }
        delay(50);
    }
}

// Vérifie la collision entre la balle et une raquette
// isLeft = true pour la raquette gauche, false pour la raquette droite
bool check_paddle_collision(paddle_t &p, bool isLeft)
{
    if (ball.y >= p.y && ball.y <= p.y + p.h)
    {
        if (isLeft)
        {
            if (ball.x - ball.size <= p.x + p.w && ball.x >= p.x)
            {
                ball.x = p.x + p.w + ball.size;
                ball.dx = abs(ball.dx); // renvoie vers la droite
                tone_ball.play_tone(523.0, 100);
				player.pool();
                return true;
            }
        }
        else
        {
            if (ball.x + ball.size >= p.x && ball.x - ball.size <= p.x + p.w)
            {
                ball.x = p.x - ball.size;
                ball.dx = -abs(ball.dx); // renvoie vers la gauche
                tone_ball.play_tone(587.0, 100);
				player.pool();
                return true;
            }
        }
    }
    return false;
}

// Réinitialise les scores et retourne au menu
void reset_scores_and_menu()
{
    scoreP1 = 0;
    scoreP2 = 0;
    gameMode = 0;
    menu_selection = 0;
	graphics.fillRect(0, 0, SCREEN_W, SCREEN_H); // effacer écran
draw_menu();
menu_loop();   // ⚡ reste actif jusqu’à ce qu’un mode soit choisi

while (true) {
    main_loop(); // ⚡ ensuite le jeu tourne en continu
}
}

void clamp_paddle(paddle_t &p) {
    if (p.y < 0) p.y = 0;
    if (p.y + p.h > SCREEN_H) p.y = SCREEN_H - p.h;
}

// Relance la balle avec une direction diagonale aléatoire
void reset_ball()
{
    ball.x = SCREEN_W/2;
    ball.y = SCREEN_H/2;

    int speed = 2; // vitesse horizontale de base

    // choisir une des 4 directions : 0=haut droite, 1=bas droite, 2=haut gauche, 3=bas gauche
    int dir = rand() % 4;

    // petite variation verticale (entre 1 et 3 pixels par tick)
    int dy_variation = (rand() % 3) + 1;

    switch (dir)
    {
    case 0: // haut droite
        ball.dx = speed;
        ball.dy = -dy_variation;
        break;
    case 1: // bas droite
        ball.dx = speed;
        ball.dy = dy_variation;
        break;
    case 2: // haut gauche
        ball.dx = -speed;
        ball.dy = -dy_variation;
        break;
    case 3: // bas gauche
        ball.dx = -speed;
        ball.dy = dy_variation;
        break;
    }
}

void update_game()
{
    // lecture clavier
    uint16_t keys = expander_read();

    // mouvement raquette gauche (joueur 1)
    if (keys & EXPANDER_KEY_UP)
        leftPaddle.y -= 3;
    if (keys & EXPANDER_KEY_DOWN)
        leftPaddle.y += 3;

    // retour au menu si C est pressé
    if (keys & EXPANDER_KEY_C)
    {
        show_message_and_wait("Retourner au menu principal ?", "A: Oui   B: Non");

        while (1)
        {
            uint16_t confirm = expander_read();
            if (confirm & EXPANDER_KEY_A)
            {
                wait_for_key_release(EXPANDER_KEY_A);
                reset_scores_and_menu();
                break;
            }
            if (confirm & EXPANDER_KEY_B)
            {
                wait_for_key_release(EXPANDER_KEY_B);
                break;
            }
            delay(50);
        }
    }

    // raquette droite : joueur 2 ou IA
    if (gameMode == 1)
    {
        // paramètres IA selon difficulté
        int ai_speed;
        int reactionDelay;
        float errorChance;

        switch (difficulty)
        {
        case 0: ai_speed = 3; reactionDelay = 3; errorChance = 0.10f; break;
        case 1: ai_speed = 4; reactionDelay = 3; errorChance = 0.08f; break;
        case 2: ai_speed = 4; reactionDelay = 1; errorChance = 0.06f; break;
        default: ai_speed = 5; reactionDelay = 0; errorChance = 0.04f; break;
        }

        if (ai_delay_counter < reactionDelay)
        {
            ai_delay_counter++;
        }
        else
        {
            ai_delay_counter = 0;
            bool wrongDirection = ((float)rand() / (float)RAND_MAX) < errorChance;
            int target = rightPaddle.y + rightPaddle.h / 2;
            if (ball.y > target)
                rightPaddle.y += wrongDirection ? -ai_speed : ai_speed;
            else if (ball.y < target)
                rightPaddle.y += wrongDirection ? ai_speed : -ai_speed;
        }
    }
    else if (gameMode == 2)
    {
        if (keys & EXPANDER_KEY_D)
            rightPaddle.y -= 3;
        if (keys & EXPANDER_KEY_A)
            rightPaddle.y += 3;
    }

    // éteindre la console
    if (keys & EXPANDER_KEY_RUN)
        expander_power_off();

    // limites écran
    clamp_paddle(leftPaddle);
    clamp_paddle(rightPaddle);

    // mouvement balle
    ball.x += ball.dx;
    ball.y += ball.dy;

    // rebond haut/bas
    if (ball.y < ball.size || ball.y > SCREEN_H - ball.size)
    {
        ball.dy = -ball.dy;
        tone_ball.play_tone(330.0, 120);
		player.pool();
    }

    // collision raquette gauche
	check_paddle_collision(leftPaddle, true);


    // collision raquette droite
	check_paddle_collision(rightPaddle, false);

    // sortie balle → reset
    if (ball.x < 0)
    {
        tone_ball.play_tone(220.0, 300);
		player.pool();
        scoreP2++;
        reset_ball();
    }
    else if (ball.x > SCREEN_W)
    {
        tone_ball.play_tone(220.0, 300);
		player.pool();
        scoreP1++;
        reset_ball();
    }

    // condition de victoire
    if (scoreP1 >= WIN_SCORE || scoreP2 >= WIN_SCORE)
    {
        if (scoreP1 >= WIN_SCORE)
        {
            if (gameMode == 1)
                show_message_and_wait("Victoire ! Felicitations Joueur 1 !", "Appuyez sur A pour continuer");
            else
                show_message_and_wait("Victoire ! Joueur 1 gagne !", "Appuyez sur A pour continuer");
        }
        else 
            if (gameMode == 1)
                show_message_and_wait("Victoire de l'IA !", "Appuyez sur A pour continuer");
            else
                show_message_and_wait("Victoire ! Joueur 2 gagne !", "Appuyez sur A pour continuer");

        reset_scores_and_menu();
    }

    player.pool(); // nécessaire pour jouer les sons
}


void main_loop() {
    update_game();
    draw_game();
}
