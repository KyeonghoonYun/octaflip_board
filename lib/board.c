// board.c

#include "board.h"
#include "led-matrix-c.h"

#include <stdlib.h>
#include <string.h>

#define PANEL_SIZE 64
#define CELL_SIZE   8

// Color table indices:
//   0: 'R' → red
//   1: 'B' → blue
//   2: '#' → gray
//   3: '.' or other → black
//   4: grid line → white
static struct Color rgb_colors[] = {
    {255,   0,   0},  // index 0
    {  0,   0, 255},  // index 1
    {128, 128, 128},  // index 2
    {  0,   0,   0},  // index 3
    {255, 255, 255}   // index 4
};

struct LedPanelSettings *led_initialize(void) {
    struct LedPanelSettings *leds = (struct LedPanelSettings *)malloc(sizeof(*leds));
    if (!leds) return NULL;
    memset(leds, 0, sizeof(*leds));
    leds->size = PANEL_SIZE;

    struct RGBLedMatrixOptions opts;
    memset(&opts, 0, sizeof(opts));
    opts.rows = PANEL_SIZE;
    opts.cols = PANEL_SIZE;

    leds->matrix = (struct LedMatrix *)led_matrix_create_from_options(&opts, NULL, NULL);
    if (!leds->matrix) {
        free(leds);
        return NULL;
    }

    leds->font = load_font("rpi-rgb-led-matrix-master/fonts/5x8.bdf");
    if (!leds->font) {
        led_matrix_delete((struct LedMatrix *)leds->matrix);
        free(leds);
        return NULL;
    }

    leds->canvas = (struct LedCanvas *)led_matrix_get_canvas((struct LedMatrix *)leds->matrix);
    if (!leds->canvas) {
        delete_font((struct LedFont *)leds->font);
        led_matrix_delete((struct LedMatrix *)leds->matrix);
        free(leds);
        return NULL;
    }

    led_canvas_clear((struct LedCanvas *)leds->canvas);
    leds->canvas = (struct LedCanvas *)led_matrix_swap_on_vsync((struct LedMatrix *)leds->matrix,
                                                               (struct LedCanvas *)leds->canvas);
    return leds;
}

void draw_board(struct LedPanelSettings *leds, char board[8][8]) {
    led_canvas_clear((struct LedCanvas *)leds->canvas);

    for (int y = 0; y < PANEL_SIZE; y++) {
        for (int x = 0; x < PANEL_SIZE; x++) {
            int color_index;

            if ((x % CELL_SIZE == 0) || (x % CELL_SIZE == CELL_SIZE - 1) ||
                (y % CELL_SIZE == 0) || (y % CELL_SIZE == CELL_SIZE - 1)) {
                color_index = 4;  // grid line
            } else {
                int row = y / CELL_SIZE;
                int col = x / CELL_SIZE;
                char c = board[row][col];
                if      (c == 'R') color_index = 0;
                else if (c == 'B') color_index = 1;
                else if (c == '#') color_index = 2;
                else                color_index = 3;
            }

            led_canvas_set_pixel((struct LedCanvas *)leds->canvas,
                                 x, y,
                                 rgb_colors[color_index].r,
                                 rgb_colors[color_index].g,
                                 rgb_colors[color_index].b);
        }
    }

    leds->canvas = (struct LedCanvas *)led_matrix_swap_on_vsync((struct LedMatrix *)leds->matrix,
                                                                (struct LedCanvas *)leds->canvas);
}

void led_clear(struct LedPanelSettings *leds) {
    led_canvas_clear((struct LedCanvas *)leds->canvas);
    leds->canvas = (struct LedCanvas *)led_matrix_swap_on_vsync((struct LedMatrix *)leds->matrix,
                                                                (struct LedCanvas *)leds->canvas);
}

void led_delete(struct LedPanelSettings *leds) {
    if (!leds) return;
    delete_font((struct LedFont *)leds->font);
    led_matrix_delete((struct LedMatrix *)leds->matrix);
    free(leds);
}
