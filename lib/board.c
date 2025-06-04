#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Define panel size as 64×64
#define LED_PANEL_SIZE 64

// Color lookup table for board symbols (R, B, #, .) and borders (white)
static struct Color rgb_colors[] = {
    {255,   0,   0},   // 'R' → red
    {  0,   0, 255},   // 'B' → blue
    {128, 128, 128},   // '#' → gray
    {  0,   0,   0},   // '.' → black (empty)
    {255, 255, 255}    // borders (white)
};

// Initialize LED matrix: set up options, load font, get canvas
struct LedPanelSettings *led_initialize(void) {
    struct LedPanelSettings *leds = malloc(sizeof(*leds));
    if (!leds) return NULL;
    memset(leds, 0, sizeof(*leds));

    leds->size = LED_PANEL_SIZE;

    // Initialize options: only set rows and cols to 64
    struct RGBLedMatrixOptions opts;
    memset(&opts, 0, sizeof(opts));
    opts.cols = LED_PANEL_SIZE;
    opts.rows = LED_PANEL_SIZE;

    leds->matrix = led_matrix_create_from_options(&opts, NULL, NULL);
    if (!leds->matrix) {
        free(leds);
        return NULL;
    }

    // Load a 5×8 bitmap font (even if we don't render text, font must be non-NULL)
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

    // Clear canvas initially and swap once to apply black frame
    led_canvas_clear(leds->canvas);
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);

    return leds;
}

// Draw 8×8 board on 64×64 matrix: draw grid borders + fill cell interiors
void draw_board(struct LedPanelSettings *leds, char board[8][8]) {
    // 1) Clear entire canvas
    led_canvas_clear(leds->canvas);

    // 2) Draw grid borders: horizontal and vertical lines at multiples of 8
    for (int i = 0; i <= 8; i++) {
        int coord = i * 8;
        // Vertical line at x = coord, y = 0..63
        for (int y = 0; y < LED_PANEL_SIZE; y++) {
            led_canvas_set_pixel(leds->canvas, coord, y,
                                 rgb_colors[4].r,
                                 rgb_colors[4].g,
                                 rgb_colors[4].b);
        }
        // Horizontal line at y = coord, x = 0..63
        for (int x = 0; x < LED_PANEL_SIZE; x++) {
            led_canvas_set_pixel(leds->canvas, x, coord,
                                 rgb_colors[4].r,
                                 rgb_colors[4].g,
                                 rgb_colors[4].b);
        }
    }

    // 3) Fill each cell's interior (6×6 pixels) based on board symbol
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char c = board[row][col];
            int color_idx;
            if      (c == 'R') color_idx = 0;
            else if (c == 'B') color_idx = 1;
            else if (c == '#') color_idx = 2;
            else                color_idx = 3;  // '.' or any other → black

            uint8_t r = rgb_colors[color_idx].r;
            uint8_t g = rgb_colors[color_idx].g;
            uint8_t b = rgb_colors[color_idx].b;

            // Define interior region: (col*8+1 .. col*8+6, row*8+1 .. row*8+6)
            int x_start = col * 8 + 1;
            int y_start = row * 8 + 1;
            for (int y = y_start; y < y_start + 6; y++) {
                for (int x = x_start; x < x_start + 6; x++) {
                    led_canvas_set_pixel(leds->canvas, x, y, r, g, b);
                }
            }
        }
    }

    // 4) Swap once to update display
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// Clear the entire canvas and swap to black
void led_clear(struct LedPanelSettings *leds) {
    led_canvas_clear(leds->canvas);
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// Delete resources: font → matrix → free struct
void led_delete(struct LedPanelSettings *leds) {
    if (!leds) return;
    delete_font(leds->font);
    led_matrix_delete(leds->matrix);
    free(leds);
}
