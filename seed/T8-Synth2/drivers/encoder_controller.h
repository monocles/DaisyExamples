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

    // Именованные константы для энкодеров
    enum EncoderIndex {
        // Группа A (0-3)
        ENC_1_A = 0,
        ENC_2_A = 1,
        ENC_3_A = 2,
        ENC_4_A = 3,
        // Группа B (4-7)
        ENC_1_B = 4,
        ENC_2_B = 5,
        ENC_3_B = 6,
        ENC_4_B = 7,
        // Служебные энкодеры (8-10)
        ENC_MOD_A = 8,
        ENC_MOD_B = 9,
        ENC_DATA = 10,
        // Маркер конца перечисления
        ENC_LAST
    };

    // Класс для доступа к энкодеру через operator[]
    class EncoderAccess {
    public:
        EncoderAccess(EncoderController& controller, uint8_t logical_index) 
            : controller_(controller), logical_index_(logical_index) {}

        bool just_pressed() const { 
            return controller_.just_pressed(controller_.MapLogicalToPhysical(logical_index_)); 
        }
        
        bool pressed() const { 
            return controller_.pressed(controller_.MapLogicalToPhysical(logical_index_)); 
        }
        
        bool released() const { 
            return controller_.released(controller_.MapLogicalToPhysical(logical_index_)); 
        }
        
        int32_t increment() const { 
            return controller_.increment(controller_.MapLogicalToPhysical(logical_index_)); 
        }
        
    private:
        EncoderController& controller_;
        uint8_t logical_index_;
    };

    // Операторы [] для доступа к энкодерам
    EncoderAccess operator[](uint8_t index) {
        return EncoderAccess(*this, index);
    }

    EncoderAccess operator[](EncoderIndex index) {
        return EncoderAccess(*this, static_cast<uint8_t>(index));
    }

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

    // Маппинг логического индекса в физический
    uint8_t MapLogicalToPhysical(uint8_t logical_index) const {
        if(logical_index <= 3) {
            return logical_index; // 0-3 -> 0-3 (ENC_1_A - ENC_4_A)
        }
        else if(logical_index <= 7) {
            return logical_index + 3; // 4-7 -> 7-10 (ENC_1_B - ENC_4_B)
        }
        else switch(logical_index) {
            case ENC_MOD_A: return 5;  // 8 -> 5
            case ENC_MOD_B: return 6;  // 9 -> 6
            case ENC_DATA:  return 4;  // 10 -> 4
            default: return logical_index;
        }
    }

    // Маппинг физического индекса в логический
    uint8_t MapPhysicalToLogical(uint8_t physical_index) const {
        if(physical_index <= 3) {
            return physical_index; // 0-3 -> 0-3 (ENC_1_A - ENC_4_A)
        }
        else if(physical_index >= 7 && physical_index <= 10) {
            return physical_index - 3; // 7-10 -> 4-7 (ENC_1_B - ENC_4_B)
        }
        else switch(physical_index) {
            case 5: return ENC_MOD_A;  // 5 -> 8
            case 6: return ENC_MOD_B;  // 6 -> 9
            case 4: return ENC_DATA;   // 4 -> 10
            default: return physical_index;
        }
    }
};
