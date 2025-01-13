#pragma once

#include "daisy_seed.h"
#include "stmlib/stmlib.h"
#include "stmlib/ui/event_queue.h"
#include "drivers/encoder_controller.h"
#include "drivers/display_controller.h"
#include "drivers/pot_controller.h"
#include "drivers/region.h"
#include "drivers/font.h"
#include "dsp/voice.h"  // Добавляем включение для Patch

extern daisy::DaisySeed hw;

using namespace stmlib;

static constexpr uint8_t NUM_MENU_ITEMS = 10;
static constexpr uint8_t VISIBLE_ITEMS = 5;
static constexpr uint8_t ITEM_HEIGHT = 12;
static constexpr uint8_t MENU_HEIGHT = VISIBLE_ITEMS * ITEM_HEIGHT;


class Ui {
  public: 
    Ui() {}
    ~Ui() {}
		void Init(EncoderController* encoders, DisplayController* display, PotController* pots, plaits::Patch* patch, plaits::Voice* voice, plaits::Modulations* modulations);
		void Poll();
		void DoEvents();
		void FlushEvents();
    void SetBlinkSpeed(uint8_t speed_index);  // Добавляем объявление метода

	private:
		void OnSwitchPressed(const Event& e);
		void OnSwitchReleased(const Event& e);
		void OnEncoderIncrement(const Event& e);
		void UpdateMenuDisplay();

		EncoderController* encoders_;
		DisplayController* display_;
    	PotController* pots_;

		EventQueue<16> queue_;

    uint32_t encoder_press_time_[EncoderController::NUM_ENCODERS]{};
    bool encoder_long_press_event_sent_[EncoderController::NUM_ENCODERS]{};
    bool encoder_pressed_[EncoderController::NUM_ENCODERS]{};

		region menuRegion;
		int8_t currentMenuItem{0};
		static const char* const menuItems[NUM_MENU_ITEMS];

		uint8_t sub_clock_;

    	bool update_display_pending_{false};

    static constexpr uint8_t NUM_ENGINES = 3; // Количество доступных движков (0-2)

    plaits::Patch* patch_;  // Добавляем указатель на patch
    plaits::Voice* voice_;  // Добавляем указатель на voice
    plaits::Modulations* modulations_;  // Добавляем указатель на modulations

    bool button_states_[EncoderController::NUM_BUTTONS]{};  // Добавляем состояния кнопок
    bool prev_button_states_[EncoderController::NUM_BUTTONS]{};  // Предыдущие состояния

};