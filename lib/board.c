#include "board.h"
#include "led-matrix-c.h"  // C API

#include <stdlib.h>
#include <string.h>

#define LED_PANEL_SIZE 64

// Color table for symbols and grid lines.
//   index:   0    1      2      3     4
//   symbol: 'R', 'B',   '#',   '.',  grid
static struct Color rgb_colors[] = {
    {255,   0,   0},   // 'R' → red
    {  0,   0, 255},   // 'B' → blue
    {128, 128, 128},   // '#' → gray
    {  0,   0,   0},   // '.' (or 기타) → black
    {255, 255, 255}    // grid-line → white
};

//-----------------------------------------------------------------------------
// led_initialize()
//   - 64×64 RGB LED 매트릭스를 생성하고, 내부 Font도 로드하며
//   - 초기 화면을 검은색으로 클리어한 뒤 swap_on_vsync 합니다.
//   - 성공 시 malloc한 struct LedPanelSettings*를 반환, 실패 시 NULL 반환.
//-----------------------------------------------------------------------------
struct LedPanelSettings *led_initialize(void) {
    struct LedPanelSettings *leds = (struct LedPanelSettings *)malloc(sizeof(*leds));
    if (!leds) return NULL;
    memset(leds, 0, sizeof(*leds));

    leds->size = LED_PANEL_SIZE;

    // C API의 옵션 구조체
    struct RGBLedMatrixOptions opts;
    memset(&opts, 0, sizeof(opts));
    opts.rows = LED_PANEL_SIZE;
    opts.cols = LED_PANEL_SIZE;

    // led_matrix_create_from_options의 원형은 C API 상 “LedMatrix *led_matrix_create_from_options(...)”
    // 실제 내부 구현은 C++ 클래스(RGBLedMatrix)를 반환하지만, 
    // 컴파일 시 C++ 쪽 class가 불완전 타입이기 때문에 오류가 뜹니다.
    // → 명시적으로 (struct LedMatrix *)로 캐스팅해 줍니다.
    leds->matrix = (struct LedMatrix *)led_matrix_create_from_options(&opts, NULL, NULL);
    if (!leds->matrix) {
        free(leds);
        return NULL;
    }

    // Font는 반드시 non-NULL이 되어야 다음 단계에서 에러가 나지 않습니다.
    leds->font = load_font("rpi-rgb-led-matrix-master/fonts/5x8.bdf");
    if (!leds->font) {
        // matrix만 삭제
        led_matrix_delete((struct LedMatrix *)leds->matrix);
        free(leds);
        return NULL;
    }

    // canvas 얻기 (리턴 타입도 opaque C 타입인 LedCanvas *)
    leds->canvas = (struct LedCanvas *)led_matrix_get_canvas((struct LedMatrix *)leds->matrix);
    if (!leds->canvas) {
        delete_font((struct LedFont *)leds->font);
        led_matrix_delete((struct LedMatrix *)leds->matrix);
        free(leds);
        return NULL;
    }

    // 맨 처음 검은색으로 클리어 + swap
    led_canvas_clear((struct LedCanvas *)leds->canvas);
    leds->canvas = (struct LedCanvas *)led_matrix_swap_on_vsync((struct LedMatrix *)leds->matrix, (struct LedCanvas *)leds->canvas);
    return leds;
}

//-----------------------------------------------------------------------------
// draw_board()
//   - (0 ≤ x,y < 64) 한 번씩만 루프를 돌며 각 픽셀을 결정합니다.
//   - (x % 8 == 0 || x % 8 == 7 || y % 8 == 0 || y % 8 == 7)인 경우 → grid-line (흰색).
//   - 그 외 → interior. row=y/8, col=x/8로 board[row][col] 확인, 색상(red/blue/gray/black).
//   - 모든 픽셀을 칠한 뒤 swap_on_vsync을 단 한 번 호출합니다.
//-----------------------------------------------------------------------------
void draw_board(struct LedPanelSettings *leds, char board[8][8]) {
    // 전체를 검은색으로 클리어
    led_canvas_clear((struct LedCanvas *)leds->canvas);

    for (int y = 0; y < LED_PANEL_SIZE; y++) {
        for (int x = 0; x < LED_PANEL_SIZE; x++) {
            int color_idx;
            // 1픽셀 굵기의 grid-line 판별
            if ((x % 8 == 0) || (x % 8 == 7) || (y % 8 == 0) || (y % 8 == 7)) {
                color_idx = 4;  // 흰색으로 그리드
            } else {
                // interior → 8×8 그리드 셀 내부
                int row = y / 8;  // 0..7
                int col = x / 8;  // 0..7
                char c = board[row][col];
                if      (c == 'R') color_idx = 0;  // 빨강
                else if (c == 'B') color_idx = 1;  // 파랑
                else if (c == '#') color_idx = 2;  // 회색
                else                color_idx = 3;  // '.' 또는 그 외 → 검정
            }

            // 픽셀 칠하기
            led_canvas_set_pixel(
                (struct LedCanvas *)leds->canvas,
                x, y,
                rgb_colors[color_idx].r,
                rgb_colors[color_idx].g,
                rgb_colors[color_idx].b
            );
        }
    }

    // 한 번의 swap_on_vsync으로 전체 화면을 업데이트
    leds->canvas = (struct LedCanvas *)led_matrix_swap_on_vsync(
        (struct LedMatrix *)leds->matrix,
        (struct LedCanvas *)leds->canvas
    );
}

//-----------------------------------------------------------------------------
// led_clear()
//   - 화면 전체를 검은색으로 지우고 즉시 swap합니다.
//-----------------------------------------------------------------------------
void led_clear(struct LedPanelSettings *leds) {
    led_canvas_clear((struct LedCanvas *)leds->canvas);
    leds->canvas = (struct LedCanvas *)led_matrix_swap_on_vsync(
        (struct LedMatrix *)leds->matrix, 
        (struct LedCanvas *)leds->canvas
    );
}

//-----------------------------------------------------------------------------
// led_delete()
//   - load_font() → delete_font()
//   - led_matrix_create_from_options() → led_matrix_delete()
//   - malloc → free()
//-----------------------------------------------------------------------------
void led_delete(struct LedPanelSettings *leds) {
    if (!leds) return;
    delete_font((struct LedFont *)leds->font);
    led_matrix_delete((struct LedMatrix *)leds->matrix);
    free(leds);
}
