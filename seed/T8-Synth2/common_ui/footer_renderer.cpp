#include "footer_renderer.h"
#include "drivers/font.h"
#include <cstdio>  // for snprintf
#include <cstring> // for strlen
#include <cstdlib> // for abs

namespace t8synth {

void FooterRenderer::DrawFooter(
    region& footer_region,
    const char* labelA,
    const char* labelB,
    const char* blockLabelA,
    const char* blockLabelB,
    int8_t value_a,
    int8_t value_b
) {
    region_fill(&footer_region, 0x00);
    DrawValuesAndBlocks(footer_region, labelA, labelB, blockLabelA, blockLabelB, value_a, value_b);
    DrawFooterSlider(footer_region);
}

void FooterRenderer::DrawValuesAndBlocks(
    region& footer_region,
    const char* labelA,
    const char* labelB,
    const char* blockLabelA,
    const char* blockLabelB,
    int8_t value_a,
    int8_t value_b
) {
    // Увеличиваем размер буфера для безопасного форматирования
    char value_a_str[8]; // Достаточно для "-12+" или "0"
    char value_b_str[8];

    // Безопасное форматирование значений
    if(value_a == 0) {
        value_a_str[0] = '0';
        value_a_str[1] = '\0';
    } else {
        char sign = value_a > 0 ? '+' : '-';
        snprintf(value_a_str, sizeof(value_a_str), "%d%c", abs(value_a), sign);
    }

    if(value_b == 0) {
        value_b_str[0] = '0';
        value_b_str[1] = '\0';
    } else {
        char sign = value_b > 0 ? '+' : '-';
        snprintf(value_b_str, sizeof(value_b_str), "%d%c", abs(value_b), sign);
    }

    static constexpr uint8_t BLOCK_WIDTH = 38;
    static constexpr uint8_t BLOCK_HEIGHT = 17;
    static constexpr uint8_t MARGIN = 2;
    static constexpr uint8_t TEXT_MARGIN = 2;
    static constexpr uint8_t VALUE_Y_OFFSET = 0;

    // Рисуем белые блоки
    for(uint32_t y = 0; y < BLOCK_HEIGHT; y++) {
        region_fill_part(&footer_region, y * footer_region.w + MARGIN, BLOCK_WIDTH, 0xFF);
        region_fill_part(&footer_region,
                         y * footer_region.w + (footer_region.w - MARGIN - BLOCK_WIDTH),
                         BLOCK_WIDTH, 0xFF);
    }

    // Рисуем статичные буквы A и B
    region_string_big(&footer_region, labelA, MARGIN + 2, 2, 0xF, 0x0, 1);
    region_string_big(&footer_region, labelB,
                      footer_region.w - (FONT2_CHARW - 3),
                      2, 0xF, 0x0, 1);

    // Блоковые подписи (TUNE, GAIN и т.д.)
    region_string(&footer_region, blockLabelA,
                  MARGIN + BLOCK_WIDTH - 17,
                  BLOCK_HEIGHT - FONT_CHARH - TEXT_MARGIN,
                  0x0, 0xF, 0);
    region_string(&footer_region, blockLabelB,
                  footer_region.w - MARGIN - BLOCK_WIDTH + TEXT_MARGIN,
                  BLOCK_HEIGHT - FONT_CHARH - TEXT_MARGIN,
                  0x0, 0xF, 0);

    // Значения A и B
    uint32_t value_a_x = MARGIN + BLOCK_WIDTH - (strlen(value_a_str) * 4) - TEXT_MARGIN + 1;
    region_string(&footer_region, value_a_str, value_a_x, VALUE_Y_OFFSET, 0x0, 0xF, 0);
    region_string(&footer_region, value_b_str,
                  footer_region.w - MARGIN - BLOCK_WIDTH + TEXT_MARGIN,
                  VALUE_Y_OFFSET, 0x0, 0xF, 0);
}

void FooterRenderer::DrawFooterSlider(region& footer_region) {
    static constexpr uint8_t SLIDER_WIDTH = 33;
    static constexpr uint8_t SLIDER_HEIGHT = 2;
    uint32_t x_start = (footer_region.w - SLIDER_WIDTH) / 2;
    uint32_t y_start = (footer_region.h - SLIDER_HEIGHT) / 2;

    for(uint8_t y = 0; y < SLIDER_HEIGHT; y++) {
        for(uint8_t x = 0; x < SLIDER_WIDTH; x++) {
            if(x % 2 == 0) {
                uint32_t offset = (y_start + y) * footer_region.w + x_start + x;
                region_fill_part(&footer_region, offset, 1, 0xFF);
            }
        }
    }
}

} // namespace t8synth