#include "config.h"
#include "menu.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"
#include "lib/expander.h"
#include "lib/LCD.h"
#include "lib/graphics_basic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern graphics_basic graphics;

void draw_menu() {
    graphics.setColor(color_black);
    graphics.fillRect(0, 0, SCREEN_W, SCREEN_H);

    lcd_move_cursor(100, 40);
    lcd_printf("=== MENU PRINCIPAL ===");

    lcd_move_cursor(100, 80);
    lcd_printf(menu_selection == 0 ? "> Mode 1 joueur" : "  Mode 1 joueur");

    lcd_move_cursor(100, 100);
    lcd_printf(menu_selection == 1 ? "> Mode 2 joueurs" : "  Mode 2 joueurs");

    lcd_move_cursor(100, 140);
    lcd_printf("UP/DOWN pour choisir");
    lcd_move_cursor(100, 160);
    lcd_printf("A pour valider");

    lcd_refresh();
    while (!lcd_refresh_completed());
}

void menu_loop() {
    while (gameMode == 0) {
        uint16_t keys = expander_read();

        if (keys & EXPANDER_KEY_UP) {
            if (menu_selection > 0) menu_selection--;
            draw_menu();
            delay(200);
        }
        if (keys & EXPANDER_KEY_DOWN) {
            if (menu_selection < 1) menu_selection++;
            draw_menu();
            delay(200);
        }
        if (keys & EXPANDER_KEY_A) {
            if (menu_selection == 0) {
                gameMode = 1;
                while (expander_read() & EXPANDER_KEY_A) delay(50);
                choose_difficulty();
                play_intro_music();
            } else {
                gameMode = 2;
                while (expander_read() & EXPANDER_KEY_A) delay(50);
                play_intro_music();
            }
        }

        delay(50);
    }
}

// Affichage du menu de difficulté
static void draw_difficulty(int sel) {
    graphics.setColor(color_black);
    graphics.fillRect(0, 0, SCREEN_W, SCREEN_H);

    lcd_move_cursor(80, 40);
    lcd_printf("== Choisissez la difficulte ==");

    lcd_move_cursor(100, 80);
    lcd_printf(sel == 0 ? "> Facile"     : "  Facile");
    lcd_move_cursor(100, 100);
    lcd_printf(sel == 1 ? "> Normal"     : "  Normal");
    lcd_move_cursor(100, 120);
    lcd_printf(sel == 2 ? "> Dur"        : "  Dur");
    lcd_move_cursor(100, 140);
    lcd_printf(sel == 3 ? "> Impossible" : "  Impossible");

    lcd_move_cursor(100, 180);
    lcd_printf("UP/DOWN pour choisir");
    lcd_move_cursor(100, 200);
    lcd_printf("A pour valider, C retour");

    lcd_refresh();
    while (!lcd_refresh_completed());
}

void choose_difficulty() {
    int sel = difficulty; // point de départ = dernière difficulté choisie
    if (sel < 0 || sel > 3) sel = 0;

    uint16_t prevKeys = 0;

    while (true) {
        draw_difficulty(sel);

        uint16_t keys = expander_read();
        uint16_t pressed = keys & ~prevKeys; // détection d'edges (nouvelles pressions)

        if (pressed & EXPANDER_KEY_UP) {
            if (sel > 0) sel--;
        }

        if (pressed & EXPANDER_KEY_DOWN) {
            if (sel < 3) sel++;
        }

        if (pressed & EXPANDER_KEY_A) {
            // valider la difficulté
            difficulty = sel;
            // attendre relâchement pour éviter double validation
            while (expander_read() & EXPANDER_KEY_A) vTaskDelay(10);
            vTaskDelay(150); // anti-rebond doux
            break; // sortir et lancer le jeu ensuite (dans app_main/reset)
        }

        if (pressed & EXPANDER_KEY_C) {
            // retour au menu principal
            while (expander_read() & EXPANDER_KEY_C) vTaskDelay(10);
            vTaskDelay(150);

            gameMode = 0;
            menu_selection = 0;

            graphics.setColor(color_black);
            graphics.fillRect(0, 0, SCREEN_W, SCREEN_H);

            draw_menu();
            menu_loop(); // le flux complet du menu (choix mode, etc.)

            // en sortie du menu, si mode 1 joueur, repasser par le choix de difficulté
            if (gameMode == 1) {
                // re‑afficher le choix de difficulté (l'utilisateur a demandé retour)
                continue;
            } else {
                // si mode 2 joueurs choisi, on quitte l'écran de difficulté
                break;
            }
        }

        prevKeys = keys;
        vTaskDelay(20);
    }
}
