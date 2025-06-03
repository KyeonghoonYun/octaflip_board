#ifndef BOARD_H
#define BOARD_H

#include "led-matrix-c.h"


#ifdef __cplusplus
extern "C" {
#endif

struct LedPanelSettings {
    int size;
    struct RGBLedMatrix *matrix;
    struct LedCanvas *canvas;
    struct LedFont *font;
};

struct LedPanelSettings *led_initialize(void);

void draw_grid(struct LedPanelSettings *leds);

void draw_board(struct LedPanelSettings *leds, char board[8][8]);

void draw_points(struct LedPanelSettings *leds, int point_red, int point_blue);

void led_clear(struct LedPanelSettings *leds);

void led_delete(struct LedPanelSettings *leds);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif