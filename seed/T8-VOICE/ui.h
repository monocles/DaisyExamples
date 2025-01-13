#pragma once
#include <algorithm>  // для std::min/max
#include "daisy_seed.h"
#include "encoder_controller.h"
#include "pot_controller.h"
#include "display_controller.h"
#include "region.h"
#include "font.h"
#include "event_queue.h"
#include "plaits/dsp/voice.h"  // Добавляем включение заголовка plaits

// Используем static constexpr вместо #define для лучшей типобезопасности
static constexpr uint8_t QUEUE_SIZE = 16;
static constexpr uint8_t NUM_MENU_ITEMS = 10;
static constexpr uint8_t VISIBLE_ITEMS = 5;
static constexpr uint8_t ITEM_HEIGHT = 12;
static constexpr uint8_t MENU_HEIGHT = VISIBLE_ITEMS * ITEM_HEIGHT;
static constexpr float POT_THRESHOLD = 0.01f;

// Добавляем секцию для констант
#define MENU_SECTION __attribute__((section(".rodata")))

class Ui {
public:
    using ControlType = stmlib::ControlType;
    using Event = stmlib::Event;
    using EventQueue = stmlib::EventQueue<QUEUE_SIZE>;

    // Добавляем константный метод для проверки состояния
    [[nodiscard]] int8_t GetCurrentMenuItem() const { return currentMenuItem; }
    
    void Init(EncoderController* encoders, PotController* pots, DisplayController* display, DaisySeed* hw);
    void Poll();
    void DoEvents();
    void SaveState();

    // Добавляем метод для установки чувствительности энкодера
    void SetEncoderSensitivity(float scale) { 
        if(encoders_) encoders_->SetSensitivity(scale); 
    }

    // Добавляем метод установки patch
    void SetPatch(plaits::Patch* patch) { patch_ = patch; }

private:
    // Убираем always_inline, оставляем только hot для критичных методов
    void OnEncoderChanged(const Event& e);
    void OnPotChanged(const Event& e);
    void OnButtonPressed(const Event& e);
    void OnButtonReleased(const Event& e);
    void UpdateMenuDisplay() __attribute__((hot));

    // Размещаем строки меню в секции .rodata
    static const char* const menuItems[NUM_MENU_ITEMS] MENU_SECTION;
    
    // Определяем типы для единообразия
    using time_t = uint32_t;
    static constexpr time_t kLongPressDuration = 1000;

    // Оптимизируем размеры переменных для 32-битной архитектуры
    EncoderController* __attribute__((aligned(4))) encoders_;
    PotController* __attribute__((aligned(4))) pots_;
    DisplayController* __attribute__((aligned(4))) display_;
    DaisySeed* __attribute__((aligned(4))) hw_;
    EventQueue queue_;
    
    // Выравниваем данные по 4 байта для оптимальной производительности
    alignas(4) int8_t currentMenuItem{0};
    alignas(4) int16_t last_encoder_positions_[EncoderController::NUM_ENCODERS];
    alignas(4) uint8_t button_states_[PotController::NUM_BUTTONS];
    alignas(4) uint32_t press_time_[PotController::NUM_BUTTONS];
    bool refresh_display_ = false;
    
    // Убираем статический буфер дисплея, так как region_alloc сам выделяет память
    alignas(4) region menuRegion;
    alignas(4) float last_pot_values_[PotController::NUM_ANALOG_POTS];

    // Добавляем указатель на patch
    plaits::Patch* patch_{nullptr};
};
