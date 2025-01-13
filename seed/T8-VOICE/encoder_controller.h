#pragma once

#include "daisy_seed.h"

class EncoderController {
public:
    static constexpr int NUM_ENCODERS = 4;
    
    struct EncoderState {
        int32_t position;     
        bool button;    // Changed from bit field to bool
        bool changed;   // Changed from bit field to bool
    };

    void Init(daisy::DaisySeed& hw);
    void Update();
    const EncoderState& GetEncoder(uint8_t index) const { return encoders_[index]; }

    // Добавляем методы для управления чувствительностью
    void SetSensitivity(float scale) { sensitivity_ = scale; }
    float GetSensitivity() const { return sensitivity_; }

private:
    static constexpr uint8_t MCP23S17_ADDR = 0x40;
    static constexpr uint8_t IODIRA = 0x00;
    static constexpr uint8_t IODIRB = 0x01;
    static constexpr uint8_t GPIO_A = 0x12;  // Renamed from GPIOA
    static constexpr uint8_t GPIO_B = 0x13;  // Renamed from GPIOB
    static constexpr uint8_t GPPU_A = 0x0C;
    static constexpr uint8_t GPPU_B = 0x0D;

    // Lookup таблица для декодирования энкодера
    static constexpr int8_t ENCODER_LUT[4][4] = {
        {0, 1, -1, 0},
        {-1, 0, 0, 1},
        {1, 0, 0, -1},
        {0, -1, 1, 0}
    };

    void WriteReg(uint8_t reg, uint8_t value) __attribute__((always_inline));
    uint8_t ReadReg(uint8_t reg) __attribute__((always_inline));
    void UpdateEncoder(uint8_t index, uint8_t new_state) __attribute__((always_inline));
    void InitSPI();

    daisy::DaisySeed* seed_;
    daisy::SpiHandle spi_;
    EncoderState encoders_[NUM_ENCODERS];
    uint8_t last_state_[NUM_ENCODERS];  // Changed from last_states_ to last_state_
    dsy_gpio cs_pin_;

    // Добавляем переменную чувствительности
    float sensitivity_{1.0f};  // По умолчанию нормальная скорость
};
