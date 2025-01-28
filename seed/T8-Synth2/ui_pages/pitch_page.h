#pragma once

#include "ui_page.h"
#include "../drivers/region.h"
#include "../drivers/encoder_controller.h" // Добавляем этот инклюд
#include "../common_ui/modal_window.h"
#include "../common_ui/icons_renderer.h"
#include "../common_ui/sliders_renderer.h"
#include "../common_ui/footer_renderer.h" // Добавляем этот инклюд

namespace t8synth {

// Constants
struct NoteRange {
    static constexpr uint8_t MIN_OCTAVE = 2;
    static constexpr uint8_t MAX_OCTAVE = 4;
    static constexpr uint8_t NOTES_PER_OCTAVE = 12;
    static constexpr uint8_t TOTAL_NOTES = (MAX_OCTAVE - MIN_OCTAVE + 1) * NOTES_PER_OCTAVE;
};

struct DisplayLayout {
    static constexpr uint8_t SCREEN_WIDTH = 128;
    static constexpr uint8_t SCREEN_HEIGHT = 64;
    static constexpr uint8_t HEADER_HEIGHT = 9;
    static constexpr uint8_t CONTENT_HEIGHT = 36;
};

// Удаляем дублирующуюся структуру ModalConfig, так как она определена в modal_window.h

struct Note {
    char base;      // Базовая нота (C, D, E, F, G, A, B)
    uint8_t octave; // Октава
    bool sharp;     // Признак диеза
    
    bool operator==(const Note& other) const {
        return base == other.base && 
               octave == other.octave && 
               sharp == other.sharp;
    }
    
    bool operator!=(const Note& other) const {
        return !(*this == other);
    }
};

class PitchPage : public UiPage {
public:
    PitchPage() = default;
    ~PitchPage() override = default;

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
    struct NotesData {
        static constexpr uint8_t NOTES_PER_GROUP = 4;
        Note left[NOTES_PER_GROUP];
        Note right[NOTES_PER_GROUP];
    } notes_;

    // Рендереры
    ModalWindow modal_window_;
    FooterRenderer footer_renderer_;
    IconsRenderer icons_renderer_;
    SlidersRenderer sliders_renderer_;

    // Методы для работы с регионами
    void InitRegions();
    void UpdateRegion(const region& reg);
    void MarkAllRegionsDirty(); // Add this declaration
    
    // Оптимизированные методы рисования
    void RenderContent();
    void RenderHeader();
    void RenderFooter();

    // Notes management
    static constexpr uint8_t MIN_OCTAVE = 2;
    static constexpr uint8_t MAX_OCTAVE = 4;
    static constexpr uint8_t NOTES_PER_OCTAVE = 12;
    static const char* const available_notes_[];

    // Drawing methods
    void DrawNotes();
    void RenderNote(const Note& note, uint32_t x_offset, uint32_t y_offset);
    void DrawIcons();
    void DrawSliders();  // Add this declaration back

    // Modal methods
    void ShowNoteModal(const Note& note);

    // Note handling
    void UpdateNoteByEncoder(uint8_t encoder, int32_t increment);
    void GetNoteFromIndex(uint8_t index, Note& note);
    uint8_t GetIndexFromNote(const Note& note);
    void UpdatePatchNote(const Note& note, plaits::Patch* patch);
    void UpdateAllVoicePitches();

    // Drawing context
    struct DrawContext {
        bool is_active{false};
    } draw_context_;

    // Note management
    void InitNotes();
    void UpdateNote(uint8_t encoder_index, int32_t increment);

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
};

} // namespace t8synth
