#include "ui.h"
#include <algorithm>

// Размещаем строки меню в .rodata секции
const char* const Ui::menuItems[NUM_MENU_ITEMS] MENU_SECTION = {
    "ANALOG", "WSHAPE", "DUAL FM", "GRAINS",
    "HARM", "WTABLE", "CHORDS", "VOVEL",
    "CLOUD", "NOISE"
};

static constexpr uint32_t kLongPressDuration = 1000;

void Ui::Init(EncoderController* encoders, PotController* pots, DisplayController* display, DaisySeed* hw) {
    encoders_ = encoders;
    pots_ = pots;
    display_ = display;
    hw_ = hw;
    queue_.Init();
    
    // Устанавливаем начальную чувствительность энкодера
    SetEncoderSensitivity(0.2f); // Уменьшаем скорость в 5 раз
    
    // Инициализируем всё нулями за один раз
    memset(last_encoder_positions_, 0, sizeof(last_encoder_positions_));
    memset(button_states_, 0, sizeof(button_states_));
    memset(press_time_, 0, sizeof(press_time_));
    memset(last_pot_values_, 0, sizeof(last_pot_values_));
    
    // Возвращаем корректную инициализацию региона меню
    menuRegion.w = 64;
    menuRegion.h = 64;
    menuRegion.x = 0;
    menuRegion.y = 0;
    region_alloc(&menuRegion); // Возвращаем вызов region_alloc
    
    UpdateMenuDisplay();
    
}

void Ui::Poll() {
    const uint32_t now = System::GetNow();

    // Оптимизированный опрос энкодеров
    for(uint8_t i = 0; i < EncoderController::NUM_ENCODERS; ++i) {
        const auto& enc = encoders_->GetEncoder(i);
        
        if(enc.position != last_encoder_positions_[i]) {
            queue_.AddEvent(stmlib::CONTROL_ENCODER, i, enc.position);
        }

        if(enc.changed) {
            queue_.AddEvent(enc.button ? stmlib::CONTROL_ENCODER_CLICK : 
                                       stmlib::CONTROL_ENCODER_LONG_CLICK,
                           i, enc.button ? 0 : now - press_time_[i]);
            if(enc.button) press_time_[i] = now;
        }
    }

    // Оптимизированный опрос потенциометров
    for(uint8_t i = 0; i < PotController::NUM_ANALOG_POTS; ++i) {
        const float new_value = pots_->GetPotValue(i);
        const float diff = fabsf(new_value - last_pot_values_[i]);
        
        if(diff > POT_THRESHOLD) {
            queue_.AddEvent(stmlib::CONTROL_POT, i, 
                          static_cast<int32_t>(new_value * 4096.f));
            last_pot_values_[i] = new_value;
        }
    }
}

void Ui::DoEvents() {
    while(queue_.available()) {
        Event e = queue_.PullEvent();
        // hw_->PrintLine("Queue event: type=%d id=%d data=%ld", e.control_type, e.control_id, e.data);
        switch(e.control_type) {
            case stmlib::CONTROL_ENCODER:
                OnEncoderChanged(e);
                break;
            case stmlib::CONTROL_POT:
                OnPotChanged(e);
                break;
            case stmlib::CONTROL_ENCODER_CLICK:
                OnButtonPressed(e);
                break;
            case stmlib::CONTROL_ENCODER_LONG_CLICK:
                OnButtonReleased(e);
                break;
            default:
                break;
        }
    }
    if (queue_.idle_time() > 1000) {
        refresh_display_ = true;
        queue_.Flush();
    }
    if (refresh_display_) {
        queue_.Touch();
        // UpdateMenuDisplay();
    }
}

void Ui::OnEncoderChanged(const Event& e) {
    // hw_->PrintLine("OnEncoder event: id=%d pos=%ld", e.control_id, e.data);
    if(e.control_id == 0) { // First encoder
        // Calculate menu change based on encoder direction
        int32_t diff = e.data - last_encoder_positions_[0];
        // hw_->PrintLine("Diff calculated: %ld (curr=%ld last=%ld)", 
        //               diff, e.data, last_encoder_positions_[0]);
        
        if(diff != 0) {
            // Scale down the difference to make menu movement smoother
            diff = (diff > 0) ? 1 : -1;
            
            currentMenuItem += diff;
            if(currentMenuItem >= NUM_MENU_ITEMS) currentMenuItem = 0;
            if(currentMenuItem < 0) currentMenuItem = NUM_MENU_ITEMS - 1;
            
            // Обновляем engine в patch
            // if(patch_ != nullptr) {
            //     // patch_->engine = currentMenuItem;
            //     hw_->PrintLine("Engine changed to: %d", currentMenuItem);
            // }
            
            hw_->PrintLine("Menu changed: item=%d diff=%ld", currentMenuItem, diff);
            UpdateMenuDisplay();
        }
        last_encoder_positions_[0] = e.data;  // Обновляем позицию только здесь
    }
}

void Ui::OnPotChanged(const Event& e) {
    // Implement pot handling
}

void Ui::OnButtonPressed(const Event& e) {
    // Implement button press handling
}

void Ui::OnButtonReleased(const Event& e) {
    // Приводим типы к uint32_t для корректного сравнения
    if(static_cast<uint32_t>(e.data) >= kLongPressDuration) {
        // Handle long press
    } else {
        // Handle short press
    }
}

void Ui::UpdateMenuDisplay() {
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
    display_->DrawRegion(0, 0, menuRegion.w, menuRegion.h, menuRegion.data);
    refresh_display_ = false;
}

void Ui::SaveState() {
    // Implement state saving if needed
}
