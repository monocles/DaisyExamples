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

    // Добавляем массив для хранения значений заполнения слайдеров (0-100%)
    uint8_t slider_values_[NUM_SLIDERS]{0};
    
    void Init(region* content_region) {
        content_region_ = content_region;
    }
    
    // Метод для обновления значения слайдера
    void SetSliderValue(uint8_t slider_index, uint8_t value) {
        if(slider_index < NUM_SLIDERS) {
            slider_values_[slider_index] = value;
        }
    }

    // Отрисовка всех слайдеров с заданным вертикальным смещением
    void Draw(uint8_t y_offset) {
        // Первая группа слайдеров
        for(uint8_t i = 0; i < SLIDERS_PER_GROUP; i++) {
            uint32_t x_offset = PATTERN_LEFT_OFFSET + (i * (3 + PATTERN_SPACING));
            DrawSlider(x_offset, y_offset, slider_values_[i]);
        }

        // Вторая группа слайдеров
        uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                                    (SLIDERS_PER_GROUP * (3 + PATTERN_SPACING)) + 
                                    GROUP_SPACING;
                               
        for(uint8_t i = 0; i < SLIDERS_PER_GROUP; i++) {
            uint32_t x_offset = second_group_start + (i * (3 + PATTERN_SPACING));
            DrawSlider(x_offset, y_offset, slider_values_[i + SLIDERS_PER_GROUP]);
        }
    }

private:
    region* content_region_{nullptr};

    void DrawSlider(uint32_t x_offset, uint32_t start_y, uint8_t value) {
        // Вычисляем количество заполненных пикселей
        uint8_t filled_pixels = (PATTERN_HEIGHT * value) / 100;

        for(int y = 0; y < PATTERN_HEIGHT; y++) {
            uint32_t row_offset = (start_y + y) * content_region_->w + x_offset;
            
            // Проверяем, должна ли эта строка быть заполнена
            bool should_fill = y >= (PATTERN_HEIGHT - filled_pixels);
            
            if(y % 2 == 0) {
                // Контур слайдера
                region_fill_part(content_region_, row_offset, 1, 0xFF);     // o
                region_fill_part(content_region_, row_offset + 1, 1, 0x00); // x
                region_fill_part(content_region_, row_offset + 2, 1, 0xFF); // o
            } else {
                // Заполнение слайдера
                region_fill_part(content_region_, row_offset, 3, should_fill ? 0xFF : 0x00);
            }
        }
    }
};

} // namespace t8synth
