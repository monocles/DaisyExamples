#pragma once

#include "../drivers/display_controller.h"
#include "../dsp/voice.h"

namespace t8synth {

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
                 DisplayController* display) {
    for(size_t i = 0; i < NUM_VOICES; i++) {
        patches_[i] = patches[i];
        voices_[i] = voices[i];
    }
    modulations_ = mods;
    display_ = display;
  }

 protected:
  EditMode edit_mode_{EDIT_IDLE};
  int8_t active_control_{0};
  
  static constexpr size_t NUM_VOICES = 4;
  plaits::Patch* patches_[NUM_VOICES]{nullptr};
  plaits::Voice* voices_[NUM_VOICES]{nullptr};
  plaits::Modulations* modulations_{nullptr};
  DisplayController* display_{nullptr};
};

}  // namespace t8synth
