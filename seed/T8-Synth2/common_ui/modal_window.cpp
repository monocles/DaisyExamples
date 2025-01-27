#include "modal_window.h"
#include "drivers/font.h"
#include "stmlib/system/system_clock.h"

namespace t8synth {
using namespace stmlib;

void ModalWindow::Init(DisplayController* display) {
    display_ = display;
    
    // Initialize modal region
    modal_region_.w = ModalConfig::WIDTH;
    modal_region_.h = ModalConfig::HEIGHT;
    modal_region_.x = (128 - ModalConfig::WIDTH) / 2;  // Center horizontally
    modal_region_.y = (64 - ModalConfig::HEIGHT) / 2;  // Center vertically
    region_alloc(&modal_region_);
}

void ModalWindow::Show(ModalType type) {
    visible_ = true;
    current_type_ = type;
    region_fill(&modal_region_, 0x0);
    DrawFrame();
    Draw();
    modal_region_.dirty = 1;
}

void ModalWindow::Hide() {
    if(visible_) {  // Вызываем NotifyClose только если окно было видимым
        visible_ = false;
        NotifyClose();
    }
}

void ModalWindow::Update(uint32_t current_time) {
    if(visible_ && (current_time - last_update_time_ >= ModalConfig::HIDE_DELAY_MS)) {
        Hide();  // Теперь Hide() вызовет NotifyClose
    }
}

void ModalWindow::SetPitchData(char base, uint8_t octave, bool sharp, uint8_t encoder_id) {
    pitch_data_.base = base;
    pitch_data_.octave = octave;
    pitch_data_.sharp = sharp;
    pitch_data_.encoder_id = encoder_id;
    last_update_time_ = system_clock.milliseconds();
}

void ModalWindow::Draw() {
    if(!visible_) return;
    
    switch(current_type_) {
        case ModalType::PITCH_MODAL:
            DrawPitchContent();
            break;
        // Обработка других типов модальных окон
    }
}

void ModalWindow::DrawFrame() {
    // Draw border
    for(uint32_t x = 0; x < modal_region_.w; x++) {
        region_fill_part(&modal_region_, x, 1, 0xFF);
        region_fill_part(&modal_region_, (modal_region_.h-1) * modal_region_.w + x, 1, 0xFF);
    }
    
    for(uint32_t y = 0; y < modal_region_.h; y++) {
        region_fill_part(&modal_region_, y * modal_region_.w, 1, 0xFF);
        region_fill_part(&modal_region_, y * modal_region_.w + modal_region_.w - 1, 1, 0xFF);
    }
}

void ModalWindow::DrawPitchContent() {
    // Clear inner area
    for(uint32_t y = 1; y < modal_region_.h-1; y++) {
        region_fill_part(&modal_region_, 
                        y * modal_region_.w + 1, 
                        modal_region_.w - 2, 
                        0x0);
    }
    
    char note_str[4];
    if(pitch_data_.sharp) {
        note_str[0] = pitch_data_.base;
        note_str[1] = '#';
        note_str[2] = '0' + pitch_data_.octave;
        note_str[3] = 0;
    } else {
        note_str[0] = pitch_data_.base;
        note_str[1] = '0' + pitch_data_.octave;
        note_str[2] = 0;
    }

    // Constants for layout
    static constexpr uint8_t LEFT_PADDING = 8;
    static constexpr uint8_t BOTTOM_PADDING = 8;
    static constexpr uint8_t VERTICAL_SPACING = 4;

    uint32_t note_y = modal_region_.h - FONT2_CHARH - BOTTOM_PADDING;
    uint32_t label_y = note_y - FONT_CHARH - VERTICAL_SPACING;

    region_string(&modal_region_, "NOTE:", LEFT_PADDING, label_y, 0xF, 0x0, 0);
    region_string_big(&modal_region_, note_str, LEFT_PADDING, note_y, 0x0, 0xF, 1);

    // Draw encoder ID box
    static constexpr uint8_t SQUARE_WIDTH = 11;
    static constexpr uint8_t SQUARE_HEIGHT = 9;
    static constexpr uint8_t RIGHT_PADDING = 9;
    static constexpr uint8_t TOP_PADDING = 7;
    
    uint32_t square_x = modal_region_.w - SQUARE_WIDTH - RIGHT_PADDING;
    uint32_t square_y = TOP_PADDING;

    for(uint32_t y = 0; y < SQUARE_HEIGHT; y++) {
        region_fill_part(&modal_region_,
                        (square_y + y) * modal_region_.w + square_x,
                        SQUARE_WIDTH,
                        0xFF);
    }

    char element_str[3];
    element_str[0] = pitch_data_.encoder_id < 4 ? 'A' : 'B';
    element_str[1] = '1' + (pitch_data_.encoder_id % 4);
    element_str[2] = 0;

    uint32_t text_x = square_x + (SQUARE_WIDTH - 7) / 2;
    uint32_t text_y = square_y + (SQUARE_HEIGHT - 8) / 2;

    region_string(&modal_region_, element_str, text_x, text_y, 0x0, 0xFF, 0);
}

} // namespace t8synth
