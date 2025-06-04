#ifndef BOARD_H
#define BOARD_H

#include "led-matrix-c.h"

#ifdef __cplusplus
extern "C" {
#endif

// Holds the RGB LED matrix, its canvas, and an optional font handle.
struct LedPanelSettings {
    struct LedCanvas *canvas;  // opaque C 타입
    struct LedMatrix *matrix;  // opaque C 타입
    struct LedFont   *font;    // opaque C 타입
    int               size;    // panel size (e.g. 64)
};

// Initialize a 64×64 RGB LED panel. Returns NULL on failure.
struct LedPanelSettings *led_initialize(void);

// Draw an 8×8 logical board onto the 64×64 panel in a single pass:
//  - Any pixel where (x%8==0 || x%8==7 || y%8==0 || y%8==7) → paint white (grid line).
//  - Otherwise: row=y/8, col=x/8 → board[row][col]
//       'R' → red, 'B' → blue, '#' → gray, 그 외 → black.
// 마지막에 swap_on_vsync 한 번만 호출합니다.
void draw_board(struct LedPanelSettings *leds, char board[8][8]);

// Clear the entire 64×64 LED panel to black (single clear + swap).
void led_clear(struct LedPanelSettings *leds);

// Free all resources (font, matrix, struct).
void led_delete(struct LedPanelSettings *leds);

#ifdef __cplusplus
}
#endif

#endif // BOARD_H
