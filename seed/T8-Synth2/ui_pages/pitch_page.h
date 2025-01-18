#pragma once

#include "ui_page.h"
#include "../drivers/region.h"

namespace t8synth {

class PitchPage : public UiPage {
 public:
  PitchPage() {}
  ~PitchPage() override {}

  void OnInit() override;
  void OnEnterPage() override;
  void OnExitPage() override;
  void OnEncoder(uint8_t encoder, int32_t increment) override;
  void OnClick(uint8_t encoder) override;
  void OnLongClick(uint8_t encoder) override;
  void OnSwitch(uint8_t sw, bool pressed) override;
  void OnIdle() override;
  void UpdateDisplay() override;
  
 private:
  bool is_active_{false};
  region display_region_;
  region value_region_;
  region pattern_region_; // Добавляем новый регион для шаблона
  region note_region_;  // Добавляем регион для отображения нот
  region footer_region_;  // Добавляем регион для футера
  bool needs_redraw_{false};
  
  static constexpr uint8_t DISPLAY_HEIGHT = 64;
  static constexpr uint8_t DISPLAY_WIDTH = 64;

  static constexpr uint8_t NUM_NOTES = 8;
  
  struct Note {
    char base;      // Базовая нота (C, D, E, F, G, A, B)
    uint8_t octave; // Октава (2-4)
    bool sharp;     // Признак диеза
  };

  static constexpr uint8_t NOTES_PER_OCTAVE = 12;
  static constexpr uint8_t MIN_OCTAVE = 2;
  static constexpr uint8_t MAX_OCTAVE = 4;
  
  Note left_notes_[4];   // Ноты для энкодеров 0-3
  Note right_notes_[4];  // Ноты для энкодеров 7-10

  static const char* const available_notes_[]; // Массив всех доступных нот
  void UpdateNoteByEncoder(uint8_t encoder, int32_t increment); // Метод изменения ноты энкодером

  void GetNoteFromIndex(uint8_t index, Note& note);  // Получение ноты по индексу
  uint8_t GetIndexFromNote(const Note& note);        // Получение индекса из ноты
  void RenderNote(const Note& note, uint32_t x_offset, uint32_t y_offset); // Отрисовка ноты с диезом

  void DrawPattern(); // Метод для отрисовки шаблона
  void DrawSinglePattern(uint32_t x_offset, uint32_t start_y); // Добавляем новый метод
  void DrawNotes();    // Метод для отрисовки нот
  void DrawFooterSlider(); // Метод для отрисовки слайдера

  int8_t footer_value_a_{0}; // Значение A от -12 до +12
  int8_t footer_value_b_{0}; // Значение B от -12 до +12
  void UpdateFooterValues(); // Метод для обновления значений в футере
};

}  // namespace t8synth
