#include "board.h"
#include "led-matrix-c.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LED_PANEL_SIZE 64

struct LedPanelSettings *leds;

// Color table for symbols and grid lines.
//   index:   0      1        2          3      4
//   symbol: 'R',   'B',    '#',       '.',   grid-line
static struct Color rgb_colors[] = {
    {255,   0,   0},   // 'R' → red
    {  0,   0, 255},   // 'B' → blue
    {128, 128, 128},   // '#' → gray
    {  0,   0,   0},   // '.' (or any other) → black
    {255, 255, 255}    // grid-line → white
};

struct LedPanelSettings *led_initialize(void) {
    if (leds){
        free(leds);
        leds = NULL;
    }
    *led = (struct LedPanelSettings*)malloc(sizeof(*leds));
    if (!leds) return NULL;
    memset(leds, 0, sizeof(*leds));

    leds->size = LED_PANEL_SIZE;

    // Set up a 64×64 matrix
    struct RGBLedMatrixOptions opts;
    memset(&opts, 0, sizeof(opts));
    opts.rows = LED_PANEL_SIZE;
    opts.cols = LED_PANEL_SIZE;

    leds->matrix = led_matrix_create_from_options(&opts, NULL, NULL);
    if (!leds->matrix) {
        free(leds);
        return NULL;
    }

    // Load a basic font so that led_matrix_create doesn't fail.
    leds->font = load_font("rpi-rgb-led-matrix-master/fonts/5x8.bdf");
    if (!leds->font) {
        led_matrix_delete(leds->matrix);
        free(leds);
        return NULL;
    }

    // Grab the canvas pointer
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

void draw_board(char board[8][8]) {
    // 1) Clear to black first
    led_canvas_clear(leds->canvas);

    // 2) Single pass: for each pixel (x,y) in [0..63]
    for (int y = 0; y < LED_PANEL_SIZE; y++) {
        for (int x = 0; x < LED_PANEL_SIZE; x++) {
            int color_idx;

            // If this pixel is on a cell border (grid line), paint white:
            if ((x % 8 == 0) || (y % 8 == 0) || (x % 8 == 7) || (y % 8 == 7)) {
                color_idx = 4;  // white (grid)
            } else {
                // Otherwise: interior. Determine which cell by integer-dividing by 8.
                int row = y / 8;   // 0..7
                int col = x / 8;   // 0..7
                char c = board[row][col];
                if      (c == 'R') color_idx = 0;
                else if (c == 'B') color_idx = 1;
                else if (c == '#') color_idx = 2;
                else                color_idx = 3;  // '.' or any other → black
            }

            // Paint the pixel at (x,y)
            led_canvas_set_pixel(
                leds->canvas, x, y,
                rgb_colors[color_idx].r,
                rgb_colors[color_idx].g,
                rgb_colors[color_idx].b
            );
        }
    }

    // 3) Swap once to update the entire display
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

void led_clear() {
    // Clear all to black and swap immediately
    led_canvas_clear(leds->canvas);
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

void led_delete() {
    if (!leds) return;
    delete_font(leds->font);
    led_matrix_delete(leds->matrix);
    free(leds);
}

#ifdef D
int main(){
    led_initialize();
    if (!leds) {
        fprintf(stderr, "Failed to initialize LED panel\n");
        return 1;
    }
    char board[8][8] = {0};
    for(int i=0;i<8;i++){
        char line[100];
        scanf("%s", line);
        for (int j=0;j<8;j++)
            board[i][j] = line[j];
    }
    
    // Draw the board and hold for 5 seconds
    draw_board(board);
    sleep(5);

    // Clear and exit
    led_clear();
    led_delete();
    return 0;
}
#endif