#include "daisysp.h"
#include "pitch_page.h"
#include "drivers/region.h"
#include "drivers/font.h"

namespace t8synth {
using namespace daisysp;

// Определяем массив всех доступных нот
const char* const PitchPage::available_notes_[] = {
    "C2", "D2", "E2", "F2", "G2", "A2", "B2", 
    "C3", "D3", "E3", "F3", "G3", "A3", "B3",
    "C4", "D4", "E4", "F4", "G4", "A4", "B4"
};

void PitchPage::OnInit() {
//   UiPage::OnInit();  // Вызываем базовый метод

  // Инициализация основного региона дисплея
  display_region_.w = 128;
  display_region_.h = 9;
  display_region_.x = 0;
  display_region_.y = 0;
  region_alloc(&display_region_);

  // Инициализация региона для шаблона
  pattern_region_.w = 128;  
  pattern_region_.h = 24;
  pattern_region_.x = 0; 
  pattern_region_.y = 9; // Располагаем под display_region
  region_alloc(&pattern_region_);
  
  region_fill(&pattern_region_, 0x0); // Очищаем регион
  DrawPattern();

  // Инициализация региона для нот
  note_region_.w = 128;
  note_region_.h = 10;
  note_region_.x = 0;
  note_region_.y = pattern_region_.y + pattern_region_.h; // Размещаем под pattern_region
  region_alloc(&note_region_);
  
  region_fill(&note_region_, 0x0);
  DrawNotes();

  // Инициализация футера
  footer_region_.w = 128;
  footer_region_.h = 64 - (note_region_.y + note_region_.h); // Оставшаяся высота
  footer_region_.x = 0;
  footer_region_.y = note_region_.y + note_region_.h;
  region_alloc(&footer_region_);
  
  region_fill(&footer_region_, 0x00); 
  DrawFooterSlider(); // Добавляем отрисовку слайдера
  // Добавляем текст A2 в футер
  region_string_font2(&footer_region_, "A2", 28, 4, 0xf, 0x0);
  region_string_font2(&footer_region_, "B0", 90, 4, 0xf, 0x0);

  // Инициализируем начальные значения нот
  left_notes_[0] = {.base = 'C', .octave = 2, .sharp = false}; // C2
  left_notes_[1] = {.base = 'G', .octave = 2, .sharp = false}; // G2
  left_notes_[2] = {.base = 'A', .octave = 2, .sharp = false}; // A2
  left_notes_[3] = {.base = 'C', .octave = 4, .sharp = false}; // C4

  // Инициализируем начальные значения правых нот
  right_notes_[0] = {.base = 'C', .octave = 4, .sharp = false};
  right_notes_[1] = {.base = 'G', .octave = 2, .sharp = false};
  right_notes_[2] = {.base = 'A', .octave = 2, .sharp = false};
  right_notes_[3] = {.base = 'C', .octave = 4, .sharp = false};

  // Заполняем регион начальным содержимым
//   region_fill(&display_region_, 0x0);
//   region_string_font2(&display_region_, "PITCH PAGE", 2, 2, 0xf, 0x0);

  // Инициализируем значения футера
  footer_value_a_ = 0;
  footer_value_b_ = 0;
  UpdateFooterValues();
}

void PitchPage::DrawPattern() {
  static constexpr uint8_t PATTERN_HEIGHT = 16;
  static constexpr uint8_t PATTERN_SPACING = 10;  // Отступ между паттернами
  static constexpr uint8_t GROUP_SPACING = 10;    // Отступ между группами
  static constexpr uint8_t PATTERN_LEFT_OFFSET = 12;  // Начальный отступ слева
  static constexpr uint8_t NUM_PATTERNS = 4;  // Количество паттернов в группе
  
  uint32_t start_y = pattern_region_.h - PATTERN_HEIGHT;
  
  // Рисуем первую группу из 4 паттернов
  for(uint8_t pattern = 0; pattern < NUM_PATTERNS; pattern++) {
    uint32_t x_offset = PATTERN_LEFT_OFFSET + (pattern * (3 + PATTERN_SPACING));
    DrawSinglePattern(x_offset, start_y);
  }

  // Рисуем вторую группу из 4 паттернов с дополнительным отступом
  uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                               (NUM_PATTERNS * (3 + PATTERN_SPACING)) + 
                               GROUP_SPACING;
                               
  for(uint8_t pattern = 0; pattern < NUM_PATTERNS; pattern++) {
    uint32_t x_offset = second_group_start + (pattern * (3 + PATTERN_SPACING));
    DrawSinglePattern(x_offset, start_y);
  }
}

// Вспомогательный метод для отрисовки одного паттерна
void PitchPage::DrawSinglePattern(uint32_t x_offset, uint32_t start_y) {
  static constexpr uint8_t PATTERN_HEIGHT = 16;
  
  for(int y = 0; y < PATTERN_HEIGHT; y++) {
    uint32_t row_offset = (start_y + y) * pattern_region_.w + x_offset;
    
    if(y % 2 == 0) {
      region_fill_part(&pattern_region_, row_offset, 1, 0xFF);     // o
      region_fill_part(&pattern_region_, row_offset + 1, 1, 0x00); // x
      region_fill_part(&pattern_region_, row_offset + 2, 1, 0xFF); // o
    } else {
      region_fill_part(&pattern_region_, row_offset, 3, 0x00);     // xxx
    }
  }
}

void PitchPage::DrawNotes() {
  static constexpr uint8_t PATTERN_SPACING = 10;
  static constexpr uint8_t GROUP_SPACING = 10;
  static constexpr uint8_t PATTERN_LEFT_OFFSET = 12;
  static constexpr uint8_t NUM_PATTERNS = 4;

  region_fill(&note_region_, 0x0); // Очищаем регион

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

void PitchPage::UpdateNoteByEncoder(uint8_t encoder, int32_t increment) {
  if(encoder >= 8) return; // Обрабатываем 8 энкодеров (4 левых + 4 правых)
  
  Note& current_note = encoder < 4 ? left_notes_[encoder] : right_notes_[encoder - 4];
  uint8_t current_index = GetIndexFromNote(current_note);
  
  int new_index = current_index + increment;
  if(new_index < 0) new_index = 0;
  uint8_t max_index = (MAX_OCTAVE - MIN_OCTAVE + 1) * NOTES_PER_OCTAVE - 1;
  if(new_index > max_index) new_index = max_index;
  
  GetNoteFromIndex(new_index, current_note);
  needs_redraw_ = true;
}

void PitchPage::OnEncoder(uint8_t encoder, int32_t increment) {
    if(!is_active_) return;
    
    if(encoder < 4) {
        UpdateNoteByEncoder(encoder, increment);
    } 
    else if(encoder >= 7 && encoder <= 10) {
        UpdateNoteByEncoder(encoder - 7 + 4, increment);
    }
    else if(encoder == 5) {
        footer_value_a_ = fclamp(footer_value_a_ + increment, -12, 12);
        UpdateFooterValues();
    }
    else if(encoder == 6) {
        footer_value_b_ = fclamp(footer_value_b_ + increment, -12, 12);
        UpdateFooterValues();
    }
    
    needs_redraw_ = true;
}

void PitchPage::OnSwitch(uint8_t sw, bool pressed) {
  if(!is_active_) return;

  needs_redraw_ = true;
}

void PitchPage::UpdateDisplay() {
    if(!is_active_) return;

    static uint32_t last_update_time = 0;
    uint32_t current_time = System::GetNow();
    
    // Принудительно обновляем если прошло достаточно времени
    
    if(needs_redraw_ ) {
        DrawPattern();
        DrawNotes();
        DrawFooterSlider();
        UpdateFooterValues();
        
        display_->DrawRegion(display_region_.x, display_region_.y, 
                           display_region_.w, display_region_.h,
                           display_region_.data);
        
        display_->DrawRegion(pattern_region_.x, pattern_region_.y,
                           pattern_region_.w, pattern_region_.h, 
                           pattern_region_.data);

        display_->DrawRegion(note_region_.x, note_region_.y,
                           note_region_.w, note_region_.h, 
                           note_region_.data);

        display_->DrawRegion(footer_region_.x, footer_region_.y,
                           footer_region_.w, footer_region_.h, 
                           footer_region_.data);
                           
        needs_redraw_ = false;
        last_update_time = current_time;
    }
}

void PitchPage::OnEnterPage() {
    is_active_ = true;
    needs_redraw_ = true;

    // Очищаем все регионы
    region_fill(&display_region_, 0x0);
    region_fill(&pattern_region_, 0x0);
    region_fill(&note_region_, 0x0);
    region_fill(&footer_region_, 0x0);

    // Отрисовываем начальное содержимое
    region_string(&display_region_, "PITCH", 2, 2, 0xf, 0x0, 0);
    region_string(&display_region_, "C MINOR", 34, 2, 0xf, 0x0, 0);

    // Отрисовываем все компоненты
    DrawPattern();
    DrawNotes();
    DrawFooterSlider();
    UpdateFooterValues();

    // Принудительно устанавливаем флаг обновления
    needs_redraw_ = true;
    // Сразу вызываем обновление дисплея
    UpdateDisplay();
}

void PitchPage::OnExitPage() {
  is_active_ = false;
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
  char note_str[3] = {note.base, '0' + note.octave, 0};
  
  // Подвигаем ноты ближе к нижней части региона
  uint32_t text_y = note_region_.h - 8; // 8 пикселей от низа
  
  if(note.sharp) {
    // Рисуем подчеркивание над текстом
    region_fill_part(&note_region_, 
                    (text_y - 2) * note_region_.w + x_offset, // На 2 пикселя выше текста
                    9, // Длина подчеркивания
                    0xFF); // Белый цвет
  }
  
  // Отрисовываем текст ноты
  region_string(&note_region_, note_str, x_offset, text_y, 0xf, 0x0, 0);
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
  char str_a[5];
  char str_b[5];
  
  // Форматируем строку для значения A
  if(footer_value_a_ == 0) {
    snprintf(str_a, sizeof(str_a), "A0");
  } else {
    snprintf(str_a, sizeof(str_a), "A%d%c", abs(footer_value_a_), 
             footer_value_a_ > 0 ? '+' : '-');
  }

  // Форматируем строку для значения B
  if(footer_value_b_ == 0) {
    snprintf(str_b, sizeof(str_b), "B0");
  } else {
    snprintf(str_b, sizeof(str_b), "B%d%c", abs(footer_value_b_),
             footer_value_b_ > 0 ? '+' : '-');
  }

  region_fill(&footer_region_, 0x00);
  DrawFooterSlider();
  region_string(&footer_region_, "SCALE", 52, 11, 0xf, 0x0, 0);
  region_string_font2(&footer_region_, str_a, 20, 4, 0xf, 0x0);
  region_string_font2(&footer_region_, str_b, 90, 4, 0xf, 0x0);
}

}  // namespace t8synth
