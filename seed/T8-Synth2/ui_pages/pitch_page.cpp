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
    modal_window_.Init(display_); // Инициализируем модальное окно
    modal_window_.SetOnCloseCallback([](void* context) {
        auto page = static_cast<PitchPage*>(context);
        // Помечаем все регионы для перерисовки при закрытии модального окна
        page->header_region.dirty = 1;
        page->content_region.dirty = 1;
        page->footer_region_.dirty = 1;
    }, this);
    icons_renderer_.Init(&content_region);
    sliders_renderer_.Init(&content_region); // Инициализируем рендерер слайдеров
    
    // Устанавливаем глифы для каждой иконки
    for(uint8_t i = 0; i < IconsRenderer::NUM_ICONS; i++) {
        icons_renderer_.SetGlyph(i, '0' + i);
    }
}

void PitchPage::DrawSliders() {
    static constexpr uint8_t SLIDER_Y_OFFSET = 9;
    sliders_renderer_.Draw(SLIDER_Y_OFFSET);
}

void PitchPage::DrawNotes() {
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

void PitchPage::ShowNoteModal(const Note& note) {
    modal_window_.SetPitchData(note.base, note.octave, note.sharp, current_encoder_);
    modal_window_.Show(ModalType::PITCH_MODAL);
}

void PitchPage::UpdateDisplay() {
    if(!is_active_ || !draw_context_.is_active) return;

    modal_window_.Update(system_clock.milliseconds());

    if(modal_window_.IsVisible()) {
        if(modal_window_.GetRegion().dirty) {
            modal_window_.Draw();
            display_->DrawRegion(modal_window_.GetRegion().x, 
                               modal_window_.GetRegion().y,
                               modal_window_.GetRegion().w, 
                               modal_window_.GetRegion().h,
                               modal_window_.GetRegion().data);
            modal_window_.GetRegion().dirty = 0;
        }
        return;
    }

    // Draw regions in order: header -> content -> footer
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
            // Сразу закрываем модальное окно при работе с футером
            modal_window_.Hide();
            
            if(encoder == ENC::ENC_MOD_A) {
                footer_.value_a = fclamp(footer_.value_a + increment, -12, 12);
                UpdateAllVoicePitches();
            }
            else if(encoder == ENC::ENC_MOD_B) {
                footer_.value_b = fclamp(footer_.value_b + increment, -12, 12);
            }
            footer_.Update();
            footer_region_.dirty = 1;
        }
    }
}

void PitchPage::OnSwitch(uint8_t sw, bool pressed) {
  if(!is_active_) return;

  needs_redraw_ = true;
}

void PitchPage::OnEnterPage() {
    is_active_ = true;
    draw_context_.is_active = true;
    
    // Clear all regions
    region_fill(&header_region, 0x0);
    region_fill(&content_region, 0x0);
    region_fill(&footer_region_, 0x0);

    // Mark all regions as dirty
    header_region.dirty = 1;
    content_region.dirty = 1;
    footer_region_.dirty = 1;

    // Draw initial content
    region_string(&header_region, "PITCH", 2, 0, 0xf, 0x0, 0);
    region_string(&header_region, "C MINOR", 34, 0, 0xf, 0x0, 0);

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

void PitchPage::OnExitPage() {
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
        display_->DrawRegion(header_region.x, header_region.y,
                           header_region.w, header_region.h,
                           header_region.data);
        header_region.dirty = 0;
    }

    // ...similarly update content and footer regions...
}

void PitchPage::DrawIcons() {
    static constexpr uint8_t ICON_Y_OFFSET = 27;
    icons_renderer_.Draw(ICON_Y_OFFSET);
}

void PitchPage::UpdateNoteByEncoder(uint8_t encoder, int32_t increment) {
    current_encoder_ = encoder;  // Сохраняем текущий энкодер
    
    Note& current_note = encoder < 4 ? left_notes_[encoder] : right_notes_[encoder - 4];
    uint8_t current_index = GetIndexFromNote(current_note);
    
    int new_index = current_index + increment;
    if(new_index < 0) new_index = 0;
    uint8_t max_index = (MAX_OCTAVE - MIN_OCTAVE + 1) * NOTES_PER_OCTAVE - 1;
    if(new_index > max_index) new_index = max_index;
    
    GetNoteFromIndex(new_index, current_note);
    ShowNoteModal(current_note); // Show modal when note changes
}

}  // namespace t8synth

