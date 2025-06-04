#include "board.h"
#include "led-matrix-c.h"

#include <stdlib.h>
#include <string.h>

// === Color table for board symbols and grid ===
//   index: 0    1        2          3      4
//   char: 'R', 'B',      '#',       '.',   grid-line
static struct Color rgb_colors[] = {
    {255,   0,   0},   // 'R' → red
    {  0,   0, 255},   // 'B' → blue
    {128, 128, 128},   // '#' → gray
    {  0,   0,   0},   // '.' (empty) → black
    {255, 255, 255}    // grid-line → white
};

#define LED_PANEL_SIZE 64

struct LedPanelSettings *led_initialize(void) {
    struct LedPanelSettings *leds = (struct LedPanelSettings*)malloc(sizeof(*leds));
    if (!leds) return NULL;
    memset(leds, 0, sizeof(*leds));

    leds->size = LED_PANEL_SIZE;

    // Prepare 64×64 options
    struct RGBLedMatrixOptions opts;
    memset(&opts, 0, sizeof(opts));
    opts.rows = LED_PANEL_SIZE;
    opts.cols = LED_PANEL_SIZE;

    leds->matrix = led_matrix_create_from_options(&opts, NULL, NULL);
    if (!leds->matrix) {
        free(leds);
        return NULL;
    }

    // Load a small font (even if we don't use it, it must be non-NULL)
    leds->font = load_font("rpi-rgb-led-matrix-master/fonts/5x8.bdf");
    if (!leds->font) {
        led_matrix_delete(leds->matrix);
        free(leds);
        return NULL;
    }

    leds->canvas = led_matrix_get_canvas(leds->matrix);
    if (!leds->canvas) {
        delete_font(leds->font);
        led_matrix_delete(leds->matrix);
        free(leds);
        return NULL;
    }

    // Start with a black screen
    led_canvas_clear(leds->canvas);
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
    return leds;
}

void draw_board(struct LedPanelSettings *leds, char board[8][8]) {
    // 1) Clear to black before drawing
    led_canvas_clear(leds->canvas);

    // 2) For each pixel (x,y) in 0..63, decide its color:
    for (int y = 0; y < LED_PANEL_SIZE; y++) {
        for (int x = 0; x < LED_PANEL_SIZE; x++) {
            int color_idx;

            // If on a grid line (border of an 8×8 cell), paint white:
            if ((x % 8 == 0) || (x % 8 == 7) || (y % 8 == 0) || (y % 8 == 7)) {
                color_idx = 4;  // white (grid)
            } else {
                // Otherwise, look up board cell at row = y/8, col = x/8
                int row = y / 8;
                int col = x / 8;
                char c = board[row][col];
                if      (c == 'R') color_idx = 0;
                else if (c == 'B') color_idx = 1;
                else if (c == '#') color_idx = 2;
                else                color_idx = 3;  // '.' or anything else → black
            }

            // Paint the pixel
            led_canvas_set_pixel(
                leds->canvas, x, y,
                rgb_colors[color_idx].r,
                rgb_colors[color_idx].g,
                rgb_colors[color_idx].b
            );
        }
    }

    // 3) Finally, swap once to update the entire 64×64 display
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

void led_clear(struct LedPanelSettings *leds) {
    // Clear to black and swap immediately
    led_canvas_clear(leds->canvas);
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

void led_delete(struct LedPanelSettings *leds) {
    if (!leds) return;
    delete_font(leds->font);
    led_matrix_delete(leds->matrix);
    free(leds);
}
