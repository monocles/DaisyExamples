#include "encoder_controller.h"

// Define the static lookup table
constexpr int8_t EncoderController::ENCODER_LUT[4][4];

void EncoderController::Init() {

    // Reset all encoder states
    for(int i = 0; i < NUM_ENCODERS; i++) {
        encoders_[i] = {0, 0x00};  // Initialize button state as released (all 0's)
        last_state_[i] = 0;  // Using last_state_ to match header file
        last_reported_positions_[i] = 0;
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

    // Initialize step accumulators
    for(int i = 0; i < NUM_ENCODERS; i++) {
        step_accumulator_[i] = 0;
    }
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
    UpdateHardware();
    ProcessEncoders();
    ProcessButtons();
}

void EncoderController::UpdateHardware() {
    // Только чтение данных по SPI
    register uint8_t port_a = ReadReg(GPIO_A);
    register uint8_t port_b = ReadReg(GPIO_B);
    
    // Сохраняем сырые данные
    raw_data_[0].ab_state = (port_b >> 0) & 0x3;
    raw_data_[0].button = (port_b & (1 << 2)) ? 0 : 1;
    
    raw_data_[1].ab_state = (port_b >> 3) & 0x3;
    raw_data_[1].button = (port_b & (1 << 5)) ? 0 : 1;
    
    raw_data_[2].ab_state = (port_b >> 6) & 0x3;
    raw_data_[2].button = (port_a & (1 << 0)) ? 0 : 1;
    
    raw_data_[3].ab_state = (port_a >> 1) & 0x3;
    raw_data_[3].button = (port_a & (1 << 3)) ? 0 : 1;
}

void EncoderController::ProcessEncoders() {
    for(uint8_t i = 0; i < NUM_ENCODERS; i++) {
        int8_t delta = ENCODER_LUT[last_state_[i]][raw_data_[i].ab_state];
        if(delta != 0 && last_state_[i] != raw_data_[i].ab_state) {
            step_accumulator_[i] += delta;
            
            // When we accumulate enough steps, update position
            if(abs(step_accumulator_[i]) >= sensitivity_divisor_) {
                encoders_[i].position += step_accumulator_[i] / sensitivity_divisor_;
                step_accumulator_[i] %= sensitivity_divisor_;
            }
        }
        last_state_[i] = raw_data_[i].ab_state;
    }
}

void EncoderController::ProcessButtons() {
    // Обработка кнопок
    for(uint8_t i = 0; i < NUM_ENCODERS; i++) {
        uint8_t prev_state = encoders_[i].button_state;
        encoders_[i].button_state = (prev_state << 1) | raw_data_[i].button;
    }
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
