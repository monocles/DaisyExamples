#pragma once

#include "ui_page.h"
#include "../drivers/region.h"

namespace t8synth {

class PerformancePage : public UiPage {
 public:
  PerformancePage() {}
  ~PerformancePage() override {}

  void OnInit() override;
  void OnEnterPage() override;
  void OnExitPage() override;
  void OnEncoder(uint8_t encoder, int32_t increment) override;
  void OnClick(uint8_t encoder) override;
  void OnLongClick(uint8_t encoder) override;
  void OnSwitch(uint8_t sw, bool pressed) override;
  void OnIdle() override;
  void UpdateDisplay() override;
  
 private:
  int8_t current_engine_{0};
  int8_t current_menu_item_{0};
  bool is_active_{false};
  region menu_region_;
  bool needs_redraw_{false}; // Add flag for redraw request
  
  static constexpr uint8_t NUM_ENGINES = 3;  // Move NUM_ENGINES inside the class
  static constexpr uint8_t NUM_MENU_ITEMS = 10;
  static constexpr uint8_t VISIBLE_ITEMS = 5;
  static constexpr uint8_t ITEM_HEIGHT = 12;
  static constexpr uint8_t MENU_HEIGHT = VISIBLE_ITEMS * ITEM_HEIGHT;
  
  static const char* const menu_items_[NUM_MENU_ITEMS];
};

}  // namespace t8synth
