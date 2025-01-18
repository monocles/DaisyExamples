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

  void SetContext(plaits::Patch* patch, 
                 plaits::Patch* patch2,
                 plaits::Patch* patch3,
                 plaits::Patch* patch4,
                 plaits::Voice* voice,
                 plaits::Voice* voice2,
                 plaits::Voice* voice3,
                 plaits::Voice* voice4,
                 plaits::Modulations* mods,
                 DisplayController* display) {
    patch_ = patch;
    patch2_ = patch2;
    patch3_ = patch3;
    patch4_ = patch4;
    voice_ = voice;
    voice2_ = voice2;
    voice3_ = voice3;
    voice4_ = voice4;
    modulations_ = mods;
    display_ = display;
  }

 protected:
  EditMode edit_mode_{EDIT_IDLE};
  int8_t active_control_{0};
  
  plaits::Patch* patch_{nullptr};
  plaits::Patch* patch2_{nullptr};
  plaits::Patch* patch3_{nullptr};
  plaits::Patch* patch4_{nullptr};
  plaits::Voice* voice_{nullptr};
  plaits::Voice* voice2_{nullptr};
  plaits::Voice* voice3_{nullptr};
  plaits::Voice* voice4_{nullptr};
  plaits::Modulations* modulations_{nullptr};
  DisplayController* display_{nullptr};
};

}  // namespace t8synth
