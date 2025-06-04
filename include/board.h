#ifndef BOARD_H
#define BOARD_H

#include "led-matrix-c.h"

#ifdef __cplusplus
extern "C" {
#endif

// Holds the RGB LED matrix, its canvas, and an optional font handle.
struct LedPanelSettings {
    struct LedCanvas *canvas;
    struct LedMatrix *matrix;
    struct LedFont   *font;
    int               size;  // panel size, e.g. 64
};

// Initialize a 64×64 RGB LED panel. Returns NULL on failure.
struct LedPanelSettings *led_initialize(void);

// Draw an 8×8 logical board onto the 64×64 panel in a single pass.
//   - Any pixel where (x%8==0 || x%8==7 || y%8==0 || y%8==7) is painted white (grid line).
//   - Otherwise, compute row=y/8 and col=x/8, look up board[row][col]:
//       'R' → red, 'B' → blue, '#' → gray, anything else → black.
// After filling all 64×64 pixels, call swap_on_vsync once to update.
void draw_board(struct LedPanelSettings *leds, char board[8][8]);

// Clear the entire 64×64 LED panel to black (single clear + swap).
void led_clear(struct LedPanelSettings *leds);

// Free all resources (font, matrix, struct).
void led_delete(struct LedPanelSettings *leds);

#ifdef __cplusplus
}
#endif

#endif // BOARD_H
