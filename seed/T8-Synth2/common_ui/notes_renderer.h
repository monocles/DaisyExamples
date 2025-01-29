#pragma once
#include "drivers/region.h"
#include "drivers/font.h"
#include <cstdio>
#include <cstring>

namespace t8synth {

class NotesRenderer {
public:
    static constexpr uint8_t NOTES_PER_GROUP = 4;
    static constexpr uint8_t PATTERN_SPACING = 15;
    static constexpr uint8_t GROUP_SPACING = 6;
    static constexpr uint8_t PATTERN_LEFT_OFFSET = 3;

    void Init(region* content_region) {
        content_region_ = content_region;
    }

    // Отрисовка групп нот
    void Draw(const uint8_t* left_notes, const uint8_t* right_notes) {
        // Левая группа
        DrawNoteGroup(left_notes, PATTERN_LEFT_OFFSET);

        // Правая группа, смещаем на 6 пикселей влево от расчётной позиции
        uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                                    (NOTES_PER_GROUP * (3 + PATTERN_SPACING)) - GROUP_SPACING;  // Вычитаем 6 пикселей
        DrawNoteGroup(right_notes, second_group_start);
    }

private:
    region* content_region_{nullptr};
    static constexpr size_t NOTE_STR_SIZE = 8; // Увеличиваем размер буфера

    void DrawNoteGroup(const uint8_t* midi_notes, uint32_t base_x) {
        for(uint8_t i = 0; i < NOTES_PER_GROUP; i++) {
            uint32_t x = base_x + (i * PATTERN_SPACING);
            DrawNote(midi_notes[i], x, 0);
        }
    }

    void DrawNote(uint8_t midi_note, uint32_t x, uint32_t y) {
        static const char NOTE_NAMES[] = "CCDDEFFGGAAB";
        static const bool SHARP_TABLE[] = {0,1,0,1,0,0,1,0,1,0,1,0};
        
        uint8_t note_in_octave = midi_note % 12;
        uint8_t octave = (midi_note / 12) - 1;
        
        char note_str[NOTE_STR_SIZE];
        bool is_sharp = SHARP_TABLE[note_in_octave];
        
        if(is_sharp) {
            char* p = note_str;
            *p++ = NOTE_NAMES[note_in_octave];
            *p++ = '#';
            p += snprintf(p, NOTE_STR_SIZE - (p - note_str), "%d", octave);
            *p = '\0';
            
            region_string(content_region_, note_str, x, y, 0xf, 0x0, 0);
        } else {
            char* p = note_str;
            *p++ = NOTE_NAMES[note_in_octave];
            p += snprintf(p, NOTE_STR_SIZE - (p - note_str), "%d", octave);
            *p = '\0';
            
            region_string(content_region_, note_str, x + 3, y, 0xf, 0x0, 0);
        }
    }
};

} // namespace t8synth
