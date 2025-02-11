#include "ui.h"
#include "stmlib/system/system_clock.h"
#include "ui_pages/performance_page.h"
#include "ui_pages/pitch_page.h"

const int32_t kLongPressDuration = 1000;

void Ui::Init(EncoderController* encoders, DisplayController* display, 
              PotController* pots, VoiceManager* voices,
              plaits::Modulations* modulations) {
    encoders_ = encoders;
    display_ = display;
    pots_ = pots;
    voice_manager_ = voices;
    modulations_ = modulations;

    // Инициализируем массивы указателей на все 8 голосов
    for(size_t i = 0; i < VoiceManager::NUM_VOICES; i++) {
        auto& voice_unit = voices->GetVoice(i);
        patches_[i] = &voice_unit.patch;
        voices_[i] = &voice_unit.voice;
    }

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

  // Handle encoders using new mapping with ENC_LAST
  for(uint8_t i = 0; i < EncoderController::ENC_LAST; ++i) {
      if(auto increment = (*encoders_)[i].increment()) {
          queue_.AddEvent(CONTROL_ENCODER, i, increment);
      }

      // Handle encoder button states
      if((*encoders_)[i].just_pressed() && !encoder_pressed_[i]) {
          encoder_pressed_[i] = true;
          encoder_press_time_[i] = now;
          encoder_long_press_event_sent_[i] = false;
      } 
      else if((*encoders_)[i].pressed() && encoder_pressed_[i] && !encoder_long_press_event_sent_[i]) {
          if(now - encoder_press_time_[i] >= kLongPressDuration) {
              queue_.AddEvent(CONTROL_ENCODER_LONG_CLICK, i, 0);
              encoder_long_press_event_sent_[i] = true;
          }
      }
      else if((*encoders_)[i].released() && encoder_pressed_[i]) {
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

  // Удаляем обновление дисплея из Poll()
}

void Ui::ShowPage(UiPageNumber page) {
    if(current_page_) {
        pots_->Freeze();
        current_page_->OnExitPage();
    }
    
    FlushEvents();
    current_page_ = pages_[page];
    
    if(current_page_) {
        // Очищаем весь дисплей используя правильный метод
        // display_->ScreenSetRect(0, 0, 128, 64);
        // display_->ScreenClear();
        
        current_page_->OnEnterPage();
        // Обновляем дисплей сразу после входа на страницу
        current_page_->UpdateDisplay();
        
        pots_->Unfreeze();
    }
}

void Ui::HandlePageEvent(const Event& e) {
    if(!current_page_) return;
    
    switch(e.control_type) {
        case CONTROL_ENCODER:
            hw.PrintLine("Encoder %d: %d", e.control_id, e.data);
            current_page_->OnEncoder(e.control_id, e.data);
            break;
            
        case CONTROL_ENCODER_CLICK:
            hw.PrintLine("Encoder %d: Click", e.control_id);
            current_page_->OnClick(e.control_id);
            break;
            
        case CONTROL_ENCODER_LONG_CLICK:
            hw.PrintLine("Encoder %d: Long Click", e.control_id);
            current_page_->OnLongClick(e.control_id);
            break;
            
        case CONTROL_SWITCH:
            hw.PrintLine("Switch %d: %d", e.control_id, e.data);
            using BTN = EncoderController::ButtonIndex;
            if(e.control_id == BTN::VOICE_BUTTON) {
                ShowPage(PAGE_PERFORMANCE);
                return;
            }
            if(e.control_id == BTN::PITCH_BUTTON) {
                ShowPage(PAGE_PITCH);
                return;
            }
            if(e.control_id == BTN::CLEAR_BUTTON) {
                __disable_irq();
                daisy::System::ResetToBootloader(daisy::System::BootloaderMode::DAISY_INFINITE_TIMEOUT);
            }
            current_page_->OnSwitch(e.control_id, e.data > 0);
            break;

        // Добавляем обработку остальных событий
        case CONTROL_POT:
        case CONTROL_SWITCH_HOLD:
        case CONTROL_REFRESH:
            // Игнорируем эти события
            break;
    }
}

void Ui::DoEvents() {
    // static uint32_t next_display_update = 0;
    // uint32_t now = system_clock.milliseconds();

    while (queue_.available()) {
        HandlePageEvent(queue_.PullEvent());
    }

    if (queue_.idle_time() > 1000) {
        queue_.Touch();
        if(current_page_) {
            current_page_->OnIdle();
        }
    }

    // if(current_page_ && now >= next_display_update && pots_->IsFullySampled()) {
    //     current_page_->UpdateDisplay();
    //     next_display_update = now + DISPLAY_UPDATE_INTERVAL;
    //     System::DelayUs(50);
    // }
    if(current_page_ && pots_->IsFullySampled()) {
        current_page_->UpdateDisplay();
        // next_display_update = now + DISPLAY_UPDATE_INTERVAL;
        // System::DelayUs(50);
    }

}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::InitPages() {
    // Очищаем массив страниц
    for(int i = 0; i < PAGE_LAST; i++) {
        pages_[i] = nullptr;
    }

    // Статическое создание страниц
    static t8synth::PerformancePage performance_page;
    static t8synth::PitchPage pitch_page;

    // Настройка указателей на страницы
    pages_[PAGE_PERFORMANCE] = &performance_page;
    pages_[PAGE_PITCH] = &pitch_page;

    // Инициализация всех страниц
    for(int i = 0; i < PAGE_LAST; i++) {
        if(pages_[i]) {
            pages_[i]->SetContext(patches_, voices_, modulations_, display_);
            pages_[i]->OnInit();
        }
    }

    // Установка начальной страницы
    current_page_ = pages_[PAGE_PERFORMANCE];
    if(current_page_) {
        current_page_->OnEnterPage();
    }
}
