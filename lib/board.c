#include "board.h"
#include "led-matrix-c.h"

#include <stdlib.h>
#include <string.h>

#define PANEL_SIZE 64
#define CELL_SIZE   8

// Index → 의미
//   0: 'R' → 빨강
//   1: 'B' → 파랑
//   2: '#' → 회색
//   3: '.'(또는 기타) → 검정
//   4: 그리드 → 흰색
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

    // C API 함수 결과를 LedMatrix*로 캐스팅
    leds->matrix = (struct LedMatrix *)led_matrix_create_from_options(&opts, NULL, NULL);
    if (!leds->matrix) {
        free(leds);
        return NULL;
    }

    // 폰트가 반드시 non-NULL 이어야 이후에 led_matrix_get_canvas()가 실패하지 않습니다.
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

    // 최초 화면을 검은색으로 클리어 & swap
    led_canvas_clear((struct LedCanvas *)leds->canvas);
    leds->canvas = (struct LedCanvas *)led_matrix_swap_on_vsync(
        (struct LedMatrix *)leds->matrix,
        (struct LedCanvas *)leds->canvas
    );
    return leds;
}

void draw_board(struct LedPanelSettings *leds, char board[8][8]) {
    // 1) 전체를 검은색으로 클리어
    led_canvas_clear((struct LedCanvas *)leds->canvas);

    // 2) 0 ≤ x,y < 64 범위에서 픽셀을 한 번만 방문
    for (int y = 0; y < PANEL_SIZE; y++) {
        for (int x = 0; x < PANEL_SIZE; x++) {
            int color_index;

            // — 그리드(테두리) 여부 판별 —
            //   x%8==0 or x%8==7 or y%8==0 or y%8==7 → 흰색(인덱스 4)
            if ((x % CELL_SIZE == 0) || (x % CELL_SIZE == CELL_SIZE - 1) ||
                (y % CELL_SIZE == 0) || (y % CELL_SIZE == CELL_SIZE - 1)) {
                color_index = 4; // white for grid
            } else {
                // — 내부(interior) 픽셀은 board[row][col] 값으로 색 결정 —
                int row = y / CELL_SIZE;  // 0..7
                int col = x / CELL_SIZE;  // 0..7
                char c = board[row][col];
                if      (c == 'R') color_index = 0;
                else if (c == 'B') color_index = 1;
                else if (c == '#') color_index = 2;
                else                color_index = 3;
            }

            // 3) 해당 픽셀에 색 칠하기
            led_canvas_set_pixel(
                (struct LedCanvas *)leds->canvas,
                x, y,
                rgb_colors[color_index].r,
                rgb_colors[color_index].g,
                rgb_colors[color_index].b
            );
        }
    }

    // 4) 한 번만 swap_on_vsync 호출하여 전체 화면을 업데이트
    leds->canvas = (struct LedCanvas *)led_matrix_swap_on_vsync(
        (struct LedMatrix *)leds->matrix,
        (struct LedCanvas *)leds->canvas
    );
}

void led_clear(struct LedPanelSettings *leds) {
    // 전체를 검은색으로 클리어하고 바로 swap
    led_canvas_clear((struct LedCanvas *)leds->canvas);
    leds->canvas = (struct LedCanvas *)led_matrix_swap_on_vsync(
        (struct LedMatrix *)leds->matrix,
        (struct LedCanvas *)leds->canvas
    );
}

void led_delete(struct LedPanelSettings *leds) {
    if (!leds) return;
    delete_font((struct LedFont *)leds->font);
    led_matrix_delete((struct LedMatrix *)leds->matrix);
    free(leds);
}
