#include "highscores.h"
#include "core/graphics.h"
#include "core/input.h"
#include "lib/LCD.h"
#include "lib/graphics_basic.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "core/audio.h"	

void highscores_init() {
    FILE* f = fopen("/sdcard/AKAsseBrick.sco", "rb");
    if (!f) {
        // Crée un fichier vide (0 entrée)
        f = fopen("/sdcard/AKAsseB.sco", "wb");
        if (f) fclose(f);
    } else {
        fclose(f);
    }
}

std::vector<HighscoreEntry> highscores_load() {
    std::vector<HighscoreEntry> scores;
    FILE* f = fopen("/sdcard/AKAsseB.sco", "rb");
    if (!f) return scores;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    size_t count = (size >= 0) ? (size_t)(size / sizeof(HighscoreEntry)) : 0;
    scores.resize(count);

    if (count > 0) {
        fread(scores.data(), sizeof(HighscoreEntry), count, f);
    }
    fclose(f);
    return scores;
}

void highscores_submit(int score) {
    auto scores = highscores_load();

    std::string name = highscores_input_name();

    HighscoreEntry entry;
    strncpy(entry.name, name.c_str(), 8);
    entry.name[8] = '\0';
    entry.score = score;

    scores.push_back(entry);

    std::sort(scores.begin(), scores.end(),
              [](const HighscoreEntry& a, const HighscoreEntry& b){
                  return a.score > b.score;
              });

    if (scores.size() > MAX_SCORES) scores.resize(MAX_SCORES);

    FILE* f = fopen("/sdcard/AKAsseB.sco", "wb");
    if (f) {
        fwrite(scores.data(), sizeof(HighscoreEntry), scores.size(), f);
        fclose(f);
    }
}

void highscores_show() {
    auto scores = highscores_load();
    gfx_clear(color_black);
    gfx_text(80, 10, "=== Highscores ===", color_yellow);

    int y = 50;
    int rank = 1;
    for (const auto& e : scores) {
        if (rank > MAX_SCORES) break;
        if (e.name[0] == '\0') continue; // skip entrées vides

        char buf[64];
        snprintf(buf, sizeof(buf), "%2d. %-8s %6d", rank, e.name, e.score);
        gfx_text(20, y, buf, color_white);
        y += 20;
        rank++;
    }
	
	gfx_text(20, 170, "Appuyez sur <B> pour revenir au menu.", color_yellow);
	
    gfx_flush();
}


std::string highscores_input_name() {
    std::string name;
    char currentChar = 'A';
    const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int index = 0;

    bool b_pressed = false;

    while (true) {
        Keys k;
        input_poll(k);

        if (k.left) {
            if (--index < 0) index = alphabet.size()-1;
            currentChar = alphabet[index];
            snd_keypress.play_tone(880.0f, 50); // feedback navigation
        }
        if (k.right) {
            if (++index >= alphabet.size()) index = 0;
            currentChar = alphabet[index];
            snd_keypress.play_tone(880.0f, 50);
        }

        if (k.A && name.size() < 8) {
            name.push_back(currentChar);
            index = 0;
            currentChar = 'A';
            snd_keypress.play_tone(880.0f, 50); // feedback ajout
        }

        if (k.C && !name.empty()) {
            name.pop_back();
            snd_delete.play_tone(110.0f, 80); // feedback suppression
        }

        // Validation par B : press → note l’état, release → valide
        if (k.B && !name.empty()) {
            b_pressed = true;
        }
        if (!k.B && b_pressed) {
            snd_keypress.play_tone(660.0f, 120); // feedback validation
            break;
        }

        gfx_clear(color_black);
        gfx_text(20, 100, ("Pseudo: " + name + currentChar).c_str(), color_white);
        gfx_text(20, 120, "A=Valider char, C=Effacer, B=OK", color_yellow);
        gfx_flush();

        player.pool();   // 🔊 indispensable pour entendre les sons
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return name;
}
