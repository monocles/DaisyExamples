#include "encoder_controller.h"

// Define the static lookup table
constexpr int8_t EncoderController::ENCODER_LUT[4][4];

void EncoderController::Init(daisy::DaisySeed& hw) {
    seed_ = &hw;

    // Reset all encoder states
    for(int i = 0; i < NUM_ENCODERS; i++) {
        encoders_[i] = {0, false, false};
        last_state_[i] = 0;  // Using last_state_ to match header file
    }

    // Initialize CS pin
    cs_pin_.pin = {DSY_GPIOA, 4};
    cs_pin_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    cs_pin_.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&cs_pin_);
    dsy_gpio_write(&cs_pin_, 1); // CS inactive high

    // Initialize SPI6
    InitSPI();

    // Configure MCP23S17
    WriteReg(IODIRA, 0xFF);
    WriteReg(IODIRB, 0xFF);
    WriteReg(GPPU_A, 0xFF);
    WriteReg(GPPU_B, 0xFF);
}

void EncoderController::InitSPI() {
    // Configure SPI6
    daisy::SpiHandle::Config spi_config;
    spi_config.periph = daisy::SpiHandle::Config::Peripheral::SPI_6;
    spi_config.mode = daisy::SpiHandle::Config::Mode::MASTER;
    spi_config.direction = daisy::SpiHandle::Config::Direction::TWO_LINES;
    spi_config.datasize = 8;
    spi_config.clock_polarity = daisy::SpiHandle::Config::ClockPolarity::LOW;
    spi_config.clock_phase = daisy::SpiHandle::Config::ClockPhase::ONE_EDGE;
    spi_config.nss = daisy::SpiHandle::Config::NSS::SOFT;
    spi_config.baud_prescaler = daisy::SpiHandle::Config::BaudPrescaler::PS_32;

    // Pin config for SPI6
    spi_config.pin_config.sclk = {DSY_GPIOA, 5};
    spi_config.pin_config.miso = {DSY_GPIOA, 6};
    spi_config.pin_config.mosi = {DSY_GPIOA, 7};

    spi_.Init(spi_config);
}

void EncoderController::Update() {
    // Читаем оба порта за один раз
    register uint8_t port_a = ReadReg(GPIO_A);  // Use renamed constant
    register uint8_t port_b = ReadReg(GPIO_B);  // Use renamed constant
    
    // Упаковываем A и B сигналы в один байт для каждого энкодера
    register uint8_t enc1_state = ((port_b >> 0) & 0x3) | ((port_b & (1 << 2)) ? 0 : 0x4);
    register uint8_t enc2_state = ((port_b >> 3) & 0x3) | ((port_b & (1 << 5)) ? 0 : 0x4);
    register uint8_t enc3_state = ((port_b >> 6) & 0x3) | ((port_a & (1 << 0)) ? 0 : 0x4);
    register uint8_t enc4_state = ((port_a >> 1) & 0x3) | ((port_a & (1 << 3)) ? 0 : 0x4);

    // Обновляем состояния всех энкодеров
    UpdateEncoder(0, enc1_state);
    UpdateEncoder(1, enc2_state);
    UpdateEncoder(2, enc3_state);
    UpdateEncoder(3, enc4_state);
}

inline void EncoderController::WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[3] = {MCP23S17_ADDR, reg, value};
    dsy_gpio_write(&cs_pin_, 0);
    spi_.BlockingTransmit(data, 3);
    dsy_gpio_write(&cs_pin_, 1);
}

inline uint8_t EncoderController::ReadReg(uint8_t reg) {
    uint8_t tx_data[3] = {MCP23S17_ADDR | 0x01, reg, 0x00};
    uint8_t rx_data[3];
    
    dsy_gpio_write(&cs_pin_, 0);
    spi_.BlockingTransmitAndReceive(tx_data, rx_data, 3);
    dsy_gpio_write(&cs_pin_, 1);
    
    return rx_data[2];
}

inline void EncoderController::UpdateEncoder(uint8_t index, uint8_t new_state) {
    EncoderState& enc = encoders_[index];
    
    // Обновляем состояние кнопки (3-й бит в new_state)
    uint8_t button = (new_state >> 2) & 0x1;
    enc.changed = (enc.button != button);
    enc.button = button;

    // Обновляем позицию используя lookup таблицу с масштабированием
    uint8_t ab_state = new_state & 0x3;
    int8_t delta = ENCODER_LUT[last_state_[index]][ab_state];
    
    // Накапливаем дельту с учетом чувствительности
    if(delta != 0) {
        static float accumulator = 0.0f;
        accumulator += delta * sensitivity_;
        
        // Применяем изменение только когда накопилось достаточно
        int32_t steps = static_cast<int32_t>(accumulator);
        if(steps != 0) {
            enc.position += steps;
            accumulator -= steps;
        }
    }
    
    last_state_[index] = ab_state;
}
