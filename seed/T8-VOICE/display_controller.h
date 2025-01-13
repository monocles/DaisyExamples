#pragma once
#include "daisy_seed.h"
#include "spi_manager.h"

using namespace daisy;

// Объявляем буферы как extern
extern DMA_BUFFER_MEM_SECTION uint8_t display_pixel_buffer[4096 * 2];
extern DMA_BUFFER_MEM_SECTION uint8_t display_cmd_buffer[32];
extern DMA_BUFFER_MEM_SECTION uint8_t display_data_buffer[32];

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
    void Update();

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

    // Убираем буферы из класса, используем глобальные
    // uint8_t display_buffer_[GRAM_BYTES * 2];  // Удаляем
    
    // Добавляем указатели на DMA буферы
    static uint8_t* const pixel_buffer_;  // = display_pixel_buffer
    static uint8_t* const cmd_buffer_;    // = display_cmd_buffer
    static uint8_t* const data_buffer_;   // = display_data_buffer

    // Pin definitions (убираем CS пин, т.к. он управляется через MultiSlaveSpiHandle)
    static constexpr Pin OLED_DC_PIN = Pin(PORTB, 7);
    static constexpr Pin OLED_RES_PIN = Pin(PORTG, 10);

    // Pin control
    dsy_gpio dc_pin_;
    dsy_gpio res_pin_;

    volatile bool is_busy_{false};

    // Буфер для команд дисплея
    struct DisplayCommand {
        uint8_t cmd;
        uint8_t* data;
        size_t size;
        bool is_command;
    };
    static constexpr size_t CMD_QUEUE_SIZE = 32;
    DisplayCommand cmd_queue_[CMD_QUEUE_SIZE];
    volatile size_t cmd_queue_head_{0};
    volatile size_t cmd_queue_tail_{0};
    
    void QueueCommand(uint8_t cmd);
    void QueueData(uint8_t* data, size_t size);
    void ProcessCommandQueue();
    
    static void DmaCompleteCallback(void* context, SpiHandle::Result result);
};
