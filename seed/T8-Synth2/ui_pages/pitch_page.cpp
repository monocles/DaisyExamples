#include "daisysp.h"
#include "pitch_page.h"
#include "drivers/region.h"
#include "drivers/font.h"
#include "stmlib/system/system_clock.h"

namespace t8synth {
using namespace daisysp;
using namespace stmlib;

// Определяем массив всех доступных нот
const char* const PitchPage::available_notes_[] = {
    "C2", "D2", "E2", "F2", "G2", "A2", "B2", 
    "C3", "D3", "E3", "F3", "G3", "A3", "B3",
    "C4", "D4", "E4", "F4", "G4", "A4", "B4"
};

void PitchPage::OnInit() {
    draw_context_.is_active = true;  // Initialize active state
    InitRegions();
    InitNotes();
    UpdateFooterValues();  // Use direct method instead of footer_.Update()
}

void PitchPage::DrawSliders() {
  static constexpr uint8_t PATTERN_HEIGHT = 16;
  static constexpr uint8_t PATTERN_SPACING = 10;  // Отступ между паттернами
  static constexpr uint8_t GROUP_SPACING = 10;    // Отступ между группами
  static constexpr uint8_t PATTERN_LEFT_OFFSET = 12;  // Начальный отступ слева
  static constexpr uint8_t NUM_PATTERNS = 4;  // Количество паттернов в группе
  
  uint32_t start_y = content_region.h - PATTERN_HEIGHT - 10; // 10 для нот
  
  // Рисуем первую группу из 4 паттернов
  for(uint8_t pattern = 0; pattern < NUM_PATTERNS; pattern++) {
    uint32_t x_offset = PATTERN_LEFT_OFFSET + (pattern * (3 + PATTERN_SPACING));
    DrawSlider(x_offset, start_y);
  }

  // Рисуем вторую группу из 4 паттернов с дополнительным отступом
  uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                               (NUM_PATTERNS * (3 + PATTERN_SPACING)) + 
                               GROUP_SPACING;
                               
  for(uint8_t pattern = 0; pattern < NUM_PATTERNS; pattern++) {
    uint32_t x_offset = second_group_start + (pattern * (3 + PATTERN_SPACING));
    DrawSlider(x_offset, start_y);
  }
}

// Вспомогательный метод для отрисовки одного паттерна
void PitchPage::DrawSlider(uint32_t x_offset, uint32_t start_y) {
  static constexpr uint8_t PATTERN_HEIGHT = 16;
  
  for(int y = 0; y < PATTERN_HEIGHT; y++) {
    uint32_t row_offset = (start_y + y) * content_region.w + x_offset;
    
    if(y % 2 == 0) {
      region_fill_part(&content_region, row_offset, 1, 0xFF);     // o
      region_fill_part(&content_region, row_offset + 1, 1, 0x00); // x
      region_fill_part(&content_region, row_offset + 2, 1, 0xFF); // o
    } else {
      region_fill_part(&content_region, row_offset, 3, 0x00);     // xxx
    }
  }
}

void PitchPage::DrawNotes() {
  static constexpr uint8_t PATTERN_SPACING = 10;
  static constexpr uint8_t GROUP_SPACING = 10;
  static constexpr uint8_t PATTERN_LEFT_OFFSET = 12;
  static constexpr uint8_t NUM_PATTERNS = 4;

  // Отрисовка нот для левой группы используя выбранные индексы
  for(uint8_t i = 0; i < NUM_PATTERNS; i++) {
    uint32_t x_offset = PATTERN_LEFT_OFFSET + (i * (3 + PATTERN_SPACING)) - 3;
    RenderNote(left_notes_[i], x_offset, 2);
  }

  // Отрисовка правой группы нот
  uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                               (NUM_PATTERNS * (3 + PATTERN_SPACING)) + 
                               GROUP_SPACING;

  for(uint8_t i = 0; i < NUM_PATTERNS; i++) {
    uint32_t x_offset = second_group_start + (i * (3 + PATTERN_SPACING)) - 3;
    RenderNote(right_notes_[i], x_offset, 2);
  }
}

void PitchPage::ShowNoteModal(const Note& note) {
    bool note_changed = !modal_visible_ || 
                       current_modal_note_.base != note.base || 
                       current_modal_note_.octave != note.octave ||
                       current_modal_note_.sharp != note.sharp;
    
    last_note_change_time_ = system_clock.milliseconds();
    
    if(!modal_visible_) {
        // Сохраняем текущее состояние экрана перед показом модального окна
        current_modal_note_ = note;
        modal_visible_ = true;
        region_fill(&modal_region, 0x0);
        DrawModalFrame();
        DrawModalContent();
        modal_region.dirty = 1;
    } else if(note_changed) {
        // Обновляем только содержимое модального окна
        current_modal_note_ = note;
        DrawModalContent();
        modal_region.dirty = 1;
    }
}

void PitchPage::DrawModalFrame() {
    // Draw border
    for(uint32_t x = 0; x < MODAL_WIDTH; x++) {
        region_fill_part(&modal_region, x, 1, 0xFF);
        region_fill_part(&modal_region, (MODAL_HEIGHT-1) * MODAL_WIDTH + x, 1, 0xFF);
    }
    
    for(uint32_t y = 0; y < MODAL_HEIGHT; y++) {
        region_fill_part(&modal_region, y * MODAL_WIDTH, 1, 0xFF);
        region_fill_part(&modal_region, y * MODAL_WIDTH + MODAL_WIDTH - 1, 1, 0xFF);
    }
}

void PitchPage::DrawModalContent() {
    // Очищаем только внутреннюю область
    for(uint32_t y = 1; y < MODAL_HEIGHT-1; y++) {
        region_fill_part(&modal_region, 
                        y * MODAL_WIDTH + 1, 
                        MODAL_WIDTH - 2, 
                        0x0);
    }
    
    char note_str[4];
    if(current_modal_note_.sharp) {
        note_str[0] = current_modal_note_.base;
        note_str[1] = '#';
        note_str[2] = '0' + current_modal_note_.octave;
        note_str[3] = 0;
    } else {
        note_str[0] = current_modal_note_.base;
        note_str[1] = '0' + current_modal_note_.octave;
        note_str[2] = 0;
    }

    uint32_t text_width = current_modal_note_.sharp ? 24 : 16;
    uint32_t text_x = (MODAL_WIDTH - text_width) / 2;
    uint32_t text_y = (MODAL_HEIGHT - 16) / 2;
    region_string(&modal_region, note_str, text_x, text_y, 0xF, 0x0, 1);
}

void PitchPage::DrawModal() {
    if(!modal_visible_) return;
    DrawModalFrame();
    DrawModalContent();
}

void PitchPage::UpdateModal() {
    if(modal_visible_) {
        uint32_t now = system_clock.milliseconds();
        if(now - last_note_change_time_ >= MODAL_HIDE_DELAY_MS) {
            modal_visible_ = false;
            // Mark other regions only after modal closes
            header_region.dirty = 1;
            content_region.dirty = 1;
            footer_region_.dirty = 1;
        }
    }
}

void PitchPage::UpdateNoteByEncoder(uint8_t encoder, int32_t increment) {
    Note& current_note = encoder < 4 ? left_notes_[encoder] : right_notes_[encoder - 4];
    uint8_t current_index = GetIndexFromNote(current_note);
    
    int new_index = current_index + increment;
    if(new_index < 0) new_index = 0;
    uint8_t max_index = (MAX_OCTAVE - MIN_OCTAVE + 1) * NOTES_PER_OCTAVE - 1;
    if(new_index > max_index) new_index = max_index;
    
    GetNoteFromIndex(new_index, current_note);
    ShowNoteModal(current_note); // Show modal when note changes
}

void PitchPage::OnEncoder(uint8_t encoder, int32_t increment) {
    if(!draw_context_.is_active) return;

    if(encoder < 8) {
        // Voice encoders
        UpdateNote(encoder, increment);
    }
    else {
        // Mod encoders
        using ENC = EncoderController::EncoderIndex;
        if(encoder == ENC::ENC_MOD_A || encoder == ENC::ENC_MOD_B) {
            if(modal_visible_) {
                modal_visible_ = false;
                header_region.dirty = 1;
                content_region.dirty = 1;
                footer_region_.dirty = 1;
            }
            if(encoder == ENC::ENC_MOD_A) {
                footer_.value_a = fclamp(footer_.value_a + increment, -12, 12);
                UpdateAllVoicePitches();
            }
            else if(encoder == ENC::ENC_MOD_B) {
                footer_.value_b = fclamp(footer_.value_b + increment, -12, 12);
            }
            footer_.Update();
            footer_region_.dirty = 1; // Fixed: using footer_region_ instead of regions_.footer
        }
    }
}

void PitchPage::OnSwitch(uint8_t sw, bool pressed) {
  if(!is_active_) return;

  needs_redraw_ = true;
}

void PitchPage::UpdateDisplay() {
    if(!is_active_ || !draw_context_.is_active) return;

    UpdateModal();

    // If modal is still visible, update only modal content
    if(modal_visible_) {
        if(modal_region.dirty) {
            DrawModal();
            // pots_->Freeze();
            display_->DrawRegion(modal_region.x, modal_region.y,
                               modal_region.w, modal_region.h,
                               modal_region.data);
            // pots_->Unfreeze();
            modal_region.dirty = 0;
        }
        return; // Skip updating other regions
    }

    // Draw regions in order: header -> content -> footer -> modal
    if(header_region.dirty) {
        // pots_->Freeze();
        display_->DrawRegion(header_region.x, header_region.y,
                           header_region.w, header_region.h,
                           header_region.data);
        // pots_->Unfreeze();
        header_region.dirty = 0;
    }

    if(content_region.dirty) {
        region_fill(&content_region, 0x0);
        DrawSliders();
        DrawNotes();
        
        // pots_->Freeze();
        display_->DrawRegion(content_region.x, content_region.y,
                           content_region.w, content_region.h,
                           content_region.data);
        // pots_->Unfreeze();
        content_region.dirty = 0;
    }

    if(footer_region_.dirty) {
        region_fill(&footer_region_, 0x0);
        DrawFooterSlider();
        UpdateFooterValues();
        
        // pots_->Freeze();
        display_->DrawRegion(footer_region_.x, footer_region_.y,
                           footer_region_.w, footer_region_.h,
                           footer_region_.data);
        // pots_->Unfreeze();
        footer_region_.dirty = 0;
    }

    // Draw modal last if visible
    if(modal_visible_ && modal_region.dirty) {
        DrawModal();
        // pots_->Freeze();
        display_->DrawRegion(modal_region.x, modal_region.y,
                           modal_region.w, modal_region.h,
                           modal_region.data);
        // pots_->Unfreeze();
        modal_region.dirty = 0;
    }
}

void PitchPage::OnEnterPage() {
    is_active_ = true;
    draw_context_.is_active = true;  // Set active state
    
    // Clear all regions
    region_fill(&header_region, 0x0);
    region_fill(&content_region, 0x0);
    region_fill(&footer_region_, 0x0);
    region_fill(&modal_region, 0x0);

    // Mark all regions as dirty
    header_region.dirty = 1;
    content_region.dirty = 1;
    footer_region_.dirty = 1;
    modal_region.dirty = 1;

    // Draw initial content
    region_string(&header_region, "PITCH", 2, 2, 0xf, 0x0, 0);
    region_string(&header_region, "C MINOR", 34, 2, 0xf, 0x0, 0);

    DrawSliders();
    DrawNotes();
    DrawFooterSlider();
    UpdateFooterValues();

    // Force immediate display update
    display_->DrawRegion(header_region.x, header_region.y,
                      header_region.w, header_region.h,
                      header_region.data);
    
    display_->DrawRegion(content_region.x, content_region.y,
                      content_region.w, content_region.h,
                      content_region.data);
    
    display_->DrawRegion(footer_region_.x, footer_region_.y,
                      footer_region_.w, footer_region_.h,
                      footer_region_.data);
}

void PitchPage::OnExitPage() {
    modal_visible_ = false;
    is_active_ = false;
    draw_context_.is_active = false;
}

void PitchPage::OnClick(uint8_t encoder) {
  if(!is_active_) return;

  needs_redraw_ = true;
}

void PitchPage::OnLongClick(uint8_t encoder) {
  if(!is_active_) return;

  needs_redraw_ = true;
}

void PitchPage::OnIdle() {
  // Обработка простоя
}

void PitchPage::GetNoteFromIndex(uint8_t index, Note& note) {
  static const char bases[] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
  static const bool sharps[] = {false, true, false, true, false, false, true, false, true, false, true, false};
  
  uint8_t octave = MIN_OCTAVE + (index / NOTES_PER_OCTAVE);
  uint8_t note_in_octave = index % NOTES_PER_OCTAVE;
  
  note.base = bases[note_in_octave];
  note.sharp = sharps[note_in_octave];
  note.octave = octave;
}

uint8_t PitchPage::GetIndexFromNote(const Note& note) {
  uint8_t base_index = 0;
  switch(note.base) {
    case 'C': base_index = 0; break;
    case 'D': base_index = 2; break;
    case 'E': base_index = 4; break;
    case 'F': base_index = 5; break;
    case 'G': base_index = 7; break;
    case 'A': base_index = 9; break;
    case 'B': base_index = 11; break;
  }
  
  return (note.octave - MIN_OCTAVE) * NOTES_PER_OCTAVE + base_index + (note.sharp ? 1 : 0);
}

void PitchPage::RenderNote(const Note& note, uint32_t x_offset, uint32_t y_offset) {
    // Используем правильное приведение типов
    char note_str[3] = {
        note.base, 
        static_cast<char>('0' + note.octave), 
        0
    };
  
  // Подвигаем ноты ближе к нижней части региона
  uint32_t text_y = content_region.h - 8; // 8 пикселей от низа
  
  if(note.sharp) {
    // Рисуем подчеркивание над текстом
    region_fill_part(&content_region, 
                    (text_y - 2) * content_region.w + x_offset, // На 2 пикселя выше текста
                    9, // Длина подчеркивания
                    0xFF); // Белый цвет
  }
  
  // Отрисовываем текст ноты
  region_string(&content_region, note_str, x_offset, text_y, 0xf, 0x0, 0);
}

void PitchPage::DrawFooterSlider() {
  static constexpr uint8_t SLIDER_WIDTH = 33;
  static constexpr uint8_t SLIDER_HEIGHT = 2;
  
  // Вычисляем центральную позицию для слайдера
  uint32_t x_start = (footer_region_.w - SLIDER_WIDTH) / 2;
  uint32_t y_start = (footer_region_.h - SLIDER_HEIGHT) / 2;
  
  // Отрисовываем слайдер попиксельно
  for(uint8_t y = 0; y < SLIDER_HEIGHT; y++) {
    for(uint8_t x = 0; x < SLIDER_WIDTH; x++) {
      if(x % 2 == 0) { // Каждый четный пиксель
        uint32_t offset = (y_start + y) * footer_region_.w + x_start + x;
        region_fill_part(&footer_region_, offset, 1, 0xFF);
      }
    }
  }
}

void PitchPage::UpdateFooterValues() {
    // Use footer_.value_a / footer_.value_b
    char str_a[5];
    char str_b[5];

    if(footer_.value_a == 0) {
        snprintf(str_a, sizeof(str_a), "A0");
    } else {
        snprintf(str_a, sizeof(str_a), "A%d%c", abs(footer_.value_a),
                 footer_.value_a > 0 ? '+' : '-');
    }

    if(footer_.value_b == 0) {
        snprintf(str_b, sizeof(str_b), "B0");
    } else {
        snprintf(str_b, sizeof(str_b), "B%d%c", abs(footer_.value_b),
                 footer_.value_b > 0 ? '+' : '-');
    }

    region_fill(&footer_region_, 0x00);
    DrawFooterSlider();
    region_string(&footer_region_, "SCALE", 52, 11, 0xf, 0x0, 0);
    region_string_font2(&footer_region_, str_a, 20, 4, 0xf, 0x0);
    region_string_font2(&footer_region_, str_b, 90, 4, 0xf, 0x0);
}

void PitchPage::UpdatePatchNote(const Note& note, plaits::Patch* patch) {
    int base_note = 60; // C4 = 60 в MIDI
    int octave_offset = (note.octave - 4) * 12;
    int note_offset = 0;
    
    switch(note.base) {
        case 'C': note_offset = 0; break;
        case 'D': note_offset = 2; break;
        case 'E': note_offset = 4; break;
        case 'F': note_offset = 5; break;
        case 'G': note_offset = 7; break;
        case 'A': note_offset = 9; break;
        case 'B': note_offset = 11; break;
    }
    
    if(note.sharp) note_offset++;
    // Use footer_.value_a instead of footer_value_a_
    patch->note = base_note + octave_offset + note_offset + footer_.value_a;
}

void PitchPage::UpdateAllVoicePitches() {
    for(uint8_t i = 0; i < NUM_VOICES; i++) {
        Note& note = i < 4 ? left_notes_[i] : right_notes_[i-4];
        UpdatePatchNote(note, patches_[i]);
    }
}

void PitchPage::InitRegions() {
    // Initialize header
    header_region.w = DisplayLayout::SCREEN_WIDTH;
    header_region.h = DisplayLayout::HEADER_HEIGHT;
    header_region.x = 0;
    header_region.y = 0;
    region_alloc(&header_region);

    // Initialize content
    content_region.w = DisplayLayout::SCREEN_WIDTH;
    content_region.h = DisplayLayout::CONTENT_HEIGHT;
    content_region.x = 0;
    content_region.y = header_region.h;
    region_alloc(&content_region);

    // Initialize footer
    footer_region_.w = DisplayLayout::SCREEN_WIDTH;
    footer_region_.h = DisplayLayout::SCREEN_HEIGHT - (header_region.h + content_region.h);
    footer_region_.x = 0;
    footer_region_.y = header_region.h + content_region.h;
    region_alloc(&footer_region_);

    // Initialize modal
    modal_region.w = ModalConfig::WIDTH;
    modal_region.h = ModalConfig::HEIGHT;
    modal_region.x = (DisplayLayout::SCREEN_WIDTH - ModalConfig::WIDTH) / 2;
    modal_region.y = (DisplayLayout::SCREEN_HEIGHT - ModalConfig::HEIGHT) / 2;
    region_alloc(&modal_region);
}

void PitchPage::InitNotes() {
    // Initialize left notes
    left_notes_[0] = {.base = 'C', .octave = 2, .sharp = false};
    left_notes_[1] = {.base = 'G', .octave = 2, .sharp = false};
    left_notes_[2] = {.base = 'A', .octave = 2, .sharp = false};
    left_notes_[3] = {.base = 'C', .octave = 4, .sharp = false};

    // Initialize right notes
    right_notes_[0] = {.base = 'C', .octave = 4, .sharp = false};
    right_notes_[1] = {.base = 'G', .octave = 2, .sharp = false};
    right_notes_[2] = {.base = 'A', .octave = 2, .sharp = false};
    right_notes_[3] = {.base = 'C', .octave = 4, .sharp = false};
}

void PitchPage::UpdateNote(uint8_t encoder_index, int32_t increment) {
    Note& current_note = encoder_index < 4 ? 
                        left_notes_[encoder_index] : 
                        right_notes_[encoder_index - 4];
    
    UpdateNoteByEncoder(encoder_index, increment);
    content_region.dirty = 1;
    UpdatePatchNote(current_note, patches_[encoder_index]);
}

void PitchPage::Footer::Update() {
    // Update footer display
}

void PitchPage::UpdateRegions() {
    if(header_region.dirty) {
        // pots_->Freeze();
        display_->DrawRegion(header_region.x, header_region.y,
                           header_region.w, header_region.h,
                           header_region.data);
        // pots_->Unfreeze();
        header_region.dirty = 0;
    }

    // ...similarly update content and footer regions...
}

}  // namespace t8synth
