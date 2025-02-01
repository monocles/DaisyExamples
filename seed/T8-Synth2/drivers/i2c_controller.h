#pragma once
#include "daisy_seed.h"

using namespace daisy;

class I2CController {
public:
    struct CapSenseData {
        bool touch_status;   
        uint8_t position;    
        uint8_t slider_id;   // Добавляем идентификатор слайдера
        
        bool operator!=(const CapSenseData& other) const {
            return touch_status != other.touch_status || 
                   position != other.position ||
                   slider_id != other.slider_id;
        }
    };

    void Init(DaisySeed& hw) {
        hw_ = &hw;
        I2CHandle::Config i2c_config;
        i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
        i2c_config.pin_config.scl = {DSY_GPIOB, 8};
        i2c_config.pin_config.sda = {DSY_GPIOB, 9};
        i2c_config.speed = I2CHandle::Config::Speed::I2C_1MHZ;
        i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;
        
        auto result = i2c_.Init(i2c_config);
        hw_->PrintLine("I2C Init: %s", result == I2CHandle::Result::OK ? "OK" : "FAIL");

        // Выделяем буферы в DMA-совместимой памяти
        static uint8_t DMA_BUFFER_MEM_SECTION dma_buffer_1[2];
        static uint8_t DMA_BUFFER_MEM_SECTION dma_buffer_3[2];
        rx_buffer_1_ = dma_buffer_1;
        rx_buffer_3_ = dma_buffer_3;

        // Запускаем первые передачи
        StartNextTransfer(0);
        StartNextTransfer(2);
    }

    CapSenseData Process() {
        static CapSenseData last_data{false, 0, 0};
        CapSenseData data{false, 0, 0};

        // Проверяем первый слайдер (id=0)
        if(!transfer_in_progress_[0]) {
            data.touch_status = rx_buffer_1_[0];
            data.position = rx_buffer_1_[1];
            data.slider_id = 0;
            
            if(data != last_data) {
                last_data = data;
                StartNextTransfer(0);
                return data;
            }
            StartNextTransfer(0);
        }

        // Проверяем третий слайдер (id=2)
        if(!transfer_in_progress_[2]) {
            data.touch_status = rx_buffer_3_[0];
            data.position = rx_buffer_3_[1];
            data.slider_id = 2;
            
            if(data != last_data) {
                last_data = data;
                StartNextTransfer(2);
                return data;
            }
            StartNextTransfer(2);
        }

        return last_data;
    }

private:
    void StartNextTransfer(uint8_t slider_id) {
        transfer_in_progress_[slider_id] = true;
        
        if(slider_id == 0) {
            i2c_.ReceiveDma(kSlaveAddr1, rx_buffer_1_, 2, OnTransferComplete, &transfer_in_progress_[0]);
        }
        else if(slider_id == 2) {
            i2c_.ReceiveDma(kSlaveAddr3, rx_buffer_3_, 2, OnTransferComplete, &transfer_in_progress_[2]);
        }
    }

    static void OnTransferComplete(void* context, I2CHandle::Result result) {
        bool* in_progress = static_cast<bool*>(context);
        *in_progress = false;
    }

    static constexpr uint8_t kSlaveAddr1 = 8;   // Первый слайдер
    static constexpr uint8_t kSlaveAddr3 = 10;  // Третий слайдер
    
    DaisySeed* hw_{nullptr};
    I2CHandle i2c_;
    uint8_t* rx_buffer_1_{nullptr};  // Буфер для первого слайдера
    uint8_t* rx_buffer_3_{nullptr};  // Буфер для третьего слайдера
    bool transfer_in_progress_[8]{false};  // Статус передачи для каждого слайдера
};
