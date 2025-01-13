#pragma once
#include "daisy_seed.h"
#include "per/spiMultislave.h"

namespace daisy {

class SpiManager {
public:
    // Константы для идентификации устройств
    static constexpr uint8_t DEVICE_DISPLAY = 0;
    static constexpr uint8_t DEVICE_POTS = 1;
    int count = 0;
    void Init() {
        InitSpi();
    }

    SpiHandle::Result Transmit(uint8_t device, uint8_t* data, size_t size) {
        return spi_.BlockingTransmit(device, data, size); // Вернули передачу device=0
    }


private:
    void InitSpi() {
        spi_config_.periph = SpiHandle::Config::Peripheral::SPI_1;
        spi_config_.direction = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
        spi_config_.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
        spi_config_.clock_phase = SpiHandle::Config::ClockPhase::ONE_EDGE;
        spi_config_.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_2;
        spi_config_.datasize = 8;
        
        // Configure shared pins
        spi_config_.pin_config.mosi = {DSY_GPIOB, 5};
        spi_config_.pin_config.sclk = {DSY_GPIOG, 11};
        
        // Configure slave select pins for both devices
        spi_config_.pin_config.nss[DEVICE_DISPLAY].port = DSY_GPIOB;  // PB6 - Display NSS
        spi_config_.pin_config.nss[DEVICE_DISPLAY].pin = 6;  
        spi_config_.pin_config.nss[DEVICE_POTS].port = DSY_GPIOB;     // PB12 - Pots NSS
        spi_config_.pin_config.nss[DEVICE_POTS].pin = 12;
        
        spi_config_.num_devices = 2; // Display and Pots

        if(spi_.Init(spi_config_) != SpiHandle::Result::OK) {
            seed_->PrintLine("SPI Init failed!");
        }
    }

    DaisySeed* seed_;
    MultiSlaveSpiHandle spi_;
    MultiSlaveSpiHandle::Config spi_config_;
    // volatile uint8_t current_device_ = 0xFF;
};

} // namespace daisy
