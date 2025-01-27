#include "daisysp.h"
#include "volume_page.h"
#include "drivers/region.h"
#include "drivers/font.h"
#include "stmlib/system/system_clock.h"

namespace t8synth {
using namespace daisysp;
using namespace stmlib;
using namespace daisysp;
using namespace stmlib;

// Определяем массив всех доступных нот
// const char* const VolumePage::available_notes_[] = {
//     "C2", "D2", "E2", "F2", "G2", "A2", "B2", 
//     "C3", "D3", "E3", "F3", "G3", "A3", "B3",
//     "C4", "D4", "E4", "F4", "G4", "A4", "B4"
// };

void VolumePage::OnInit() {
    draw_context_.is_active = true;  // Initialize active state
    InitRegions();
    // InitNotes();
    UpdateFooterValues();  // Use direct method instead of footer_.Update()
}

void VolumePage::DrawSliders() {
    // static constexpr uint8_t PATTERN_HEIGHT = 16;
    static constexpr uint8_t PATTERN_SPACING = 12;
    static constexpr uint8_t GROUP_SPACING = 6;
    static constexpr uint8_t PATTERN_LEFT_OFFSET = 8;
    static constexpr uint8_t NUM_PATTERNS = 4;
    
    // Начинаем рисовать слайдеры с отступом в 1 пиксель от верхней границы нот
    uint32_t start_y = 9; // 8 пикселей на ноты + 1 пиксель отступ
    
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
void VolumePage::DrawSlider(uint32_t x_offset, uint32_t start_y) {
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

void VolumePage::DrawNotes() {
  static constexpr uint8_t PATTERN_SPACING = 12;
  static constexpr uint8_t GROUP_SPACING = 6;
  static constexpr uint8_t PATTERN_LEFT_OFFSET = 3;
  static constexpr uint8_t NUM_PATTERNS = 4;

  // Отрисовка нот для левой группы используя выбранные индексы
  for(uint8_t i = 0; i < NUM_PATTERNS; i++) {
    uint32_t x_offset = PATTERN_LEFT_OFFSET + (i * (3 + PATTERN_SPACING));
    RenderNote(left_notes_[i], x_offset, 0);
  }

  // Отрисовка правой группы нот
  uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                               (NUM_PATTERNS * (3 + PATTERN_SPACING)) + 
                               GROUP_SPACING;

  for(uint8_t i = 0; i < NUM_PATTERNS; i++) {
    uint32_t x_offset = second_group_start + (i * (3 + PATTERN_SPACING));
    RenderNote(right_notes_[i], x_offset, 0);
  }
}

void VolumePage::ShowNoteModal(const Note& note) {
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

void VolumePage::DrawModalFrame() {
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

void VolumePage::DrawModalContent() {
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

    // Отступ слева
    static constexpr uint8_t LEFT_PADDING = 8;
    // Отступ снизу для ноты
    static constexpr uint8_t BOTTOM_PADDING = 8;
    // Отступ между "NOTE:" и нотой
    static constexpr uint8_t VERTICAL_SPACING = 4;

    // Позиция для ноты (в левом нижнем углу)
    uint32_t note_y = MODAL_HEIGHT - FONT2_CHARH - BOTTOM_PADDING;

    // Позиция для текста "NOTE:" (над нотой)
    uint32_t label_y = note_y - FONT_CHARH - VERTICAL_SPACING;

    // Рендерим "NOTE:" стандартным шрифтом
    region_string(&modal_region, "NOTE:", LEFT_PADDING, label_y, 0xF, 0x0, 0);
    
    // Рендерим саму ноту большим шрифтом
    region_string_big(&modal_region, note_str, LEFT_PADDING, note_y, 0x0, 0xF, 1);

    // Рисуем белый квадрат в правом верхнем углу
    static constexpr uint8_t SQUARE_WIDTH = 11;  // Ширина 11
    static constexpr uint8_t SQUARE_HEIGHT = 9; // Высота 10 
    static constexpr uint8_t RIGHT_PADDING = 9;
    static constexpr uint8_t TOP_PADDING = 7;
    
    // Позиция квадрата
    uint32_t square_x = MODAL_WIDTH - SQUARE_WIDTH - RIGHT_PADDING;
    uint32_t square_y = TOP_PADDING;

    // Рисуем белый квадрат
    for(uint32_t y = 0; y < SQUARE_HEIGHT; y++) {
        region_fill_part(&modal_region,
                        (square_y + y) * MODAL_WIDTH + square_x,
                        SQUARE_WIDTH,
                        0xFF);
    }

    // Формируем строку с названием элемента (A1-A4 или B1-B4)
    char element_str[3];
    element_str[0] = current_encoder_ < 4 ? 'A' : 'B';
    element_str[1] = '1' + (current_encoder_ % 4);
    element_str[2] = 0;

    // Центрируем текст в квадрате с учетом разных размеров по ширине и высоте
    uint32_t text_x = square_x + (SQUARE_WIDTH - 7) / 2;  
    uint32_t text_y = square_y + (SQUARE_HEIGHT - 8) / 2;

    // Рендерим название элемента черным по белому
    region_string(&modal_region, element_str, text_x, text_y, 0x0, 0xFF, 0);
}

void VolumePage::DrawModal() {
    if(!modal_visible_) return;
    DrawModalFrame();
    DrawModalContent();
}

void VolumePage::UpdateModal() {
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

void VolumePage::UpdateNoteByEncoder(uint8_t encoder, int32_t increment) {
    current_encoder_ = encoder;  // Обновляем current_encoder_ и здесь для надежности
    Note& current_note = encoder < 4 ? left_notes_[encoder] : right_notes_[encoder - 4];
    uint8_t current_index = GetIndexFromNote(current_note);
    
    int new_index = current_index + increment;
    if(new_index < 0) new_index = 0;
    uint8_t max_index = (MAX_OCTAVE - MIN_OCTAVE + 1) * NOTES_PER_OCTAVE - 1;
    if(new_index > max_index) new_index = max_index;
    
    GetNoteFromIndex(new_index, current_note);
    ShowNoteModal(current_note); // Show modal when note changes
}

void VolumePage::OnEncoder(uint8_t encoder, int32_t increment) {
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

void VolumePage::OnSwitch(uint8_t sw, bool pressed) {
  if(!is_active_) return;

  needs_redraw_ = true;
}

void VolumePage::UpdateDisplay() {
    if(!is_active_ || !draw_context_.is_active) return;

    UpdateModal();

    // If modal is still visible, update only modal content
    if(modal_visible_) {
        if(modal_region.dirty) {
            DrawModal();
            display_->DrawRegion(modal_region.x, modal_region.y,
                               modal_region.w, modal_region.h,
                               modal_region.data);
            modal_region.dirty = 0;
        }
        return; // Skip updating other regions
    }

    // Draw regions in order: header -> content -> footer -> modal
    if(header_region.dirty) {
        display_->DrawRegion(header_region.x, header_region.y,
                           header_region.w, header_region.h,
                           header_region.data);
        header_region.dirty = 0;
    }

    if(content_region.dirty) {
        region_fill(&content_region, 0x0);
        DrawNotes();     // Сначала рисуем ноты
        DrawSliders();   // Затем слайдеры под ними
        DrawIcons();     // Добавляем отрисовку иконок
        
        display_->DrawRegion(content_region.x, content_region.y,
                           content_region.w, content_region.h,
                           content_region.data);
        content_region.dirty = 0;
    }

    if(footer_region_.dirty) {
        region_fill(&footer_region_, 0x0);
        DrawFooterSlider();
        UpdateFooterValues();
        
        display_->DrawRegion(footer_region_.x, footer_region_.y,
                           footer_region_.w, footer_region_.h,
                           footer_region_.data);
        footer_region_.dirty = 0;
    }

    // Draw modal last if visible
    if(modal_visible_ && modal_region.dirty) {
        DrawModal();
        display_->DrawRegion(modal_region.x, modal_region.y,
                           modal_region.w, modal_region.h,
                           modal_region.data);
        modal_region.dirty = 0;
    }
}

void VolumePage::OnEnterPage() {
    is_active_ = true;
    draw_context_.is_active = true;
    
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
    region_string(&header_region, "VOLUME", 2, 0, 0xf, 0x0, 0);

    // Draw all content region elements before updating display
    DrawNotes();     // Сначала ноты
    DrawSliders();   // Затем слайдеры
    DrawIcons();     // И иконки
    
    DrawFooterSlider();
    UpdateFooterValues();

    // Force immediate display update after all drawing is complete
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

void VolumePage::OnExitPage() {
    modal_visible_ = false;
    is_active_ = false;
    draw_context_.is_active = false;
}

void VolumePage::OnClick(uint8_t encoder) {
  if(!is_active_) return;

  needs_redraw_ = true;
}

void VolumePage::OnLongClick(uint8_t encoder) {
  if(!is_active_) return;

  needs_redraw_ = true;
}

void VolumePage::OnIdle() {
  // Обработка простоя
}

void VolumePage::GetNoteFromIndex(uint8_t index, Note& note) {
  static const char bases[] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
  static const bool sharps[] = {false, true, false, true, false, false, true, false, true, false, true, false};
  
  uint8_t octave = MIN_OCTAVE + (index / NOTES_PER_OCTAVE);
  uint8_t note_in_octave = index % NOTES_PER_OCTAVE;
  
  note.base = bases[note_in_octave];
  note.sharp = sharps[note_in_octave];
  note.octave = octave;
}

uint8_t VolumePage::GetIndexFromNote(const Note& note) {
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

void VolumePage::RenderNote(const Note& note, uint32_t x_offset, uint32_t y_offset) {
    // Формируем строку с нотой, включая # для диеза
    char note_str[4];
    if(note.sharp) {
        note_str[0] = note.base;
        note_str[1] = '#';
        note_str[2] = static_cast<char>('0' + note.octave);
        note_str[3] = 0;
        // Диезные ноты рисуем без дополнительного смещения
        region_string(&content_region, note_str, x_offset, y_offset, 0xf, 0x0, 0);
    } else {
        note_str[0] = note.base;
        note_str[1] = static_cast<char>('0' + note.octave);
        note_str[2] = 0;
        // Не диезные ноты сдвигаем на 4 пикселя вправо для центрирования
        region_string(&content_region, note_str, x_offset + 3, y_offset, 0xf, 0x0, 0);
    }
}

void VolumePage::DrawFooterSlider() {
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

void VolumePage::UpdateFooterValues() {
    // Формируем только строки со значениями (без букв A/B)
    char value_a[4];
    char value_b[4];

    if(footer_.value_a == 0) {
        snprintf(value_a, sizeof(value_a), "0");
    } else {
        snprintf(value_a, sizeof(value_a), "%d%c", abs(footer_.value_a),
                 footer_.value_a > 0 ? '+' : '-');
    }

    if(footer_.value_b == 0) {
        snprintf(value_b, sizeof(value_b), "0");
    } else {
        snprintf(value_b, sizeof(value_b), "%d%c", abs(footer_.value_b),
                 footer_.value_b > 0 ? '+' : '-');
    }

    // Очищаем весь футер
    region_fill(&footer_region_, 0x00);

    static constexpr uint8_t BLOCK_WIDTH = 38;
    static constexpr uint8_t BLOCK_HEIGHT = 17;
    static constexpr uint8_t MARGIN = 2;
    static constexpr uint8_t TEXT_MARGIN = 2; // Отступ для текста
    static constexpr uint8_t VALUE_Y_OFFSET = 0; // Смещение для значений сверху

    // Рисуем белые блоки
    for(uint32_t y = 0; y < BLOCK_HEIGHT; y++) {
        // Левый блок
        region_fill_part(&footer_region_, 
                        y * footer_region_.w + MARGIN, 
                        BLOCK_WIDTH, 
                        0xFF);
        // Правый блок
        region_fill_part(&footer_region_, 
                        y * footer_region_.w + (footer_region_.w - MARGIN - BLOCK_WIDTH), 
                        BLOCK_WIDTH, 
                        0xFF);
    }

    // Рисуем статичные буквы A и B большим шрифтом
    region_string_big(&footer_region_, "A", MARGIN + 2, 2, 0xF, 0x0, 1);
    region_string_big(&footer_region_, "B", 
                     footer_region_.w - (FONT2_CHARW - 3), 
                     2, 0xF, 0x0, 1);

    // "TUNE" в левом блоке (прибит к правому нижнему углу)
    region_string(&footer_region_, "TUNE",
                 MARGIN + BLOCK_WIDTH - 17, // 20 - примерная ширина слова "TUNE"
                 BLOCK_HEIGHT - FONT_CHARH - TEXT_MARGIN,
                 0x0, 0xF, 0);

    // "TUNE" в правом блоке (прибит к левому нижнему углу)
    region_string(&footer_region_, "TUNE",
                 footer_region_.w - MARGIN - BLOCK_WIDTH + TEXT_MARGIN,
                 BLOCK_HEIGHT - FONT_CHARH - TEXT_MARGIN,
                 0x0, 0xF, 0);

    // Value A в левом блоке (прибит к правому верхнему углу)
    uint32_t value_a_x = MARGIN + BLOCK_WIDTH - (strlen(value_a) * 4) - TEXT_MARGIN + 1; // Добавили +1 для смещения вправо
    region_string(&footer_region_, value_a,
                 value_a_x,
                 VALUE_Y_OFFSET, // Используем новое смещение
                 0x0, 0xF, 0);

    // Value B в правом блоке (прибит к левому верхнему углу)
    region_string(&footer_region_, value_b,
                 footer_region_.w - MARGIN - BLOCK_WIDTH + TEXT_MARGIN,
                 VALUE_Y_OFFSET, // Используем новое смещение
                 0x0, 0xF, 0);
}

void VolumePage::UpdatePatchNote(const Note& note, plaits::Patch* patch) {
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

void VolumePage::UpdateAllVoicePitches() {
    for(uint8_t i = 0; i < NUM_VOICES; i++) {
        Note& note = i < 4 ? left_notes_[i] : right_notes_[i-4];
        UpdatePatchNote(note, patches_[i]);
    }
}

void VolumePage::InitRegions() {
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

void VolumePage::InitNotes() {
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

void VolumePage::UpdateNote(uint8_t encoder_index, int32_t increment) {
    Note& current_note = encoder_index < 4 ? 
                        left_notes_[encoder_index] : 
                        right_notes_[encoder_index - 4];
    
    UpdateNoteByEncoder(encoder_index, increment);
    content_region.dirty = 1;
    UpdatePatchNote(current_note, patches_[encoder_index]);
}

void VolumePage::Footer::Update() {
    // Update footer display
}

void VolumePage::UpdateRegions() {
    if(header_region.dirty) {
        display_->DrawRegion(header_region.x, header_region.y,
                           header_region.w, header_region.h,
                           header_region.data);
        header_region.dirty = 0;
    }

    // ...similarly update content and footer regions...
}

void VolumePage::DrawIcons() {
    static constexpr uint8_t PATTERN_SPACING = 12;
    static constexpr uint8_t GROUP_SPACING = 6;
    static constexpr uint8_t PATTERN_LEFT_OFFSET = 5;
    static constexpr uint8_t NUM_PATTERNS = 4;
    static constexpr uint8_t ICON_Y_OFFSET = 27; // Позиция иконок под слайдерами
    
    // Рисуем первую группу иконок
    for(uint8_t i = 0; i < NUM_PATTERNS; i++) {
        uint32_t x_offset = PATTERN_LEFT_OFFSET + (i * (3 + PATTERN_SPACING));
        font_glyph_icon('0' + i, 
                       content_region.data + (ICON_Y_OFFSET * content_region.w + x_offset),
                       content_region.w,
                       0xF, 0x0);
    }

    // Рисуем вторую группу иконок
    uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                                 (NUM_PATTERNS * (3 + PATTERN_SPACING)) + 
                                 GROUP_SPACING;
                               
    for(uint8_t i = 0; i < NUM_PATTERNS; i++) {
        uint32_t x_offset = second_group_start + (i * (3 + PATTERN_SPACING));
        font_glyph_icon('4' + i,
                       content_region.data + (ICON_Y_OFFSET * content_region.w + x_offset),
                       content_region.w,
                       0xF, 0x0);
    }
}

}  // namespace t8synth

