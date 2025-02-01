#pragma once

#include "ui_page.h"
#include "../drivers/region.h"
#include "../drivers/encoder_controller.h" // Добавляем этот инклюд
// #include "../common_ui/modal_window.h"
#include "../common_ui/icons_renderer.h"
#include "../common_ui/sliders_renderer.h"
#include "../common_ui/footer_renderer.h" // Добавляем этот инклюд
// #include "../common_ui/notes_renderer.h"
#include "voice_manager.h"  // Add this include
#include "../globals.h"  // Добавляем include globals.h

namespace t8synth {

// Constants
// struct NoteRange {
//     static constexpr uint8_t MIN_OCTAVE = 2;
//     static constexpr uint8_t MAX_OCTAVE = 4;
//     static constexpr uint8_t NOTES_PER_OCTAVE = 12;
//     static constexpr uint8_t TOTAL_NOTES = (MAX_OCTAVE - MIN_OCTAVE + 1) * NOTES_PER_OCTAVE;
// };

// Удаляем дублирующуюся структуру DisplayLayout, так как она определена в ui_page.h

// struct Note {
//     char base;      // Базовая нота (C, D, E, F, G, A, B)
//     uint8_t octave; // Октава
//     bool sharp;     // Признак диеза
    
//     bool operator==(const Note& other) const {
//         return base == other.base && 
//                octave == other.octave && 
//                sharp == other.sharp;
//     }
    
//     bool operator!=(const Note& other) const {
//         return !(*this == other);
//     }
// };

class VolumePage : public UiPage {
public:
    VolumePage() = default;
    ~VolumePage() override = default;

    // Interface methods
    void OnInit() override;
    void OnEnterPage() override;
    void OnExitPage() override;
    void OnEncoder(uint8_t encoder, int32_t increment) override;
    void OnClick(uint8_t encoder) override;
    void OnLongClick(uint8_t encoder) override;
    void OnSwitch(uint8_t sw, bool pressed) override;
    void OnIdle() override;
    void UpdateDisplay() override;
    void OnSliderChanged(uint8_t position, uint8_t slider_id = 0) override {
        float normalized_volume = static_cast<float>(position) / 100.0f;
        
        // Каждый слайдер управляет своим голосом
        UpdateVoiceVolumeFromSlider(slider_id, normalized_volume);
        regions_.content.dirty = 1;
    }

private:
    // Группируем связанные данные в структуры
    struct PageState {
        bool is_active{false};
        bool needs_redraw{false};
        uint8_t current_encoder{0};
    } state_;

    // Группируем все регионы
    struct Regions {
        region header;
        region content;
        region footer;
    } regions_;

    // Группируем данные нот
    // struct NotesData {
    //     static constexpr uint8_t NOTES_PER_GROUP = 4;
    //     // Храним ноты как MIDI числа вместо структуры Note
    //     uint8_t left[NOTES_PER_GROUP];
    //     uint8_t right[NOTES_PER_GROUP];
    // } notes_;

    // Рендереры
    // ModalWindow modal_window_;
    FooterRenderer footer_renderer_;
    IconsRenderer icons_renderer_;
    SlidersRenderer sliders_renderer_;
    // NotesRenderer notes_renderer_;  // Добавляем рендерер нот

    // Методы для работы с регионами
    void InitRegions();
    void UpdateRegion(const region& reg);
    void MarkAllRegionsDirty(); // Add this declaration
    
    // Оптимизированные методы рисования
    void RenderContent();
    void RenderHeader();
    void RenderFooter();

    // // Notes management
    // static constexpr uint8_t MIN_OCTAVE = 2;
    // static constexpr uint8_t MAX_OCTAVE = 4;
    static constexpr uint8_t NOTES_PER_OCTAVE = 12;
    // static const char* const available_notes_[];

    // Drawing methods
    void DrawIcons();
    void DrawSliders();  // Add this declaration back

    // Modal methods
    // void ShowNoteModal(const Note& note);

    // Note handling
    // void UpdateNoteByEncoder(uint8_t encoder, int32_t increment);
    // void GetNoteFromIndex(uint8_t index, Note& note);
    // uint8_t GetIndexFromNote(const Note& note);
    // void UpdatePatchNote(const Note& note, plaits::Patch* patch);
    // void UpdateAllVoicePitches();

    // Drawing context
    struct DrawContext {
        bool is_active{false};
    } draw_context_;

    // Note management
    // void InitNotes();
    // void UpdateNote(uint8_t encoder_index, int32_t increment);

    // Удаляем неиспользуемые методы и структуру
    // void DrawFooterSlider();
    // void UpdateFooterValues();

    // Внутренние данные футера, включая кастомные названия
    struct Footer {
        int8_t value_a{0};
        int8_t value_b{0};
        const char* labelA{"A"};
        const char* labelB{"B"};
        const char* blockLabelA{"TUNE"};
        const char* blockLabelB{"TUNE"};
    } footer_;

    // Константы для MIDI нот
    // static constexpr uint8_t kMiddleC = 60;  // C4
    // static constexpr uint8_t kC2 = 36;       // C2
    // static constexpr uint8_t kC7 = 96;       // C7

    // Утилиты для работы с MIDI нотами
    // static Note MidiNoteToNote(uint8_t midi_note);
    // static uint8_t NoteToMidiNote(const Note& note);

    static constexpr float VOLUME_STEP = 0.02f;  // Шаг изменения громкости
    float encoder_volumes_[VoiceManager::NUM_VOICES]{0.0f};

    void UpdateVoiceVolume(uint8_t voice_index, int32_t increment);
    
    // Оставляем только одно определение SyncVolumes
    void SyncVolumes() {
        // Синхронизируем значения с текущими громкостями голосов
        for(size_t i = 0; i < VoiceManager::NUM_VOICES; i++) {
            encoder_volumes_[i] = voice_manager.GetVoice(i).volume;
            // Обновляем визуальное отображение всех слайдеров
            sliders_renderer_.SetSliderValue(i, encoder_volumes_[i] * 100);
            daisy::DaisySeed::PrintLine("Voice %d volume: %d%%", i, 
                static_cast<int>(encoder_volumes_[i] * 100.0f));
        }
        // Принудительно перерисовываем после обновления всех значений
        regions_.content.dirty = 1;
        RenderContent();
        display_->DrawRegion(regions_.content.x, regions_.content.y,
                           regions_.content.w, regions_.content.h,
                           regions_.content.data);
    }

    // Добавляем метод для обновления громкости от слайдера
    void UpdateVoiceVolumeFromSlider(uint8_t voice_index, float normalized_volume) {
        encoder_volumes_[voice_index] = normalized_volume;
        voice_manager.SetVoiceVolume(voice_index, 
            daisysp::fmap(normalized_volume, 0.0f, 1.0f, daisysp::Mapping::EXP));
        
        // Обновляем визуальное отображение слайдера
        sliders_renderer_.SetSliderValue(voice_index, normalized_volume * 100);
        
        // Помечаем область контента как требующую перерисовки
        regions_.content.dirty = 1;
        // Принудительно перерисовываем контент
        RenderContent();
        display_->DrawRegion(regions_.content.x, regions_.content.y,
                           regions_.content.w, regions_.content.h,
                           regions_.content.data);
        
        int volume_percent = static_cast<int>(normalized_volume * 100.0f);
        daisy::DaisySeed::PrintLine("Voice %d volume: %d%%", voice_index, volume_percent);
    }
};

} // namespace t8synth
