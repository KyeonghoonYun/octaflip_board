#include "board.h"
#include <unistd.h>
#include <stdio.h>

int main(void) {
    struct LedPanelSettings *leds = led_initialize();
    if (!leds) {
        fprintf(stderr, "Failed to initialize LED panel\n");
        return 1;
    }

    // Example 8×8 board: 'R', 'B', '#', '.' 
    char board[8][8] = {
        {'R', '.', '.', '.', '.', '.', '.', 'B'},
        {'.', '#', '.', '.', '.', '.', '#', '.'},
        {'.', '.', 'R', '.', '.', 'B', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '#', '.', '.', '.'},
        {'.', '.', 'B', '.', '.', 'R', '.', '.'},
        {'.', '#', '.', '.', '.', '.', '#', '.'},
        {'B', '.', '.', '.', '.', '.', '.', 'R'}
    };

    // Draw the board and hold for 5 seconds
    draw_board(leds, board);
    sleep(5);

    // Clear and exit
    led_clear(leds);
    led_delete(leds);
    return 0;
}
