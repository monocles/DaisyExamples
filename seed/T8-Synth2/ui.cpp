#include "ui.h"
#include "stmlib/system/system_clock.h"
#include "ui_pages/performance_page.h"
#include "ui_pages/pitch_page.h"

const int32_t kLongPressDuration = 1000;

void Ui::Init(EncoderController* encoders, DisplayController* display, 
              PotController* pots, plaits::Patch* patch,
              plaits::Patch* patch2, plaits::Patch* patch3, plaits::Patch* patch4,
              plaits::Voice* voice, plaits::Voice* voice2, 
              plaits::Voice* voice3, plaits::Voice* voice4,
              plaits::Modulations* modulations) {
    encoders_ = encoders;
    display_ = display;
    pots_ = pots;
    patch_ = patch;
    patch2_ = patch2;
    patch3_ = patch3;
    patch4_ = patch4;
    voice_ = voice;
    voice2_ = voice2;
    voice3_ = voice3;
    voice4_ = voice4;
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
            current_page_->OnEncoder(e.control_id, e.data);
            break;
            
        case CONTROL_ENCODER_CLICK:
            current_page_->OnClick(e.control_id);
            break;
            
        case CONTROL_ENCODER_LONG_CLICK:
            current_page_->OnLongClick(e.control_id);
            break;
            
        case CONTROL_SWITCH:
            if(e.control_id == 4) {
                ShowPage(PAGE_PITCH);
                return;
            }
            if(e.control_id == 5) {
                ShowPage(PAGE_PERFORMANCE);
                return;
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
            pages_[i]->SetContext(
                patch_, patch2_, patch3_, patch4_,  // Все патчи
                voice_, voice2_, voice3_, voice4_,  // Все голоса
                modulations_, display_);
            pages_[i]->OnInit();
        }
    }

    // Установка начальной страницы
    current_page_ = pages_[PAGE_PERFORMANCE];
    if(current_page_) {
        current_page_->OnEnterPage();
    }
}
