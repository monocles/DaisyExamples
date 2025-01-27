#pragma once

#include "ui_page.h"
#include "pitch_page.h"

#include "../drivers/region.h"
#include "../drivers/encoder_controller.h" // Добавляем этот инклюд

namespace t8synth {

// Constants
// struct NoteRange {
//     static constexpr uint8_t MIN_OCTAVE = 2;
//     static constexpr uint8_t MAX_OCTAVE = 4;
//     static constexpr uint8_t NOTES_PER_OCTAVE = 12;
//     static constexpr uint8_t TOTAL_NOTES = (MAX_OCTAVE - MIN_OCTAVE + 1) * NOTES_PER_OCTAVE;
// };

// struct DisplayLayout {
//     static constexpr uint8_t SCREEN_WIDTH = 128;
//     static constexpr uint8_t SCREEN_HEIGHT = 64;
//     static constexpr uint8_t HEADER_HEIGHT = 9;
//     static constexpr uint8_t CONTENT_HEIGHT = 36;
// };

// struct ModalConfig {
//     static constexpr uint8_t WIDTH = 64;
//     static constexpr uint8_t HEIGHT = 40;
//     static constexpr uint32_t HIDE_DELAY_MS = 2000;
// };

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

private:
    // State
    bool is_active_{false};
    bool needs_redraw_{false};

    // Regions
    region header_region;
    region content_region;
    region footer_region_;
    region modal_region;

    // Modal state
    static constexpr uint8_t MODAL_WIDTH = 64;
    static constexpr uint8_t MODAL_HEIGHT = 40;
    static constexpr uint32_t MODAL_HIDE_DELAY_MS = 2000;
    
    bool modal_visible_{false};
    uint32_t last_note_change_time_{0};
    Note current_modal_note_{};

    // Notes management
    static constexpr uint8_t MIN_OCTAVE = 2;
    static constexpr uint8_t MAX_OCTAVE = 4;
    static constexpr uint8_t NOTES_PER_OCTAVE = 12;
    static const char* const available_notes_[];
    Note left_notes_[4];
    Note right_notes_[4];

    // Drawing methods
    void DrawSliders();
    void DrawSlider(uint32_t x_offset, uint32_t start_y);
    void DrawNotes();
    void RenderNote(const Note& note, uint32_t x_offset, uint32_t y_offset);
    void DrawFooterSlider();
    void UpdateFooterValues();
    void DrawIcons();  // Add new method declaration

    // Modal methods
    void ShowNoteModal(const Note& note);
    void UpdateModal();
    void DrawModal();
    void DrawModalFrame();
    void DrawModalContent();

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

    // Region management
    void InitRegions();
    void UpdateRegions();

    // Note management
    void InitNotes();
    void UpdateNote(uint8_t encoder_index, int32_t increment);

    // Footer management 
    struct Footer {
        int8_t value_a{0};
        int8_t value_b{0};
        void Update();
        void Draw(region& reg);
    } footer_;

    uint8_t current_encoder_{0};  // Добавьте это поле
};

} // namespace t8synth
