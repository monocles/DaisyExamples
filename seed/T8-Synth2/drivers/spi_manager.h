#pragma once
#include "daisy_seed.h"
#include "per/spiMultislave.h"

namespace daisy {

class SpiManager {
public:
    // Device identifiers
    static constexpr uint8_t DEVICE_DISPLAY = 0;
    static constexpr uint8_t DEVICE_ENCODER_1 = 1;
    static constexpr uint8_t DEVICE_ENCODER_2 = 2;
    static constexpr uint8_t DEVICE_ENCODER_3 = 3;
    static constexpr uint8_t MAX_DEVICES = 4;

    void Init(DaisySeed* seed) {
        seed_ = seed;
        InitSpi();
        InitCsPins();
    }

    SpiHandle::Result Transmit(uint8_t device, uint8_t* data, size_t size) {
        if(device >= MAX_DEVICES) return SpiHandle::Result::ERR;
        
        dsy_gpio_write(&cs_pins_[device], 0);  // Activate CS
        SpiHandle::Result result = spi_.BlockingTransmit(data, size);
        dsy_gpio_write(&cs_pins_[device], 1);  // Deactivate CS
        return result;
    }

    SpiHandle::Result TransmitAndReceive(uint8_t device, uint8_t* tx_data, uint8_t* rx_data, size_t size) {
        if(device >= MAX_DEVICES) return SpiHandle::Result::ERR;
        
        dsy_gpio_write(&cs_pins_[device], 0);  // Activate CS
        SpiHandle::Result result = spi_.BlockingTransmitAndReceive(tx_data, rx_data, size);
        dsy_gpio_write(&cs_pins_[device], 1);  // Deactivate CS
        return result;
    }

private:
    void InitSpi() {
        spi_config_.periph = SpiHandle::Config::Peripheral::SPI_6;
        spi_config_.mode = SpiHandle::Config::Mode::MASTER;
        spi_config_.direction = SpiHandle::Config::Direction::TWO_LINES;
        spi_config_.datasize = 8;
        spi_config_.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
        spi_config_.clock_phase = SpiHandle::Config::ClockPhase::ONE_EDGE;
        spi_config_.nss = SpiHandle::Config::NSS::SOFT;
        spi_config_.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_32;
        
        // Configure shared pins
        spi_config_.pin_config.mosi = {DSY_GPIOA, 7};  // PA7
        spi_config_.pin_config.miso = {DSY_GPIOA, 6};  // PA6
        spi_config_.pin_config.sclk = {DSY_GPIOA, 5};  // PA5

        if(spi_.Init(spi_config_) != SpiHandle::Result::OK) {
            seed_->PrintLine("SPI Init failed!");
        }
    }

    void InitCsPins() {
        // Configure CS pins for each device
        const dsy_gpio_pin cs_pin_configs[MAX_DEVICES] = {
            {DSY_GPIOB, 6},  // Display CS - PB6
            {DSY_GPIOA, 4},  // Encoder 1 CS - PA4
            {DSY_GPIOC, 1},  // Encoder 2 CS - PC1
            {DSY_GPIOC, 4}   // Encoder 3 CS - PC4
        };

        for(int i = 0; i < MAX_DEVICES; i++) {
            cs_pins_[i].pin = cs_pin_configs[i];
            cs_pins_[i].mode = DSY_GPIO_MODE_OUTPUT_PP;
            cs_pins_[i].pull = DSY_GPIO_NOPULL;
            dsy_gpio_init(&cs_pins_[i]);
            dsy_gpio_write(&cs_pins_[i], 1);  // Inactive high
        }
    }

    DaisySeed* seed_;
    SpiHandle spi_;
    SpiHandle::Config spi_config_;
    dsy_gpio cs_pins_[MAX_DEVICES];
};

} // namespace daisy
