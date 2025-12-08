/*
 * Pong pour la Gamebuino Aka
 * Réalisé par Jean-Charles LEBEAU
 * le 29/11/2025
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_psram.h"
#include "common.h"

// Librairies matérielles
#include "lib/expander.h"
#include "lib/LCD.h"
#include "lib/graphics_basic.h"
#include "lib/audio.h"
#include "lib/audio_basic.h"

// Nos modules
#include "Codes/game.h"
#include "Codes/menu.h"
#include "Codes/graphics.h"
#include "Codes/audio.h"

extern void show_intro_screen();
extern void play_intro_music();
extern void intro_image_task(void *pvParameters);
extern void intro_music_task(void *pvParameters);

extern audio_player player;

extern "C" void app_main(void)
{
    printf("\n***\nGamebuino 3 starting ...\n");

    // Initialisation matériel
    lcd_init_pwm();
    lcd_update_pwm(64); // luminosité 50 %
    adc_init();
    expander_init();
    LCD_init();

    // Initialisation audio
    audio_init();
    audio_set_volume(128); // volume 50 %
	
   // attacher la piste de ton au player
    player.add_track(&tone_ball);
	player.add_track(&tone1);
	player.add_track(&tone2);
	player.add_track(&tone3);
	
    // Crée les deux tâches
    xTaskCreate(intro_image_task, "IntroImage", 4096, NULL, 5, NULL);
    xTaskCreate(intro_music_task, "IntroMusic", 4096, NULL, 5, NULL);

    // Attendre 10 secondes (image + musique tournent en parallèle)
    vTaskDelay(10000 / portTICK_PERIOD_MS);

	// Nettoyer l’écran et lancer le menu
    graphics.fillRect(0, 0, 320, 240);
	
    // Menu principal
    gameMode = 0;
    menu_selection = 0;
	draw_menu();
	menu_loop();   // ⚡ reste actif jusqu’à ce qu’un mode soit choisi

	while (true) {
		main_loop(); // ⚡ ensuite le jeu tourne en continu
	}
}