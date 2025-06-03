#include "board.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char* argv[]) {
    
    printf("initialize ...\n");
    struct LedPanelSettings *t = led_initialize();
    if (t == NULL) {
        printf("initialize failed\n");
        return 1;
    }
    printf("initialize successed\n");

    printf("draw grid ...\n");
    draw_grid(t);
    printf("draw grid successed\n");

    sleep(3);

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
    int point_red = 2, point_blue = 2;

    printf("draw board ...\n");
    draw_board(t, board);
    printf("draw board successed\n");

    sleep(3);

    printf("draw scores ...\n");
    draw_points(t, point_red, point_blue);
    printf("draw scores successed\n");

    sleep(3);

    printf("clear leds ...\n");
    led_clear(t);
    printf("clear leds successed");

    printf("delete ...\n");
    led_delete(t);
    printf("delete successed\n");
    
    return 0;
}