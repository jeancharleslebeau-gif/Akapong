#pragma once
#include <vector>
#include <string>
#include "core/persist.h"

struct HighscoreEntry {
    char name[9];   // 8 caractères + '\0'
    int score;
};

constexpr int MAX_SCORES = 6;

void highscores_init();
std::vector<HighscoreEntry> highscores_load();
std::string highscores_input_name();
void highscores_submit(int score);
void highscores_show();