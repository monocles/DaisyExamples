#pragma once

#include "daisy_seed.h"

class EncoderController {
public:
    static constexpr int NUM_ENCODERS = 11;  // 3 (first MCP) + 4 (second MCP) + 4 (third MCP)
    static constexpr int NUM_BUTTONS = 10;   // 6 (second MCP) + 4 (third MCP)
    
    struct EncoderState {
        int32_t position;     
        uint8_t button_state; // 8-bit history of button states
    };

    void Init();
    void Update();
    const EncoderState& GetEncoder(uint8_t index) const { return encoders_[index]; }

    // Методы проверки состояний без дополнительной логики
    bool just_pressed(uint8_t index) const {
        return encoders_[index].button_state == 0x01;  // 00000001
    }
    
    bool pressed(uint8_t index) const {
        return encoders_[index].button_state == 0xFF;  // 11111111
    }
    
    bool released(uint8_t index) const {
        return encoders_[index].button_state == 0x00;  // 00000000
    }

    int32_t increment(uint8_t index) const {
        int32_t diff = encoders_[index].position - last_reported_positions_[index];
        if(diff != 0) {
            last_reported_positions_[index] = encoders_[index].position;
        }
        return diff;
    }

    // Метод для обновления только состояний энкодеров
    void UpdateEncoders();  // Чтение физических энкодеров (в Audio)
    
    // Метод для обновления только состояний кнопок
    void UpdateButtons();   // Обработка кнопок (в UI)

    void ProcessEncoders(); // Обработка изменений энкодеров (в UI)

    // Метод для проверки изменения состояния кнопки
    bool CheckButtonChanged(uint8_t index) {
        bool changed = button_changed_[index];
        button_changed_[index] = false;  // Сброс флага после проверки
        return changed;
    }

    // Добавляем методы для long press
    uint32_t press_time(uint8_t index) const {
        return press_time_[index];
    }

    void set_press_time(uint8_t index, uint32_t time) {
        press_time_[index] = time;
    }

    // Set sensitivity divisor (1 = normal, 2 = half speed, 3 = one-third speed, etc)
    void set_sensitivity(uint8_t divisor) {
        sensitivity_divisor_ = divisor > 0 ? divisor : 1;
    }

    uint8_t get_sensitivity() const {
        return sensitivity_divisor_;
    }

    // Метод для получения сырых данных (вызывается в аудио потоке)
    void UpdateHardware();  // Только SPI операции
    
    // Методы обработки данных (вызываются в UI потоке)
    void ProcessButtons();  // Обработка состояний кнопок

    bool GetButtonState(uint8_t button_index) const;  // Add declaration

private:
    static constexpr uint8_t MCP23S17_ADDR = 0x40;
    static constexpr uint8_t MCP_READ = 0x01;
    static constexpr uint8_t MCP_ADDR2 = 0x02;
    static constexpr uint8_t MCP_ADDR3 = 0x04;  // Address for third MCP
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

    void WriteReg(uint8_t reg, uint8_t value, int mcp_num);
    uint8_t ReadReg(uint8_t reg, int mcp_num);
    void InitSPI();

    void UpdateEncoderState(uint8_t index, uint8_t ab_state);
    void UpdateButtonState(uint8_t index, uint8_t button);

    // CS pins for both MCP23S17
    dsy_gpio cs_pin_1_;   // First MCP CS (PA4)
    dsy_gpio cs_pin_2_;   // Second MCP CS (PC1)
    dsy_gpio cs_pin_3_;   // Third MCP CS (PC4)

    daisy::SpiHandle spi_;
    EncoderState encoders_[NUM_ENCODERS];
    uint8_t last_state_[NUM_ENCODERS];  // Changed from last_states_ to last_state_
    dsy_gpio cs_pin_;

    // Добавляем массив для хранения последних отправленных позиций
    mutable int32_t last_reported_positions_[NUM_ENCODERS]{};

    mutable bool button_changed_[NUM_ENCODERS]{};  // Изменен на mutable для работы в const методах
    uint8_t current_button_states_;        // Текущие состояния кнопок

    mutable uint32_t press_time_[NUM_ENCODERS]{};  // Время начала нажатия для каждого энкодера

    // Временные буферы для накопления изменений
    mutable int32_t encoder_positions_[NUM_ENCODERS]{};

    // Unified structure for raw encoder data
    struct RawData {
        uint8_t ab_state : 2;
        uint8_t button : 1;
    };
    RawData raw_data_[NUM_ENCODERS];  // For all 11 encoders
    bool button_states_[NUM_BUTTONS];  // For all 10 buttons

    uint8_t sensitivity_divisor_{1};  // Default sensitivity divisor
    int8_t step_accumulator_[NUM_ENCODERS]{};  // Accumulated steps
};
