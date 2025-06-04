#include "board.h"
#include "led-matrix-c.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LED_PANEL_SIZE 64


static struct Color rgb_colors[] = {
    {255, 0, 0},        // red - 'R'
    {0, 0, 255},        // blue - 'B'
    {128, 128, 128},    // gray - '#'
    {0, 0, 0},          // black - '.'
    {255, 255, 255}     // white - grid, point
};



struct LedPanelSettings *led_initialize(void) {
    struct LedPanelSettings* leds = (struct LedPanelSettings*) malloc(sizeof(*leds));
    if (!leds) return NULL;
    memset(leds, 0, sizeof(*leds));

    leds->size = LED_PANEL_SIZE;

    struct RGBLedMatrixOptions opts;
    memset(&opts, 0, sizeof(opts));
    opts.cols = LED_PANEL_SIZE;
    opts.rows = LED_PANEL_SIZE;

    leds->matrix = led_matrix_create_from_options(&opts, NULL, NULL);

    if (!leds->matrix) {
        free(leds);
        return NULL;
    }
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
    led_canvas_clear(leds->canvas);
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);

    return leds;
}

// draw grid
void draw_grid(struct LedPanelSettings *leds) {
    // Clear entire canvas before drawing grid
    led_canvas_clear(leds->canvas);

    // Draw vertical lines at x = 0, 8, 16, ..., 56
    for (int i = 0; i < 8; i++) {
        int x = i * (LED_PANEL_SIZE / 8);
        for (int y = 0; y < LED_PANEL_SIZE; y++) {
            led_canvas_set_pixel(leds->canvas, x, y,
                                 rgb_colors[4].r,
                                 rgb_colors[4].g,
                                 rgb_colors[4].b);
        }
    }
    // Draw rightmost border at x = 63
    for (int y = 0; y < LED_PANEL_SIZE; y++) {
        led_canvas_set_pixel(leds->canvas, LED_PANEL_SIZE - 1, y,
                             rgb_colors[4].r,
                             rgb_colors[4].g,
                             rgb_colors[4].b);
    }

    // Draw horizontal lines at y = 0, 8, 16, ..., 56
    for (int i = 0; i < 8; i++) {
        int y = i * (LED_PANEL_SIZE / 8);
        for (int x = 0; x < LED_PANEL_SIZE; x++) {
            led_canvas_set_pixel(leds->canvas, x, y,
                                 rgb_colors[4].r,
                                 rgb_colors[4].g,
                                 rgb_colors[4].b);
        }
    }
    // Draw bottom border at y = 63
    for (int x = 0; x < LED_PANEL_SIZE; x++) {
        led_canvas_set_pixel(leds->canvas, x, LED_PANEL_SIZE - 1,
                             rgb_colors[4].r,
                             rgb_colors[4].g,
                             rgb_colors[4].b);
    }

    // Swap once to update display
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// draw board state (R, B, #)
void draw_board(struct LedPanelSettings *leds, char board[8][8]) {
    // First draw the grid borders
    draw_grid(leds);

    // Fill each cell's interior (6×6 pixels inside the 8×8 border)
    int cell_size = LED_PANEL_SIZE / 8; // 8
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

            int x_start = col * cell_size + 1;
            int y_start = row * cell_size + 1;
            // Interior spans from +1 to +(cell_size-2) ⇒ 1..6
            for (int y = y_start; y < y_start + (cell_size - 2); y++) {
                for (int x = x_start; x < x_start + (cell_size - 2); x++) {
                    led_canvas_set_pixel(leds->canvas, x, y, r, g, b);
                }
            }
        }
    }

    // Swap once to update display
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// draw the points of Red/Blue players
void draw_points(struct LedPanelSettings *leds, int point_red, int point_blue) {
    char buf[20];
    sprintf(buf, "%s%02d   %s%02d", "R:", point_red, "B:", point_blue);
    buf[strlen(buf)] = '\0';
    int c = 3;
    
    print_debug_msg("draw_points[clean]", 3, 39, 8, 60, c);
    draw_pixels(leds, 3, 39, 8, 60, c);
    
    print_debug_msg("draw_points", 3, 39, 8, 60, c);
    draw_text(leds->canvas, leds->font, 3, 39,
                255, 255, 255,
                buf, 1);
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// clear
void led_clear(struct LedPanelSettings *leds) {
    led_canvas_clear(leds->canvas);
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// free() memory
void led_delete(struct LedPanelSettings *leds) {
    delete_font(leds->font);
    led_matrix_delete(leds->matrix);
    free(leds);
}



static inline void print_debug_msg(const char* func_name, int x, int y, int height, int width, int color_num) {
    printf("[Debug] %s(); coordinate (y,x):(%d, %d), width x height:%dx%d; color RGB (%d, %d, %d)\n", 
        func_name, y, x, width, height,
        rgb_colors[color_num].r, rgb_colors[color_num].g, rgb_colors[color_num].b);
}

static inline void draw_pixels(struct LedPanelSettings *leds, int x0, int y0, int height, int width, int color_num) {
    for (int dy = 0; dy < width; dy++) {
        for (int dx = 0; dx < height; dx++) {
            led_canvas_set_pixel(leds->canvas, x0+dx, y0+dy, rgb_colors[color_num].r, rgb_colors[color_num].g, rgb_colors[color_num].b);
            leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
            usleep(100);
        }
    }
}
