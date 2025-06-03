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
        printf("[Debug] draw_grid(); coordinate (%d, %d) with color RGB (%d, %d, %d)\n", 
            y0, x0, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
        // led_canvas_set_pixels(leds->canvas, x0, y0, 33, 1, &(rgb_colors[4]));
        for (int dy = 0; dy < 33; dy++) {
            led_canvas_set_pixel(leds->canvas, x0, y0+dy, rgb_colors[4].r, rgb_colors[4].g, rgb_colors[4].b);
        }
    }
    for (int col = 0; col < 8; col++) {
        int x0 = 3 + 4 * col;
        int y0 = 3;
        int c = 4;
        printf("[Debug] draw_grid(); coordinate (%d, %d) with color RGB (%d, %d, %d)\n", 
            y0, x0, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
        // led_canvas_set_pixels(leds->canvas, x0, y0, 1, 33, &(rgb_colors[4]));
        for (int dx = 0; dx < 33; dx++) {
            led_canvas_set_pixel(leds->canvas, x0+dx, y0, rgb_colors[4].r, rgb_colors[4].g, rgb_colors[4].b);
        }
    }
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// draw board state (R, B, #)
void draw_board(struct LedPanelSettings *leds, char board[8][8]) {
    for (int row = 0; row < 8; row++) {
        int y0 = 4 + 4 * row;
        for (int col = 0; col < 8; col++) {
            int x0 = 4 + 4 * col;

            if (board[row][col] == 'R') {
                int c = 0;
                printf("[Debug] draw_board(); coordinate (%d, %d) with color RGB (%d, %d, %d)\n", 
                    y0, x0, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                // led_canvas_set_pixels(leds->canvas, x0, y0, 3, 3, &(rgb_colors[0]));
                for (int dy = 0; dy < 3; dy++) {
                    for (int dx = 0; dx < 3; dx++) {
                        led_canvas_set_pixel(leds->canvas, x0+dx, y0+dy, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                    }
                }
            }
            else if (board[row][col] == 'B') {
                int c = 1;
                printf("[Debug] draw_board(); coordinate (%d, %d) with color RGB (%d, %d, %d)\n", 
                    y0, x0, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                // led_canvas_set_pixels(leds->canvas, x0, y0, 3, 3, &(rgb_colors[1]));
                for (int dy = 0; dy < 3; dy++) {
                    for (int dx = 0; dx < 3; dx++) {
                        led_canvas_set_pixel(leds->canvas, x0+dx, y0+dy, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                    }
                }
            }
            else if (board[row][col] == '#') {
                int c = 2;
                printf("[Debug] draw_board(); coordinate (%d, %d) with color RGB (%d, %d, %d)\n", 
                    y0, x0, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                // led_canvas_set_pixels(leds->canvas, x0, y0, 3, 3, &(rgb_colors[2]));
                for (int dy = 0; dy < 3; dy++) {
                    for (int dx = 0; dx < 3; dx++) {
                        led_canvas_set_pixel(leds->canvas, x0+dx, y0+dy, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                    }
                }
            }
            else if (board[row][col] == '.') {
                int c = 3;
                printf("[Debug] draw_board(); coordinate (%d, %d) with color RGB (%d, %d, %d)\n", 
                    y0, x0, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                // led_canvas_set_pixels(leds->canvas, x0, y0, 3, 3, &(rgb_colors[3]));
                for (int dy = 0; dy < 3; dy++) {
                    for (int dx = 0; dx < 3; dx++) {
                        led_canvas_set_pixel(leds->canvas, x0+dx, y0+dy, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                    }
                }
            }
            else {
                int c = 4;
                printf("[Debug] draw_board(); coordinate (%d, %d) with color RGB (%d, %d, %d)\n", 
                    y0, x0, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                // led_canvas_set_pixels(leds->canvas, x0, y0, 3, 3, &(rgb_colors[4]));
                for (int dy = 0; dy < 3; dy++) {
                    for (int dx = 0; dx < 3; dx++) {
                        led_canvas_set_pixel(leds->canvas, x0+dx, y0+dy, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
                    }
                }
            }
        }
    }
    leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
}

// draw the points of Red/Blue players
void draw_points(struct LedPanelSettings *leds, int point_red, int point_blue) {
    char buf[20];
    sprintf(buf, "%s%02d   %s%02d", "R:", point_red, "B:", point_blue);
    buf[strlen(buf)] = '\0';
    // led_canvas_set_pixels(leds->canvas, 3, 39, 60, 8, &(rgb_colors[3]));
    int c = 3;
    for (int dy = 0; dy < 60; dy++) {
        for (int dx = 0; dx < 8; dx++) {
            led_canvas_set_pixel(leds->canvas, 3+dx, 3+dy, rgb_colors[c].r, rgb_colors[c].g, rgb_colors[c].b);
        }
    }
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

