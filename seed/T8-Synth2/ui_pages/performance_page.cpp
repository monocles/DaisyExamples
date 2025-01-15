#include "performance_page.h"
#include "drivers/region.h"
#include "drivers/font.h"

namespace t8synth {

const char* const PerformancePage::menu_items_[NUM_MENU_ITEMS] = {
    "ANALOG", "WSHAPE", "DUAL FM", "GRAINS",
    "HARM", "WTABLE", "CHORDS", "VOVEL",
    "CLOUD", "NOISE"
};

void PerformancePage::OnInit() {
  current_engine_ = 0;
  current_menu_item_ = 0;
  
  // Initialize menu region
  menu_region_.w = 64;
  menu_region_.h = 64;
  menu_region_.x = 0;
  menu_region_.y = 0;
  region_alloc(&menu_region_);  // Remove check since function returns void
  UpdateDisplay();
  needs_redraw_ = true;
}

void PerformancePage::OnEncoder(uint8_t encoder, int32_t increment) {
  if(!is_active_) return;  // Only handle encoder when page is active
  
  switch(encoder) {
    case 0: // Menu scroll - only handle first encoder
      current_menu_item_ = (current_menu_item_ + increment);
      if(current_menu_item_ < 0) current_menu_item_ = 0;
      if(current_menu_item_ >= NUM_MENU_ITEMS) current_menu_item_ = NUM_MENU_ITEMS - 1;
      current_engine_ = current_menu_item_ % NUM_ENGINES;
      patch_->engine = current_engine_;
      needs_redraw_ = true;  // Request redraw instead of direct update
      break;
      
    default:
      break;  // Ignore other encoders
  }
}

void PerformancePage::OnSwitch(uint8_t sw, bool pressed) {
  if(sw == 0) { // Trigger button
    modulations_->trigger = pressed ? 1.0f : 0.0f;
  }
}

void PerformancePage::UpdateDisplay() {
  if(!is_active_ || !needs_redraw_) return;  // Only update if active and needs redraw
  
  memset(menu_region_.data, 0, menu_region_.len);
  
  const int8_t start_item = std::max(0,
      std::min(static_cast<int>(current_menu_item_ - (VISIBLE_ITEMS / 2)),
               static_cast<int>(NUM_MENU_ITEMS - VISIBLE_ITEMS)));

  static constexpr uint8_t EXTRA_SPACE = 8;
  int16_t y_offset = (menu_region_.h - MENU_HEIGHT) / 2;
  
  for(uint8_t i = 0; i < VISIBLE_ITEMS && (start_item + i) < NUM_MENU_ITEMS; ++i) {
    const uint8_t menu_index = start_item + i;
    
    int16_t item_y = y_offset;
    for(uint8_t j = 0; j < i; j++) {
      item_y += ITEM_HEIGHT;
      if(start_item + j == current_menu_item_) {
        item_y += EXTRA_SPACE;
      }
    }
    
    if(menu_index == current_menu_item_) {
      region_fill_part(&menu_region_, 
                       item_y * menu_region_.w,
                       16 * menu_region_.w,    
                       0xf);
      region_string(&menu_region_, 
                     menu_items_[menu_index], 
                     2, item_y, 0x0, 0xf, 1);
    } else {
      font_string_region_clip(&menu_region_, 
                              menu_items_[menu_index], 
                              2, item_y, 0xf, 0x0);
    }
  }
  
  menu_region_.dirty = 1;
  needs_redraw_ = false;  // Clear redraw flag
  display_->DrawRegion(0, 0, menu_region_.w, menu_region_.h, menu_region_.data);
}

void PerformancePage::OnEnterPage() {
  is_active_ = true;
  UpdateDisplay();
}

void PerformancePage::OnExitPage() {
  is_active_ = false;
//   region_free(&menu_region_);
}

void PerformancePage::OnClick(uint8_t encoder) {
  // Handle click events
}

void PerformancePage::OnLongClick(uint8_t encoder) {
  // Handle long click events
}

void PerformancePage::OnIdle() {
  // Handle idle state
}

}  // namespace t8synth
