#ifndef BOARD_H
#define BOARD_H

#include "led-matrix-c.h"

// Structure to hold LED panel configuration
struct LedPanelSettings {
    struct LedCanvas *canvas;
    struct LedMatrix *matrix;
    struct LedFont   *font;
    int               size;  // panel size (e.g., 64)
};

// Initialize the panel: create matrix → load font → get canvas
struct LedPanelSettings *led_initialize(void);

// Given an 8×8 board array, draw grid (borders) + fill each cell on a 64×64 matrix
void draw_board(struct LedPanelSettings *leds, char board[8][8]);

// Clear the entire LED canvas (no flicker)
void led_clear(struct LedPanelSettings *leds);

// Release resources
void led_delete(struct LedPanelSettings *leds);

#endif // BOARD_H
