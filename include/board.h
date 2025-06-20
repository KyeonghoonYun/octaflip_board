#ifndef BOARD_H
#define BOARD_H

#include "led-matrix-c.h"

#ifdef __cplusplus
extern "C" {
#endif

// Holds the RGB LED matrix, its canvas, and a font handle.
struct LedPanelSettings {
    struct LedCanvas *canvas;
    struct RGBLedMatrix *matrix;
    struct LedFont   *font;
    int               size;  // panel size (e.g., 64)
};

// Initialize a 64×64 RGB LED panel. Returns NULL on failure.
struct LedPanelSettings *led_initialize(void);

// Draw an 8×8 logical board onto the 64×64 panel in a single pass.
//   – Any pixel where (x%8 == 0) || (x%8 == 7) || (y%8 == 0) || (y%8 == 7)
//     should be painted white (grid line).
//   – Otherwise, compute row = y/8, col = x/8, and pick color from board[row][col]:
//       'R' → red, 'B' → blue, '#' → gray, else → black.
// Finally, call swap_on_vsync(...) once to update the display.
void draw_board(char board[8][8]);

// Clear the entire 64×64 panel to black (one clear + one swap).
void led_clear();

// Free all resources (font, matrix, struct).
void led_delete();

#ifdef __cplusplus
}
#endif

#endif // BOARD_H
