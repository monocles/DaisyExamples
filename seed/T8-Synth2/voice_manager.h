#pragma once
#include "dsp/voice.h"
#include "sys/system.h"
#include <array>

// Только объявляем буферы
extern char voice_buffers[8][16384];

class VoiceManager {
public:
    static constexpr size_t NUM_VOICES = 8;
        static constexpr size_t BUFFER_SIZE = 16384;
    
    struct VoiceUnit {
        plaits::Voice voice;
        plaits::Patch patch;
        char* buffer;
    };
    
    void Init() {
        for(size_t i = 0; i < NUM_VOICES; i++) {
            voices_[i].buffer = voice_buffers[i];
            stmlib::BufferAllocator allocator(voices_[i].buffer, BUFFER_SIZE);
            voices_[i].voice.Init(&allocator);
            voices_[i].patch.engine = 0;
        }
    }

    // Fix the pointer to member function syntax
    template<typename T>
    void SetCommonParameter(T plaits::Patch::* param, T value) {
        for(auto& voice : voices_) {
            voice.patch.*param = value;
        }
    }

    // Получить ссылку на голос по индексу
    VoiceUnit& GetVoice(size_t index) { 
        return voices_[index]; 
    }

    // Получить все голоса
    std::array<VoiceUnit, NUM_VOICES>& GetVoices() { 
        return voices_; 
    }

private:
    std::array<VoiceUnit, NUM_VOICES> voices_;
};
