#pragma once

#include "daisy_seed.h"
#include "stmlib/stmlib.h"
#include "stmlib/ui/event_queue.h"
#include "drivers/encoder_controller.h"
#include "drivers/display_controller.h"
#include "drivers/pot_controller.h"
#include "dsp/voice.h"
#include "ui_pages/ui_page.h"

extern daisy::DaisySeed hw;

using namespace stmlib;

// Типы страниц UI
enum UiPageNumber {
  PAGE_PERFORMANCE,
  PAGE_PITCH,  // Добавляем новую страницу
  PAGE_LAST
};

class Ui {
 public:
  Ui() {}
  ~Ui() = default;  // Используем = default вместо пустой реализации
  void Init(EncoderController* encoders, 
            DisplayController* display, 
            PotController* pots,
            plaits::Patch* patch, 
            plaits::Patch* patch2,
            plaits::Patch* patch3,
            plaits::Patch* patch4,
            plaits::Voice* voice,
            plaits::Voice* voice2,
            plaits::Voice* voice3,
            plaits::Voice* voice4,
            plaits::Modulations* modulations);
  void Poll();
  void DoEvents();
  void FlushEvents();

 private:
  void HandlePageEvent(const Event& e);
  void ShowPage(UiPageNumber page);
  void InitPages();

  EncoderController* encoders_;
  DisplayController* display_;
  PotController* pots_;
  
  EventQueue<16> queue_;
  uint8_t sub_clock_;
  
  plaits::Patch* patch_;
  plaits::Voice* voice_;
  plaits::Modulations* modulations_;

  plaits::Patch* patch2_{nullptr};
  plaits::Patch* patch3_{nullptr};
  plaits::Patch* patch4_{nullptr};
  plaits::Voice* voice2_{nullptr};
  plaits::Voice* voice3_{nullptr};
  plaits::Voice* voice4_{nullptr};

  // Button state tracking
  uint32_t encoder_press_time_[EncoderController::NUM_ENCODERS]{};
  bool encoder_long_press_event_sent_[EncoderController::NUM_ENCODERS]{};
  bool encoder_pressed_[EncoderController::NUM_ENCODERS]{};
  bool button_states_[EncoderController::NUM_BUTTONS]{};
  bool prev_button_states_[EncoderController::NUM_BUTTONS]{};

  // Page management  
  t8synth::UiPage* current_page_{nullptr};
  t8synth::UiPage* pages_[PAGE_LAST];

  // // Добавляем переменные для контроля частоты обновления дисплея
  // uint32_t last_display_update_{0};
  // static constexpr uint32_t DISPLAY_UPDATE_INTERVAL = 16; // ~60Hz
};