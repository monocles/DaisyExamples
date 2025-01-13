#pragma once
#include "daisy_seed.h"
#include "drivers/spi_manager.h"

using namespace daisy;

class DisplayController {
public:
    // Screen dimensions
    static constexpr uint8_t SCREEN_ROW_BYTES = 64;
    // static constexpr uint8_t SCREEN_ROW_BYTES_1 = 63;
    static constexpr uint8_t SCREEN_COL_BYTES = 64;
    // static constexpr uint8_t SCREEN_COL_BYTES_1 = 63;
    static constexpr uint8_t SCREEN_ROW_PX = 128;
    // static constexpr uint8_t SCREEN_ROW_PX_1 = 127;
    static constexpr uint8_t SCREEN_COL_PX = 64;
    // static constexpr uint8_t SCREEN_COL_PX_1 = 63;
    static constexpr uint32_t GRAM_BYTES = 4096;
    // static constexpr uint32_t GRAM_BYTES_1 = 4095;

    void Init(DaisySeed& hw, SpiManager& spi_manager);

    // Drawing functions
    void ScreenClear();
    void DrawRegion(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t* data);
    void DrawRegionOffset(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint32_t len, uint8_t* data, uint32_t off);
    void SetDirection(bool flipped);

private:
    void InitOled();
    void WriteCommand(uint8_t cmd);
    void WriteData(uint8_t data);
    void ScreenSetRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    void WriteScreenBuffer(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

    DaisySeed* seed_;
    SpiManager* spi_manager_;
    uint8_t screen_buffer_[GRAM_BYTES];
    bool is_screen_flipped_;
    bool is_new_revision_;

    // Pin definitions (убираем CS пин, т.к. он управляется через MultiSlaveSpiHandle)
    static constexpr Pin OLED_DC_PIN = Pin(PORTB, 7);
    static constexpr Pin OLED_RES_PIN = Pin(PORTG, 10);

    // Pin control
    dsy_gpio dc_pin_;
    dsy_gpio res_pin_;

    volatile bool is_busy_{false};

    uint8_t pixel_buffer_[GRAM_BYTES * 2];
};
