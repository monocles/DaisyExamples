#pragma once
#include "dsp/voice.h"
#include <array>

class VoiceManager {
public:
    static constexpr size_t NUM_VOICES = 4;
    
    struct VoiceUnit {
        plaits::Voice voice;
        plaits::Patch patch;
        char buffer[16384];
    };

    void Init() {
        for(size_t i = 0; i < NUM_VOICES; i++) {
            stmlib::BufferAllocator allocator(voices_[i].buffer, sizeof(VoiceUnit::buffer));
            voices_[i].voice.Init(&allocator);
            voices_[i].patch.engine = 2;
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
