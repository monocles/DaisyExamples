#pragma once
#include "dsp/voice.h"
#include "sys/system.h"
#include <array>
#include "daisysp.h"

// Только объявляем буферы
extern char voice_buffers[8][16384];
using namespace daisysp;

class VoiceManager {
public:
    static constexpr size_t NUM_VOICES = 8;
    static constexpr size_t BUFFER_SIZE = 16384;
    
    struct VoiceUnit {
        plaits::Voice voice;
        plaits::Patch patch;
        char* buffer;
        float volume{1.0f};        // целевое значение громкости
        float smooth_volume{1.0f}; // сглаженное значение громкости
    };
    
    static constexpr float VOLUME_SMOOTHING = 0.02f; // коэффициент сглаживания громкости

    void Init() {
        for(size_t i = 0; i < NUM_VOICES; i++) {
            voices_[i].buffer = voice_buffers[i];
            stmlib::BufferAllocator allocator(voices_[i].buffer, BUFFER_SIZE);
            voices_[i].voice.Init(&allocator);
            voices_[i].patch.engine = 0;
            voices_[i].volume = 0.0f; // Устанавливаем начальную громкость в 0
        }
    }

    // Fix the pointer to member function syntax
    template<typename T>
    void SetCommonParameter(T plaits::Patch::* param, T value) {
        for(auto& voice : voices_) {
            voice.patch.*param = value;
        }
    }

    // Add volume setter method
    void SetVoiceVolume(size_t index, float volume) {
        voices_[index].volume = fclamp(volume, 0.0f, 1.0f);
    }

    // Получить ссылку на голос по индексу
    VoiceUnit& GetVoice(size_t index) { 
        return voices_[index]; 
    }

    // Получить все голоса
    std::array<VoiceUnit, NUM_VOICES>& GetVoices() { 
        return voices_; 
    }

    // Добавляем сглаживание громкости
    static constexpr float SMOOTHING_FACTOR = 0.995f;
    float current_unity_gain_{1.0f};
    float target_unity_gain_{1.0f};
    
    void UpdateUnityGain() {
        // Подсчет активных голосов и их суммарной громкости
        float active_voices = 0.0f;
        float total_volume = 0.0f;
        
        for(const auto& voice : voices_) {
            if(voice.volume > 0.001f) {  // Порог для определения активного голоса
                active_voices += 1.0f;
                total_volume += voice.volume;
            }
        }
        
        // Вычисляем целевое значение unity gain
        if(active_voices > 0.0f) {
            target_unity_gain_ = 1.0f / sqrtf(active_voices);
        } else {
            target_unity_gain_ = 1.0f;
        }
        
        // Плавно приближаем текущее значение к целевому
        current_unity_gain_ = current_unity_gain_ * SMOOTHING_FACTOR + 
                            target_unity_gain_ * (1.0f - SMOOTHING_FACTOR);
    }
    
    float GetCurrentUnityGain() const { return current_unity_gain_; }

    void Process() {
        // Обновляем сглаженные значения громкости
        for(auto& voice : voices_) {
            fonepole(voice.smooth_volume, voice.volume, VOLUME_SMOOTHING);
        }
        UpdateUnityGain();
    }

private:
    std::array<VoiceUnit, NUM_VOICES> voices_;
};
