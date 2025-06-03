#include "board.h"
#include "led-matrix-c.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    leds->matrix = led_matrix_create(LED_PANEL_SIZE, 1, 1);
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
    for (int row = 0; row <= 8; row++) {
        int x0 = 3;
        int y0 = 3 + 4 * row;
        int c = 4;
        print_debug_msg("draw_grid", x0, y0, 1, 33, c);
        draw_pixels(leds->canvas, x0, y0, 1, 33, c);
    }
    for (int col = 0; col < 8; col++) {
        int x0 = 3 + 4 * col;
        int y0 = 3;
        int c = 4;
        print_debug_msg("draw_grid", x0, y0, 33, 1, c);
        draw_pixels(leds->canvas, x0, y0, 33, 1, c);
    }
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// draw board state (R, B, #)
void draw_board(struct LedPanelSettings *leds, char board[8][8]) {
    for (int row = 0; row < 8; row++) {
        int y0 = 4 + 4 * row;
        for (int col = 0; col < 8; col++) {
            int x0 = 4 + 4 * col;

            int c = 4;
            if (board[row][col] == 'R')         c = 0;
            else if (board[row][col] == 'B')    c = 1;
            else if (board[row][col] == '#')    c = 2;
            else if (board[row][col] == '.')    c = 3;
            else                                c = 4;
            print_debug_msg("draw_board", x0, y0, 3, 3, c);
            draw_pixels(leds->canvas, x0, y0, 3, 3, c);
        }
    }
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// draw the points of Red/Blue players
void draw_points(struct LedPanelSettings *leds, int point_red, int point_blue) {
    char buf[20];
    sprintf(buf, "%s%02d   %s%02d", "R:", point_red, "B:", point_blue);
    buf[strlen(buf)] = '\0';
    int c = 3;
    
    print_debug_msg("draw_points[clean]", 3, 39, 8, 60, c);
    draw_pixels(leds->canvas, 3, 39, 8, 60, c);
    
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



static inline void print_debug_msg(const char** func_name, int x, int y, int height, int width, int color_num) {
    printf("[Debug] %s(); coordinate (y,x):(%d, %d), width x height:%dx%d; color RGB (%d, %d, %d)\n", 
        func_name, y, x, width, height,
        rgb_colors[color_num].r, rgb_colors[color_num].g, rgb_colors[color_num].b);
}

static inline void draw_pixels(struct LedCanvas* canvas, int x0, int y0, int height, int width, int color_num) {
    for (int dy = 0; dy < width; dy++) {
        for (int dx = 0; dx < height; dx++) {
            led_canvas_set_pixel(canvas, x0+dx, y0+dy, rgb_colors[color_num].r, rgb_colors[color_num].g, rgb_colors[color_num].b);
        }
    }
}