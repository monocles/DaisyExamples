#pragma once
#include "daisy_seed.h"
#include "per/spiMultislave.h"
#include "spi_manager.h"

using namespace daisy;

// Move array definitions before the class
namespace {
    constexpr uint8_t ANALOG_POT_VALUES[] = {
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15,
        17, 18, 19, 20, 21, 22,
        24, 25, 26, 28, 29
    };

    constexpr uint8_t BUTTON_VALUES[] = {23, 27};
    constexpr uint8_t SWITCH_VALUES[] = {16};
    
    constexpr uint8_t MUX_VALUES[] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x08, 0x18, 0x28, 0x38, 0x48, 0x58, 0x68, 0x78
    };
    
    // Fix array size to match NUM_POTS (32)
    constexpr uint8_t POT_MAP_VALUES[32] = {
        4, 6, 7, 2, 3, 5, 1, 0,
        14, 15, 12, 13, 10, 11, 9, 8,
        22, 23, 20, 21, 18, 19, 17, 16,
        28, 29, 26, 24, 27, 25,
        0, 0  // Pad with zeros to match NUM_POTS
    };
}

class PotController {
public:
    // Используем uint_fast8_t для оптимальной производительности на ARM
    using index_t = uint_fast8_t;
    
    // Выравнивание данных для оптимального доступа
    static constexpr index_t NUM_POTS = 32;
    static constexpr index_t POTS_PER_BOARD = 16;

    static constexpr uint8_t NUM_ANALOG_POTS = 27;
    static constexpr const uint8_t* const ANALOG_POT_INDEXES = ANALOG_POT_VALUES;

    static constexpr uint8_t NUM_BUTTONS = 2;
    static constexpr const uint8_t* const BUTTON_INDEXES = BUTTON_VALUES;

    static constexpr uint8_t NUM_SWITCHES = 1;
    static constexpr const uint8_t* const SWITCH_INDEXES = SWITCH_VALUES;

    // Состояния
    static constexpr uint8_t STATE_IDLE = 0;
    static constexpr uint8_t STATE_SWITCHING = 1;
    static constexpr uint8_t STATE_STABILIZING = 2;
    static constexpr uint8_t STATE_READING = 3;

    // Предварительно рассчитанные значения для мультиплексоров
    static constexpr const uint8_t* const MUX_DATA = MUX_VALUES;

    // Маппинг потенциометров
    static constexpr const uint8_t* const POT_MAP = POT_MAP_VALUES;

    PotController() : current_channel_(0), state_(STATE_IDLE) {}
    
    void Init(DaisySeed& hw, SpiManager& spi_manager) {
        seed_ = &hw;
        spi_manager_ = &spi_manager;
        
        // Setup ADC with slower sampling for stability
        AdcChannelConfig adc_config;
        adc_config.InitSingle(Pin(PORTB, 1));
        adc_config.speed_ = AdcChannelConfig::SPEED_16CYCLES_5;
        seed_->adc.Init(&adc_config, 1);
        seed_->adc.Start();

        // Initialize state
        current_channel_ = 0;
        for(unsigned int i = 0; i < NUM_POTS; i++) {
            pot_values_[i] = 0.0f;
        }

        SelectMuxChannel(current_channel_);
        System::DelayUs(100); // Увеличенная задержка при инициализации
    }

    // Кэшируем часто используемые значения в регистрах
    inline void Update() {
        register const uint8_t curr_state = state_;
        register const uint8_t curr_channel = current_channel_;
        
        switch(curr_state) {
            case STATE_IDLE:
                state_ = STATE_SWITCHING;
                SelectMuxChannel(curr_channel);
                break;

            case STATE_SWITCHING:
                state_ = STATE_STABILIZING;
                break;

            case STATE_STABILIZING:
                state_ = STATE_READING;
                break;

            case STATE_READING:
                // Считываем с одного ADC пина для всех потенциометров
                float new_value = seed_->adc.GetFloat(0);
                float current = pot_values_[current_channel_];
                
                // Применяем только гистерезис
                if(fabsf(new_value - current) > HYSTERESIS_THRESHOLD) {
                    pot_values_[current_channel_] = new_value;
                }
                
                // Переходим к следующему каналу
                current_channel_ = (curr_channel + 1) & 0x1F; // % 32
                state_ = STATE_IDLE;
                break;
        }
    }

    // Оптимизация доступа к массивам через предварительное вычисление индексов
    inline float GetPotValue(index_t pot_idx) const {
        if(__builtin_expect(pot_idx >= NUM_ANALOG_POTS, 0)) { return 0.f; }
        
        const index_t analog_idx = ANALOG_POT_INDEXES[pot_idx];
        const index_t physical_idx = POT_MAP[analog_idx];
        const float raw_value = 1.f - pot_values_[physical_idx];
        
        // Применяем фильтр fonepole к значению
        float& smoothed = smoothed_values_[pot_idx];
        constexpr float kCoeff = 0.05f; // Коэффициент сглаживания (меньше = плавнее)
        smoothed += (raw_value - smoothed) * kCoeff;
        
        return (smoothed >= 0.995f) ? 1.f : smoothed;
    }

    inline bool GetButtonValue(uint8_t btn_idx) const {
        // Returns simple on/off for each button
        if(btn_idx >= NUM_BUTTONS) { return false; }
        uint8_t physical_index = POT_MAP[BUTTON_INDEXES[btn_idx]];
        float value = 1.f - pot_values_[physical_index];
        return value > 0.5f;
    }

    inline bool GetSwitchValue(uint8_t idx) const {
        // Switch is active above 1%
        if(idx >= NUM_SWITCHES) { return false; }
        uint8_t physical_index = POT_MAP[SWITCH_INDEXES[idx]];
        float value = 1.f - pot_values_[physical_index];
        return value > 0.01f;
    }

    inline float GetSwitchAnalogValue(uint8_t idx) const {
        if(idx >= NUM_SWITCHES) { return 0.f; }
        uint8_t physical_index = POT_MAP[SWITCH_INDEXES[idx]];
        float value = 1.f - pot_values_[physical_index];
        return value;
    }

    // Добавляем новый метод для проверки полного сканирования
    inline bool IsFullySampled() const {
        return current_channel_ == 0 && state_ == STATE_IDLE;
    }

private:
    static constexpr float HYSTERESIS_THRESHOLD = 0.002f;  // Порог гистерезиса 0.2%

    inline void SelectMuxChannel(uint8_t channel) {
        static constexpr uint8_t CHANNEL_MASK = 0x0F;
        static constexpr uint8_t DISABLE_MASK = 0x88;
        
        uint8_t mux_data[2];
        const uint8_t local_channel = channel & CHANNEL_MASK;
        
        mux_data[1] = (channel < POTS_PER_BOARD) ? MUX_DATA[local_channel] : DISABLE_MASK;
        mux_data[0] = (channel < POTS_PER_BOARD) ? DISABLE_MASK : MUX_DATA[local_channel];

        // Use pointer to SPI handle
        spi_manager_->Transmit(SpiManager::DEVICE_POTS, mux_data, 2);        
    }

    // Выравнивание данных для оптимального доступа к памяти
    DaisySeed* seed_;
    SpiManager* spi_manager_;
    float pot_values_[NUM_POTS];
    // Используем volatile для регистров состояния
    volatile uint8_t current_channel_;
    volatile uint8_t state_;
    mutable float smoothed_values_[NUM_ANALOG_POTS]{};  // Хранение сглаженных значений
};



