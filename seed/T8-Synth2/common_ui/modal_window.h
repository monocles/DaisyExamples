#pragma once
#include "drivers/region.h"
#include "drivers/display_controller.h"

namespace t8synth {

enum class ModalType {
    PITCH_MODAL,
    // Будущие типы модальных окон
    VOLUME_MODAL,
    SETTINGS_MODAL
};

struct ModalConfig {
    static constexpr uint8_t WIDTH = 64;
    static constexpr uint8_t HEIGHT = 40;
    static constexpr uint32_t HIDE_DELAY_MS = 2000;
};

class ModalWindow {
public:
    void Init(DisplayController* display);
    void Show(ModalType type);
    void Hide();
    void Update(uint32_t current_time);
    bool IsVisible() const { return visible_; }
    
    // Методы для установки данных в зависимости от типа окна
    void SetPitchData(char base, uint8_t octave, bool sharp, uint8_t encoder_id);
    
    // Методы для отрисовки
    void Draw();
    region& GetRegion() { return modal_region_; }

    // Добавляем callback для уведомления о закрытии окна
    using OnCloseCallback = void(*)(void* context);
    void SetOnCloseCallback(OnCloseCallback callback, void* context) {
        on_close_ = callback;
        callback_context_ = context;
    }

private:
    void DrawFrame();
    void DrawPitchContent();
    
    DisplayController* display_{nullptr};
    region modal_region_;
    bool visible_{false};
    ModalType current_type_{ModalType::PITCH_MODAL};
    uint32_t last_update_time_{0};
    
    // Данные для PITCH_MODAL
    struct {
        char base;
        uint8_t octave;
        bool sharp;
        uint8_t encoder_id;
    } pitch_data_;

    OnCloseCallback on_close_{nullptr};
    void* callback_context_{nullptr};

    void NotifyClose() {
        if(on_close_) {
            on_close_(callback_context_);
        }
    }
};

} // namespace t8synth
