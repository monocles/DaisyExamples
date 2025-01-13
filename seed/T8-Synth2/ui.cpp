#include "ui.h"
#include "stmlib/system/system_clock.h"

const int32_t kLongPressDuration = 1000;

const char* const Ui::menuItems[NUM_MENU_ITEMS] = {
    "ANALOG", "WSHAPE", "DUAL FM", "GRAINS",
    "HARM", "WTABLE", "CHORDS", "VOVEL",
    "CLOUD", "NOISE"
};


void Ui::Init(EncoderController* encoders, DisplayController* display, PotController* pots, plaits::Patch* patch, plaits::Voice* voice, plaits::Modulations* modulations) {
  encoders_ = encoders;
  display_ = display;
  pots_ = pots;
  patch_ = patch;
  voice_ = voice;
  modulations_ = modulations;

  system_clock.Init();
  encoders_->set_sensitivity(2);
  queue_.Init();

  // Initialize press states
  for(int i = 0; i < EncoderController::NUM_ENCODERS; i++) {
      encoder_press_time_[i] = 0;
      encoder_long_press_event_sent_[i] = false;
      encoder_pressed_[i] = false;
  }

  // Initialize button states
  for(int i = 0; i < EncoderController::NUM_BUTTONS; i++) {
      button_states_[i] = false;
      prev_button_states_[i] = false;
  }

  // Возвращаем корректную инициализацию региона меню
  menuRegion.w = 64;
  menuRegion.h = 64;
  menuRegion.x = 0;
  menuRegion.y = 0;
  region_alloc(&menuRegion); // Возвращаем вызов region_alloc
  UpdateMenuDisplay();

  update_display_pending_ = true;

  hw.PrintLine("UI Init done");
}

void Ui::Poll() {
    system_clock.Tick();
    uint32_t now = system_clock.milliseconds();
    ++sub_clock_;
    pots_->Update();
    encoders_->ProcessEncoders(); 
    encoders_->ProcessButtons();  
  
    // Обрабатываем все 8 энкодеров вместо 4
    for(uint8_t i = 0; i < EncoderController::NUM_ENCODERS; ++i) {
        // Получаем изменения на частоте UI
        int32_t increment = encoders_->increment(i);
        if(increment != 0) {
            queue_.AddEvent(CONTROL_ENCODER, i, increment);
        }

        // Обработка кнопок с учетом предыдущего состояния
        if(encoders_->just_pressed(i) && !encoder_pressed_[i]) {
            encoder_pressed_[i] = true;
            encoder_press_time_[i] = now;
            encoder_long_press_event_sent_[i] = false;
        } 
        else if(encoders_->pressed(i) && encoder_pressed_[i] && !encoder_long_press_event_sent_[i]) {
            uint32_t duration = now - encoder_press_time_[i];
            if(duration >= kLongPressDuration) {
                queue_.AddEvent(CONTROL_ENCODER_LONG_CLICK, i, duration);
                encoder_long_press_event_sent_[i] = true;
            }
        }
        else if(encoders_->released(i) && encoder_pressed_[i]) {
            encoder_pressed_[i] = false;
            if(!encoder_long_press_event_sent_[i]) {
                queue_.AddEvent(CONTROL_ENCODER_CLICK, i, 0);
            }
            encoder_long_press_event_sent_[i] = false;
        }
    }

    // Обработка дополнительных кнопок
    for(uint8_t i = 0; i < EncoderController::NUM_BUTTONS; ++i) {
        button_states_[i] = encoders_->GetButtonState(i);
        
        // Проверяем изменение состояния
        if(button_states_[i] != prev_button_states_[i]) {
            if(button_states_[i]) {
                // Кнопка нажата
                queue_.AddEvent(CONTROL_SWITCH, i, 1);
            } else {
                // Кнопка отпущена
                queue_.AddEvent(CONTROL_SWITCH, i, 0);
            }
            prev_button_states_[i] = button_states_[i];
        }
    }

    if ((sub_clock_ & 1) == 0 && update_display_pending_) {
        display_->DrawRegion(0, 0, menuRegion.w, menuRegion.h, menuRegion.data);
        update_display_pending_ = false;
    }
}

