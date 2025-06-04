#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    // 1) Initialize LED matrix
    printf("initialize ...\n");
    struct LedPanelSettings *leds = led_initialize();
    if (leds == NULL) {
        fprintf(stderr, "initialize failed\n");
        return 1;
    }
    printf("initialize succeeded\n");

    // 2) Example 8×8 board (R, B, ., #)
    char board[8][8] = {
        {'R', '.', '.', '.', '.', '.', '.', 'B'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'B', '.', '.', '.', '.', '.', '.', 'R'}
    };

    // 3) Draw the board on the 64×64 panel
    printf("draw board ...\n");
    draw_board(leds, board);
    printf("draw board succeeded\n");

    // 4) Wait for 3 seconds
    sleep(3);

    // 5) Clear the LED panel
    printf("clear leds ...\n");
    led_clear(leds);
    printf("clear leds succeeded\n");

    // 6) Delete resources
    printf("delete ...\n");
    led_delete(leds);
    printf("delete succeeded\n");

    return 0;
}
