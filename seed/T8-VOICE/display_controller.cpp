#include "display_controller.h"

// Определяем буферы один раз в cpp файле
DMA_BUFFER_MEM_SECTION uint8_t display_pixel_buffer[4096 * 2];
DMA_BUFFER_MEM_SECTION uint8_t display_cmd_buffer[32];
DMA_BUFFER_MEM_SECTION uint8_t display_data_buffer[32];

// Определяем статические указатели на буферы
uint8_t* const DisplayController::pixel_buffer_ = display_pixel_buffer;
uint8_t* const DisplayController::cmd_buffer_ = display_cmd_buffer;
uint8_t* const DisplayController::data_buffer_ = display_data_buffer;

void DisplayController::Init(DaisySeed& hw, SpiManager& spi_manager) {
    seed_ = &hw;
    spi_manager_ = &spi_manager;
    is_screen_flipped_ = false;
    
    // Initialize GPIO pins
    dc_pin_.pin = OLED_DC_PIN;
    dc_pin_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    dc_pin_.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&dc_pin_);

    res_pin_.pin = OLED_RES_PIN;
    res_pin_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    res_pin_.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&res_pin_);

    // Initialize OLED
    InitOled();
}

void DisplayController::WriteCommand(uint8_t cmd) {
    dsy_gpio_write(&dc_pin_, 0);                           // DC low for command
    cmd_buffer_[0] = cmd;
    spi_manager_->TransmitDMA(SpiManager::DEVICE_DISPLAY, cmd_buffer_, 1);
}

void DisplayController::WriteData(uint8_t data) {
    dsy_gpio_write(&dc_pin_, 1);                           // DC high for data
    spi_manager_->Transmit(SpiManager::DEVICE_DISPLAY, &data, 1);
}

void DisplayController::InitOled() {
    // Reset sequence
    dsy_gpio_write(&res_pin_, 1);
    System::Delay(1);
    dsy_gpio_write(&res_pin_, 0);
    System::Delay(1);
    dsy_gpio_write(&res_pin_, 1);
    System::Delay(10);

    // Initialize with new revision commands
    WriteCommand(0xAE); // off
    WriteCommand(0xB3); // clock rate
    WriteData(0x91);
    WriteCommand(0xCA); // mux ratio
    WriteData(0x3F);
    WriteCommand(0xA2); // set offset
    WriteData(0);
    WriteCommand(0xAB); // internal vdd reg
    WriteData(0x01);
    WriteCommand(0xA0); // remap
    WriteData(0x16);
    WriteData(0x11);
    WriteCommand(0xC7); // master contrast current
    WriteData(0x0f);
    WriteCommand(0xC1); // set contrast current
    WriteData(0x9F);
    WriteCommand(0xB1); // phase length
    WriteData(0xF2);
    WriteCommand(0xBB); // set pre-charge voltage
    WriteData(0x1F);
    WriteCommand(0xB4); // set vsl
    WriteData(0xA0);
    WriteData(0xFD);
    WriteCommand(0xBE); // set VCOMH
    WriteData(0x04);
    WriteCommand(0xA6); // set inverted display

    ScreenSetRect(0, 0, 128, 64);
    ScreenClear();
    WriteCommand(0xAF); // on
    System::Delay(10);
}

void DisplayController::ScreenSetRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    x = x + 28; // new revision column starts at 28

    // set column address
    WriteCommand(0x15);
    WriteData(x);
    WriteData(x + w - 1);

    // set row address
    WriteCommand(0x75);
    WriteData(y);
    WriteData(y + h - 1);
}

void DisplayController::ScreenClear() {
    // select chip for data
    dsy_gpio_write(&dc_pin_, 0);
    uint8_t cmd = 0x5C;  // команда start pixel data write to GDDRAM
    spi_manager_->Transmit(SpiManager::DEVICE_DISPLAY, &cmd, 1);

    // Clear screen buffer
    for(uint32_t i = 0; i < GRAM_BYTES; i++) { 
        screen_buffer_[i] = 0; 
    }
    
    // pull register select high to write data
    dsy_gpio_write(&dc_pin_, 1);
    
    uint8_t zero_data[2] = {0, 0};
    // Write zeros to display RAM (2 bytes per pixel)
    for(uint32_t i = 0; i < 8192; i++) {
        spi_manager_->Transmit(SpiManager::DEVICE_DISPLAY, zero_data, 2);
    }
}

void DisplayController::WriteScreenBuffer(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    // set drawing region
    ScreenSetRect(x, y, w, h);

    // start pixel data write to GDDRAM
    dsy_gpio_write(&dc_pin_, 0);
    uint8_t cmd = 0x5C;
    spi_manager_->Transmit(SpiManager::DEVICE_DISPLAY, &cmd, 1);

    // register select high for data
    dsy_gpio_write(&dc_pin_, 1);
    
    // Преобразуем все данные сразу в буфер
    uint32_t buffer_idx = 0;
    for(uint32_t i = 0; i < w*h; i++) {
        uint8_t a = screen_buffer_[i] & 0x0F;  // Младшая тетрада
        uint8_t b = screen_buffer_[i] & 0xF0;  // Старшая тетрада
        pixel_buffer_[buffer_idx++] = (a << 4) | a;  // Дублируем тетраду
        pixel_buffer_[buffer_idx++] = b | (b >> 4);  // Дублируем тетраду
    }

    // Используем обычную блокирующую передачу
    spi_manager_->Transmit(
        SpiManager::DEVICE_DISPLAY, 
        pixel_buffer_, 
        buffer_idx
    );
}

