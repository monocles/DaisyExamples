#include "encoder_controller.h"

// Define the static lookup table
constexpr int8_t EncoderController::ENCODER_LUT[4][4];

void EncoderController::Init() {

    // Initialize first CS pin (PA4)
    cs_pin_1_.pin = {DSY_GPIOA, 4};
    cs_pin_1_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    cs_pin_1_.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&cs_pin_1_);
    dsy_gpio_write(&cs_pin_1_, 1);

    // Initialize second CS pin (PC1)
    cs_pin_2_.pin = {DSY_GPIOC, 1};
    cs_pin_2_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    cs_pin_2_.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&cs_pin_2_);
    dsy_gpio_write(&cs_pin_2_, 1);

    // Initialize third CS pin (PC4)
    cs_pin_3_.pin = {DSY_GPIOC, 4};
    cs_pin_3_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    cs_pin_3_.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&cs_pin_3_);
    dsy_gpio_write(&cs_pin_3_, 1);

    // Initialize SPI6
    InitSPI();

    // Configure all three MCPs
    for (int i = 0; i < 3; i++) {
        WriteReg(IODIRA, 0xFF, i);
        WriteReg(IODIRB, 0xFF, i);
        WriteReg(GPPU_A, 0xFF, i);
        WriteReg(GPPU_B, 0xFF, i);
    }

    // Reset states
    for(int i = 0; i < NUM_ENCODERS; i++) {
        encoders_[i] = {0, 0x00};  // Initialize button state as released (all 0's)
        last_state_[i] = 0;  // Using last_state_ to match header file
        last_reported_positions_[i] = 0;
    }

    // Initialize step accumulators
    for(int i = 0; i < NUM_ENCODERS; i++) {
        step_accumulator_[i] = 0;
    }

    for(int i = 0; i < NUM_BUTTONS; i++) {
        button_states_[i] = false;
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
    // First MCP23S17 (encoders 0-3)
    uint8_t port_a = ReadReg(GPIO_A, 0);
    uint8_t port_b = ReadReg(GPIO_B, 0);
    
    // Сохраняем сырые данные
    raw_data_[0].ab_state = (port_b >> 0) & 0x3;
    raw_data_[0].button = (port_b & (1 << 2)) ? 0 : 1;
    
    raw_data_[1].ab_state = (port_b >> 3) & 0x3;
    raw_data_[1].button = (port_b & (1 << 5)) ? 0 : 1;
    
    raw_data_[2].ab_state = (port_b >> 6) & 0x3;
    raw_data_[2].button = (port_a & (1 << 0)) ? 0 : 1;
    
    raw_data_[3].ab_state = (port_a >> 1) & 0x3;
    raw_data_[3].button = (port_a & (1 << 3)) ? 0 : 1;

    // Second MCP23S17 (encoders 4-6 and buttons)
    uint8_t port_a2 = ReadReg(GPIO_A, 1);
    uint8_t port_b2 = ReadReg(GPIO_B, 1);

    // Encoder 5 (GPB0-2)
    raw_data_[4].ab_state = (port_b2 >> 0) & 0x3;
    raw_data_[4].button = (port_b2 & (1 << 2)) ? 0 : 1;

    // Encoder 6 (GPB6-7, GPA0)
    raw_data_[5].ab_state = (port_b2 >> 6) & 0x3;
    raw_data_[5].button = (port_a2 & (1 << 0)) ? 0 : 1;

    // Encoder 7 (GPA4-6)
    raw_data_[6].ab_state = (port_a2 >> 4) & 0x3;
    raw_data_[6].button = (port_a2 & (1 << 6)) ? 0 : 1;

    // Buttons 1-3 (GPB3-5)
    button_states_[0] = (port_b2 & (1 << 3)) ? 0 : 1;
    button_states_[1] = (port_b2 & (1 << 4)) ? 0 : 1;
    button_states_[2] = (port_b2 & (1 << 5)) ? 0 : 1;

    // Buttons 4-6 (GPA1-3)
    button_states_[3] = (port_a2 & (1 << 1)) ? 0 : 1;
    button_states_[4] = (port_a2 & (1 << 2)) ? 0 : 1;
    button_states_[5] = (port_a2 & (1 << 3)) ? 0 : 1;

    // Third MCP23S17 (encoders 7-10 and buttons 6-9)
    uint8_t port_a3 = ReadReg(GPIO_A, 2);
    uint8_t port_b3 = ReadReg(GPIO_B, 2);

    // Encoder A (GPB0-2)
    raw_data_[7].ab_state = (port_b3 >> 0) & 0x3;
    raw_data_[7].button = (port_b3 & (1 << 2)) ? 0 : 1;

    // Encoder B (GPB3-5)
    raw_data_[8].ab_state = (port_b3 >> 3) & 0x3;
    raw_data_[8].button = (port_b3 & (1 << 5)) ? 0 : 1;

    // Encoder C (GPB6-7, GPA0)
    raw_data_[9].ab_state = (port_b3 >> 6) & 0x3;
    raw_data_[9].button = (port_a3 & (1 << 0)) ? 0 : 1;

    // Encoder D (GPA1-3)
    raw_data_[10].ab_state = (port_a3 >> 1) & 0x3;
    raw_data_[10].button = (port_a3 & (1 << 3)) ? 0 : 1;

    // Additional buttons (GPA4-7)
    button_states_[6] = (port_a3 & (1 << 4)) ? 0 : 1;  // Button A
    button_states_[7] = (port_a3 & (1 << 5)) ? 0 : 1;  // Button B
    button_states_[8] = (port_a3 & (1 << 6)) ? 0 : 1;  // Button C
    button_states_[9] = (port_a3 & (1 << 7)) ? 0 : 1;  // Button D

    // Debug output for encoder states
    // for(int i = 7; i <= 10; i++) {
    //     hw.PrintLine("Encoder %d: ab=%d btn=%d", 
    //                 i, raw_data_[i].ab_state, raw_data_[i].button);
    // }
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

void EncoderController::WriteReg(uint8_t reg, uint8_t value, int mcp_num) {
    dsy_gpio* cs_pin;
    uint8_t addr = MCP23S17_ADDR;
    
    switch(mcp_num) {
        case 1: cs_pin = &cs_pin_2_; addr |= MCP_ADDR2; break;
        case 2: cs_pin = &cs_pin_3_; addr |= MCP_ADDR3; break;
        default: cs_pin = &cs_pin_1_; break;
    }

    uint8_t data[3] = {addr, reg, value};
    dsy_gpio_write(cs_pin, 0);
    spi_.BlockingTransmit(data, 3);
    dsy_gpio_write(cs_pin, 1);
}

uint8_t EncoderController::ReadReg(uint8_t reg, int mcp_num) {
    dsy_gpio* cs_pin;
    uint8_t addr = MCP23S17_ADDR | MCP_READ;
    
    switch(mcp_num) {
        case 1: cs_pin = &cs_pin_2_; addr |= MCP_ADDR2; break;
        case 2: cs_pin = &cs_pin_3_; addr |= MCP_ADDR3; break;
        default: cs_pin = &cs_pin_1_; break;
    }

    uint8_t tx_data[3] = {addr, reg, 0x00};
    uint8_t rx_data[3];
    
    dsy_gpio_write(cs_pin, 0);
    spi_.BlockingTransmitAndReceive(tx_data, rx_data, 3);
    dsy_gpio_write(cs_pin, 1);
    
    return rx_data[2];
}

bool EncoderController::GetButtonState(uint8_t button_index) const {
    if(button_index < NUM_BUTTONS) {
        return button_states_[button_index];
    }
    return false;
}
