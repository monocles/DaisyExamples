#pragma once

#include "../drivers/display_controller.h"
#include "../dsp/voice.h"
#include "../drivers/pot_controller.h"  // Add this include

namespace t8synth {

// Перемещаем DisplayLayout в общий файл
struct DisplayLayout {
    static constexpr uint8_t SCREEN_WIDTH = 128;
    static constexpr uint8_t SCREEN_HEIGHT = 64;
    static constexpr uint8_t HEADER_HEIGHT = 9;
    static constexpr uint8_t CONTENT_HEIGHT = 36;
};

enum EditMode {
  EDIT_IDLE,
  EDIT_STARTED_BY_ENCODER,
  EDIT_STARTED_BY_POT
};

class UiPage {
 public:
  UiPage() {}
  virtual ~UiPage() {}
  
  virtual void OnInit() {
    edit_mode_ = EDIT_IDLE;
    active_control_ = 0;
  }
  
  virtual void OnEnterPage() = 0;
  virtual void OnExitPage() = 0;
  virtual void OnEncoder(uint8_t encoder, int32_t increment) = 0;
  virtual void OnClick(uint8_t encoder) {
    if (edit_mode_ != EDIT_IDLE) {
      edit_mode_ = EDIT_IDLE;
    } else {
      edit_mode_ = EDIT_STARTED_BY_ENCODER;
    }
  }
  virtual void OnLongClick(uint8_t encoder) = 0;
  virtual void OnSwitch(uint8_t sw, bool pressed) = 0;
  virtual void OnIdle() {
    if (edit_mode_ == EDIT_STARTED_BY_POT) {
      edit_mode_ = EDIT_IDLE;
    }
  }
  virtual void UpdateDisplay() = 0;

  void SetContext(plaits::Patch** patches,
                 plaits::Voice** voices,
                 plaits::Modulations* mods,
                 DisplayController* display,
                 PotController* pots) {  // Add pots parameter
    patches_ = patches;
    voices_ = voices;
    modulations_ = mods;
    display_ = display;
    pots_ = pots;  // Store pots controller
    NUM_VOICES = 8;
  }

 protected:
  EditMode edit_mode_{EDIT_IDLE};
  int8_t active_control_{0};
  
  plaits::Patch** patches_{nullptr};
  plaits::Voice** voices_{nullptr};
  plaits::Modulations* modulations_{nullptr};
  DisplayController* display_{nullptr};
  PotController* pots_{nullptr};  // Add pots member
  size_t NUM_VOICES{4};  // Делаем переменной
};

}  // namespace t8synth