void DisplayController::DrawRegion(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t* data) {
    // 1 row address = 2 horizontal pixels
    // physical screen memory: 2px = 1byte
    w >>= 1;
    x >>= 1;
    uint32_t nb = w * h;
    
    #ifdef MOD_ALEPH // aleph screen is mounted upside down...
    uint8_t flip = 1;
    #else
    uint8_t flip = is_screen_flipped_;
    #endif
    
    uint8_t* pScr;
    
    if (flip) {
        pScr = screen_buffer_ + nb - 1;
        x = SCREEN_ROW_BYTES - x - w;
        y = SCREEN_COL_BYTES - y - h;
        // copy and pack into the screen buffer
        // 2 bytes input per 1 byte output
        for(uint32_t j = 0; j < h; j++) {
            for(uint32_t i = 0; i < w; i++) {
                *pScr = (0xf0 & ((*data) << 4));
                data++;
                *pScr |= ((*data) & 0xf);
                data++;
                pScr--;
            }
        }
    } else {
        pScr = screen_buffer_;
        // copy and pack into the screen buffer
        // 2 bytes input per 1 byte output
        for(uint32_t j = 0; j < h; j++) {
            for(uint32_t i = 0; i < w; i++) {
                *pScr = ((*data) & 0xf);
                data++;
                *pScr |= (0xf0 & ((*data) << 4));
                data++;
                pScr++;
            }
        }
    }

    WriteScreenBuffer(x, y, w, h);
}

void DisplayController::DrawRegionOffset(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint32_t len, uint8_t* data, uint32_t off) {
    // store region bounds for wrapping
    // inclusive lower bound
    uint8_t* const dataStart = data;
    // exclusive upper bound
    uint8_t* const dataEnd = data + len - 1;
    // begin at specified offset in region
    data += off;

    // 1 row address = 2 horizontal pixels
    // physical screen memory: 2px = 1byte
    w >>= 1;
    x >>= 1;
    uint32_t nb = len >> 1;

    uint8_t* pScr;
    #ifdef MOD_ALEPH
    uint8_t flip = 1;
    #else
    uint8_t flip = is_screen_flipped_;
    #endif

    if (flip) {
        pScr = screen_buffer_ + nb - 1;
        x = SCREEN_ROW_BYTES - x - w;
        y = SCREEN_COL_BYTES - y - h;
        // copy and pack into the screen buffer with wrapping
        for(uint32_t j = 0; j < h; j++) {
            for(uint32_t i = 0; i < w; i++) {
                *pScr = (0xf0 & ((*data) << 4));
                data++;
                if(data > dataEnd) { data = dataStart; }
                *pScr |= ((*data) & 0xf);
                data++;
                if(data > dataEnd) { data = dataStart; }
                pScr--;
            }
        }
    } else {
        pScr = screen_buffer_;
        // copy and pack into the screen buffer with wrapping
        for(uint32_t j = 0; j < h; j++) {
            for(uint32_t i = 0; i < w; i++) {
                *pScr = ((*data) & 0xf);
                data++;
                if(data > dataEnd) { data = dataStart; }
                *pScr |= (0xf0 & ((*data) << 4));
                data++;
                if(data > dataEnd) { data = dataStart; }
                pScr++;
            }
        }
    }

    WriteScreenBuffer(x, y, w, h);
}

void DisplayController::QueueCommand(uint8_t cmd) {
    if((cmd_queue_head_ + 1) % CMD_QUEUE_SIZE != cmd_queue_tail_) {
        cmd_queue_[cmd_queue_head_] = {cmd, nullptr, 1, true};
        cmd_queue_head_ = (cmd_queue_head_ + 1) % CMD_QUEUE_SIZE;
    }
}

void DisplayController::ProcessCommandQueue() {
    if(cmd_queue_head_ == cmd_queue_tail_ || is_busy_) return;
    
    auto& cmd = cmd_queue_[cmd_queue_tail_];
    is_busy_ = true;
    
    dsy_gpio_write(&dc_pin_, cmd.is_command ? 0 : 1);
    if(cmd.is_command) {
        pixel_buffer_[0] = cmd.cmd;
        spi_manager_->TransmitDMA(SpiManager::DEVICE_DISPLAY, 
            pixel_buffer_, 1, nullptr, DmaCompleteCallback, this);
    } else {
        spi_manager_->TransmitDMA(SpiManager::DEVICE_DISPLAY,
            cmd.data, cmd.size, nullptr, DmaCompleteCallback, this);
    }
}

void DisplayController::DmaCompleteCallback(void* context, SpiHandle::Result result) {
    auto display = static_cast<DisplayController*>(context);
    display->cmd_queue_tail_ = (display->cmd_queue_tail_ + 1) % CMD_QUEUE_SIZE;
    display->is_busy_ = false;
    display->ProcessCommandQueue();
}
