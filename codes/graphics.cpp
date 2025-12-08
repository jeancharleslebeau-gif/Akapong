#include "config.h"
#include "graphics.h"
#include "game.h" 
#include "lib/LCD.h"
#include "lib/graphics_basic.h"
#include "image_intro.h"
#include <cstddef>               // pour NULL
#include "freertos/FreeRTOS.h"   // pour portTICK_PERIOD_MS
#include "freertos/task.h"       // pour vTaskDelay, vTaskDelete

graphics_basic graphics;

// Taille selon ta résolution (ex: 320x240)
uint16_t framebuffer[SCREEN_W * SCREEN_H];

void show_intro_screen()
{
    for (int y = 0; y < SCREEN_H; y++) {
        for (int x = 0; x < SCREEN_W; x++) {
            graphics.setColor(image_intro[y * SCREEN_W + x]);
            graphics.drawPixel(x, y);
        }
    }

    lcd_refresh();
    while (!lcd_refresh_completed());

    vTaskDelay(10000 / portTICK_PERIOD_MS); // attente 10 secondes
}


// Tâche pour l’image
void intro_image_task(void *pvParameters)
{
    show_intro_screen();
    vTaskDelete(NULL); // termine la tâche
}


void draw_game() {
    graphics.setColor(color_blue);
    graphics.fillRect(0, 0, SCREEN_W, SCREEN_H);
    graphics.setColor(color_orange);
    graphics.fillCircle(ball.x, ball.y, ball.size);
    graphics.setColor(color_green);
    graphics.fillRect(leftPaddle.x, leftPaddle.y, leftPaddle.w, leftPaddle.h);
    graphics.fillRect(rightPaddle.x, rightPaddle.y, rightPaddle.w, rightPaddle.h);
    lcd_move_cursor(40, 10);
    lcd_printf("P1: %d", scoreP1);
    lcd_move_cursor(240, 10);
    lcd_printf("P2: %d", scoreP2);
    lcd_refresh();
    while (!lcd_refresh_completed());
}
