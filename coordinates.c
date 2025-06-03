#include "board.h"
#include "led-matrix-c.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main() {
    struct LedPanelSettings *leds = led_initialize();

    // y-axis with color red
    for (int count = 0; count < 6; count++) {
        int x = 0;
        int y = 0 + 10 * count;
        led_canvas_set_pixel(leds->canvas, x, y, 255 - (255/6)*count, 0, 0);
        leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
        usleep(1000);
    }

    // x-axis with color blue
    for (int count = 0; count < 6; count++) {
        int x = 0 + 10 * count;
        int y = 0;
        led_canvas_set_pixel(leds->canvas, x, y, 0, 0, 255 - (255/6)*count);
        leds->canvas = led_matrix_swap_on_vsync(leds->matrix, leds->canvas);
        usleep(1000);
    }

    // green at center
    led_canvas_set_pixel(leds->canvas, 32, 32, 0, 255, 0);

    sleep(5);

    led_clear(leds);

    led_delete(leds);

    return 0;
}