#include "ui.h"
#include "stmlib/system/system_clock.h"
#include "ui_pages/performance_page.h"

const int32_t kLongPressDuration = 1000;

void Ui::Init(EncoderController* encoders, DisplayController* display, 
              PotController* pots, plaits::Patch* patch, 
              plaits::Voice* voice, plaits::Modulations* modulations) {
  encoders_ = encoders;
  display_ = display;
  pots_ = pots;
  patch_ = patch;
  voice_ = voice;
  modulations_ = modulations;

  system_clock.Init();
  encoders_->set_sensitivity(2);
  queue_.Init();

  // Initialize input states
  for(int i = 0; i < EncoderController::NUM_ENCODERS; i++) {
      encoder_press_time_[i] = 0;
      encoder_long_press_event_sent_[i] = false;
      encoder_pressed_[i] = false;
  }

  for(int i = 0; i < EncoderController::NUM_BUTTONS; i++) {
      button_states_[i] = false;
      prev_button_states_[i] = false;
  }

  InitPages();
}

void Ui::Poll() {
  system_clock.Tick();
  uint32_t now = system_clock.milliseconds();
  ++sub_clock_;
  
  pots_->Update();
  encoders_->ProcessEncoders(); 
  encoders_->ProcessButtons();  

  // Handle encoders
  for(uint8_t i = 0; i < EncoderController::NUM_ENCODERS; ++i) {
      int32_t increment = encoders_->increment(i);
      if(increment != 0) {
          queue_.AddEvent(CONTROL_ENCODER, i, increment);
      }

      // Handle encoder button states
      if(encoders_->just_pressed(i) && !encoder_pressed_[i]) {
          encoder_pressed_[i] = true;
          encoder_press_time_[i] = now;
          encoder_long_press_event_sent_[i] = false;
      } 
      else if(encoders_->pressed(i) && encoder_pressed_[i] && !encoder_long_press_event_sent_[i]) {
          if(now - encoder_press_time_[i] >= kLongPressDuration) {
              queue_.AddEvent(CONTROL_ENCODER_LONG_CLICK, i, 0);
              encoder_long_press_event_sent_[i] = true;
          }
      }
      else if(encoders_->released(i) && encoder_pressed_[i]) {
          encoder_pressed_[i] = false;
          if(!encoder_long_press_event_sent_[i]) {
              queue_.AddEvent(CONTROL_ENCODER_CLICK, i, 0);
          }
      }
  }

  // Handle buttons
  for(uint8_t i = 0; i < EncoderController::NUM_BUTTONS; ++i) {
      button_states_[i] = encoders_->GetButtonState(i);
      if(button_states_[i] != prev_button_states_[i]) {
          queue_.AddEvent(CONTROL_SWITCH, i, button_states_[i] ? 1 : 0);
          prev_button_states_[i] = button_states_[i];
      }
  }

  // Update display at fixed interval
  if ((sub_clock_ & 1) == 0) {
    if(current_page_) {
      current_page_->UpdateDisplay();  // Let page handle its own display update
    }
  }
}

void Ui::ShowPage(UiPageNumber page) {
  if(current_page_) {
    current_page_->OnExitPage();
  }
  
  current_page_ = pages_[page];
  
  if(current_page_) {
    current_page_->OnEnterPage();
    current_page_->UpdateDisplay();
  }
}

void Ui::HandlePageEvent(const Event& e) {
  if(!current_page_) return;
  
  switch(e.control_type) {
    case CONTROL_ENCODER:
      current_page_->OnEncoder(e.control_id, e.data);
      break;
      
    case CONTROL_ENCODER_CLICK:
      current_page_->OnClick(e.control_id);
      break;
      
    case CONTROL_ENCODER_LONG_CLICK:
      current_page_->OnLongClick(e.control_id);
      break;
      
    case CONTROL_SWITCH:
    case CONTROL_SWITCH_HOLD:
      if(e.control_id == 1 && e.data > 0) {
        ShowPage(PAGE_PERFORMANCE);
        return;
      }
      current_page_->OnSwitch(e.control_id, e.data > 0);
      break;
  }
}

void Ui::DoEvents() {
  while (queue_.available()) {
    HandlePageEvent(queue_.PullEvent());
  }

  if (queue_.idle_time() > 1000) {
    queue_.Touch();
    if(current_page_) {
      current_page_->OnIdle();
    }
  }
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::InitPages() {
  pages_[PAGE_PERFORMANCE] = new t8synth::PerformancePage();
  // Initialize other pages when added
  
  current_page_ = pages_[PAGE_PERFORMANCE];
  if(current_page_) {
    current_page_->SetContext(patch_, voice_, modulations_, display_);
    current_page_->OnInit();
  }
}