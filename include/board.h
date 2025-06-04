// board.h

#ifndef BOARD_H
#define BOARD_H

#include "led-matrix-c.h"

#ifdef __cplusplus
extern "C" {
#endif

// Structure holding the LED matrix, its canvas, and a loaded font.
struct LedPanelSettings {
    struct LedCanvas *canvas;
    struct LedMatrix *matrix;
    struct LedFont   *font;
    int               size;  // panel size (e.g., 64)
};

// Initialize a 64×64 RGB LED panel. Returns NULL on failure.
struct LedPanelSettings *led_initialize(void);

// Draw an 8×8 logical board onto the 64×64 panel in one pass.
//   - Pixels where (x%8==0 || x%8==7 || y%8==0 || y%8==7) are painted white (grid lines).
//   - Other pixels use board[y/8][x/8] to pick color:
//       'R' → red, 'B' → blue, '#' → gray, else → black.
// One call to swap_on_vsync() updates the display.
void draw_board(struct LedPanelSettings *leds, char board[8][8]);

// Clear the entire 64×64 panel to black, then swap.
void led_clear(struct LedPanelSettings *leds);

// Release all resources (font, matrix, struct).
void led_delete(struct LedPanelSettings *leds);

#ifdef __cplusplus
}
#endif

#endif // BOARD_H
