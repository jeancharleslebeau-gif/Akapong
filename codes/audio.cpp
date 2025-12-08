#include "audio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Le tableau melody[] est déjà défini dans le code

audio_player player;
audio_track_tone tone1, tone2, tone3;

audio_track_tone tone_ball;

// Tâche qui joue la musique
void intro_music_task(void *pvParameters) {
    for (int i = 0; i < melody_intro_len; i++) {
        // lancer les trois voix de la note
        if (melody[i].freq1 > 0) {
            tone1.play_tone(melody[i].freq1, melody[i].dur1);
            player.add_track(&tone1);
        }
        if (melody[i].freq2 > 0) {
            tone2.play_tone(melody[i].freq2, melody[i].dur2);
            player.add_track(&tone2);
        }
        if (melody[i].freq3 > 0) {
            tone3.play_tone(melody[i].freq3, melody[i].dur3);
            player.add_track(&tone3);
        }

        // attendre la durée la plus longue
        uint16_t wait = melody[i].dur1;
        if (melody[i].dur2 > wait) wait = melody[i].dur2;
        if (melody[i].dur3 > wait) wait = melody[i].dur3;

        // pendant l’attente, alimenter le player
        uint32_t ticks = pdMS_TO_TICKS(wait);
        while (ticks--) {
            player.pool();       // génère et pousse les buffers
            vTaskDelay(1);       // yield FreeRTOS
        }
    }

    vTaskDelete(NULL); // termine la tâche
}

void play_intro_music() {
    xTaskCreatePinnedToCore(
        intro_music_task,
        "intro_music",
        4096,   // stack un peu plus large
        NULL,
        1,
        NULL,
        0
    );
}