void Ui::UpdateMenuDisplay() {
    // Проверяем завершение сканирования потенциометров
    // while (!pots_->IsFullySampled());
    
    memset(menuRegion.data, 0, menuRegion.len);
    
    const int8_t startItem = std::max(0,
        std::min(static_cast<int>(currentMenuItem - (VISIBLE_ITEMS / 2)),
                 static_cast<int>(NUM_MENU_ITEMS - VISIBLE_ITEMS)));

    // Добавляем константу для дополнительного отступа
    static constexpr uint8_t EXTRA_SPACE = 8;
    int16_t yOffset = (menuRegion.h - MENU_HEIGHT) / 2;
    
    for(uint8_t i = 0; i < VISIBLE_ITEMS && (startItem + i) < NUM_MENU_ITEMS; ++i) {
        const uint8_t menuIndex = startItem + i;
        
        // Рассчитываем позицию Y с учетом дополнительного отступа после активного элемента
        int16_t itemY = yOffset;
        for(uint8_t j = 0; j < i; j++) {
            itemY += ITEM_HEIGHT;
            // Добавляем отступ после активного элемента
            if(startItem + j == currentMenuItem) {
                itemY += EXTRA_SPACE;
            }
        }
        
        if(menuIndex == currentMenuItem) {
            region_fill_part(&menuRegion, 
                           itemY * menuRegion.w,
                           16 * menuRegion.w,    
                           0xf);
            region_string(&menuRegion, 
                         menuItems[menuIndex], 
                         2, itemY, 0x0, 0xf, 1);
        } else {
            font_string_region_clip(&menuRegion, 
                                  menuItems[menuIndex], 
                                  2, itemY, 0xf, 0x0);
        }
    }
    
    menuRegion.dirty = 1;
    update_display_pending_ = true;  // Помечаем что нужно обновить дисплей
    // display_->DrawRegion(0, 0, menuRegion.w, menuRegion.h, menuRegion.data);
    // refresh_display_ = false;
}

void Ui::OnEncoderIncrement(const Event& e) {
  int32_t diff = e.data;
  if(diff > 0) {
      currentMenuItem = (currentMenuItem + 1) % NUM_MENU_ITEMS;
      patch_->engine = currentMenuItem % NUM_ENGINES;
  } else {
      currentMenuItem = (currentMenuItem - 1 + NUM_MENU_ITEMS) % NUM_MENU_ITEMS;
  }

  UpdateMenuDisplay();
}

void Ui::DoEvents() {
  // bool refresh_display_ = false;
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    if (e.control_type == CONTROL_ENCODER_CLICK) {
      // OnClick();
      hw.PrintLine("Encoder %d: Click!", e.control_id);
    } else if (e.control_type == CONTROL_ENCODER_LONG_CLICK) {
      hw.PrintLine("Encoder %d: Long click!", e.control_id);
      // OnLongClick();
    } else if (e.control_type == CONTROL_ENCODER) {
      OnEncoderIncrement(e);
      hw.PrintLine("Encoder %d: value %d", e.control_id, e.data);
    } else if (e.control_type == CONTROL_SWITCH) {
      if(e.control_id == 0) { // Если это кнопка 0
        modulations_->trigger = e.data ? 1.0f : 0.0f;  // 1.0f при нажатии, 0.0f при отпускании
      }
      hw.PrintLine("Button %d: %s", e.control_id, e.data ? "pressed" : "released");
      // Здесь можно добавить специфическую обработку нажатий кнопок
      switch(e.control_id) {
          case 0:
              // Обработка первой кнопки
              break;
          case 1:
              // Обработка второй кнопки
              break;
          // ... и так далее для остальных кнопок
      }
    }
    hw.PrintLine("Free queue");
    // refresh_display_ = true;
  }

  if (queue_.idle_time() > 1000) {
    queue_.Touch();
  }

//   if ((sub_clock_ & 0xFF) == 0 ) {
//     display_->DrawRegion(0, 0, menuRegion.w, menuRegion.h, menuRegion.data);
//     update_display_pending_ = false;
//   }

}

void Ui::FlushEvents() {
  queue_.Flush();
}