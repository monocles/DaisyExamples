#pragma once
#include "drivers/region.h"

namespace t8synth {

class SlidersRenderer {
public:
    static constexpr uint8_t NUM_SLIDERS = 8;
    static constexpr uint8_t SLIDERS_PER_GROUP = 4;
    static constexpr uint8_t PATTERN_SPACING = 12;
    static constexpr uint8_t GROUP_SPACING = 6;
    static constexpr uint8_t PATTERN_LEFT_OFFSET = 8;
    static constexpr uint8_t PATTERN_HEIGHT = 16;
    
    void Init(region* content_region) {
        content_region_ = content_region;
    }
    
    // Отрисовка всех слайдеров с заданным вертикальным смещением
    void Draw(uint8_t y_offset) {
        // Первая группа слайдеров
        for(uint8_t i = 0; i < SLIDERS_PER_GROUP; i++) {
            uint32_t x_offset = PATTERN_LEFT_OFFSET + (i * (3 + PATTERN_SPACING));
            DrawSlider(x_offset, y_offset);
        }

        // Вторая группа слайдеров
        uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                                    (SLIDERS_PER_GROUP * (3 + PATTERN_SPACING)) + 
                                    GROUP_SPACING;
                               
        for(uint8_t i = 0; i < SLIDERS_PER_GROUP; i++) {
            uint32_t x_offset = second_group_start + (i * (3 + PATTERN_SPACING));
            DrawSlider(x_offset, y_offset);
        }
    }

private:
    region* content_region_{nullptr};

    void DrawSlider(uint32_t x_offset, uint32_t start_y) {
        for(int y = 0; y < PATTERN_HEIGHT; y++) {
            uint32_t row_offset = (start_y + y) * content_region_->w + x_offset;
            
            if(y % 2 == 0) {
                region_fill_part(content_region_, row_offset, 1, 0xFF);     // o
                region_fill_part(content_region_, row_offset + 1, 1, 0x00); // x
                region_fill_part(content_region_, row_offset + 2, 1, 0xFF); // o
            } else {
                region_fill_part(content_region_, row_offset, 3, 0x00);     // xxx
            }
        }
    }
};

} // namespace t8synth
