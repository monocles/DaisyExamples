#pragma once

#include "daisy_seed.h"
#include "stmlib/stmlib.h"
#include "stmlib/ui/event_queue.h"
#include "drivers/encoder_controller.h"
#include "drivers/display_controller.h"
#include "drivers/region.h"
#include "drivers/font.h"

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
		void Init(EncoderController* encoders, DisplayController* display);
		void Poll();
		void DoEvents();
		void FlushEvents();

	private:
		void OnSwitchPressed(const Event& e);
		void OnSwitchReleased(const Event& e);
		void OnEncoderIncrement(const Event& e);
		void UpdateMenuDisplay();

		EncoderController* encoders_;
		DisplayController* display_;

		EventQueue<16> queue_;

    uint32_t encoder_press_time_[EncoderController::NUM_ENCODERS];
    bool encoder_long_press_event_sent_[EncoderController::NUM_ENCODERS];
    bool encoder_pressed_[EncoderController::NUM_ENCODERS];  // Добавляем флаг текущего состояния

		region menuRegion;
		int8_t currentMenuItem{0};
		static const char* const menuItems[NUM_MENU_ITEMS];
};