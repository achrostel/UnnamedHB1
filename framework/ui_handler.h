#ifndef UI_HANDLER_H
#define UI_HANDLER_H

#include "ui_parts.h"

void init_mainMenu();
void init_pauseMenu();
void init_gameUI();

void handle_mainMenu(uint16_t buttons);
void handle_pauseMenu(uint16_t buttons);
void handle_gameUI(int curr_pellets, uint8_t max_pellets, uint32_t ammo, uint16_t score);

void enable_mainMenu();
void enable_settings();
void enable_gameUI();

void draw_bg();
#endif // UI_HANDLER_H