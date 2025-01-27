#pragma once
#include "drivers/region.h"
#include "drivers/font.h"
#include <array>

namespace t8synth {

class IconsRenderer {
public:
    static constexpr uint8_t NUM_ICONS = 8;
    static constexpr uint8_t ICONS_PER_GROUP = 4;
    static constexpr uint8_t PATTERN_SPACING = 12;
    static constexpr uint8_t GROUP_SPACING = 6;
    static constexpr uint8_t PATTERN_LEFT_OFFSET = 5;
    
    void Init(region* content_region) {
        content_region_ = content_region;
        // Инициализируем все глифы значением '0'
        glyphs_.fill('0');
    }
    
    // Установить глиф для конкретной иконки
    void SetGlyph(uint8_t index, char glyph) {
        if(index < NUM_ICONS) {
            glyphs_[index] = glyph;
        }
    }

    // Отрисовка всех иконок с заданным вертикальным смещением
    void Draw(uint8_t y_offset) {
        // Первая группа иконок
        for(uint8_t i = 0; i < ICONS_PER_GROUP; i++) {
            uint32_t x_offset = PATTERN_LEFT_OFFSET + (i * (3 + PATTERN_SPACING));
            DrawIcon(i, x_offset, y_offset);
        }

        // Вторая группа иконок
        uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                                    (ICONS_PER_GROUP * (3 + PATTERN_SPACING)) + 
                                    GROUP_SPACING;
                               
        for(uint8_t i = 0; i < ICONS_PER_GROUP; i++) {
            uint32_t x_offset = second_group_start + (i * (3 + PATTERN_SPACING));
            DrawIcon(i + ICONS_PER_GROUP, x_offset, y_offset);
        }
    }

private:
    region* content_region_{nullptr};
    std::array<char, NUM_ICONS> glyphs_;

    void DrawIcon(uint8_t index, uint32_t x_offset, uint8_t y_offset) {
        font_glyph_icon(glyphs_[index],
                       content_region_->data + (y_offset * content_region_->w + x_offset),
                       content_region_->w,
                       0xF, 0x0);
    }
};

} // namespace t8synth
