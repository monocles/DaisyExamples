#include "daisysp.h"
#include "pitch_page.h"
#include "drivers/region.h"
#include "drivers/font.h"
#include "stmlib/system/system_clock.h"

namespace t8synth {
using namespace daisysp;
using namespace stmlib;

// Определяем массив всех доступных нот
const char* const PitchPage::available_notes_[] = {
    "C2", "D2", "E2", "F2", "G2", "A2", "B2", 
    "C3", "D3", "E3", "F3", "G3", "A3", "B3",
    "C4", "D4", "E4", "F4", "G4", "A4", "B4"
};

void PitchPage::OnInit() {
    state_.is_active = true;
    InitRegions();
    InitNotes();
    
    modal_window_.Init(display_);
    modal_window_.SetOnCloseCallback([](void* context) {
        auto page = static_cast<PitchPage*>(context);
        page->MarkAllRegionsDirty();
    }, this);
    
    icons_renderer_.Init(&regions_.content);
    sliders_renderer_.Init(&regions_.content);
    
    for(uint8_t i = 0; i < IconsRenderer::NUM_ICONS; i++) {
        icons_renderer_.SetGlyph(i, '0' + i);
    }
}

void PitchPage::DrawSliders() {
    static constexpr uint8_t SLIDER_Y_OFFSET = 9;
    sliders_renderer_.Draw(SLIDER_Y_OFFSET);
}

void PitchPage::DrawNotes() {
  static constexpr uint8_t PATTERN_SPACING = 12;
  static constexpr uint8_t GROUP_SPACING = 6;
  static constexpr uint8_t PATTERN_LEFT_OFFSET = 3;
  static constexpr uint8_t NUM_PATTERNS = 4;

  // Отрисовка нот для левой группы используя выбранные индексы
  for(uint8_t i = 0; i < NUM_PATTERNS; i++) {
    uint32_t x_offset = PATTERN_LEFT_OFFSET + (i * (3 + PATTERN_SPACING));
    RenderNote(notes_.left[i], x_offset, 0);
  }

  // Отрисовка правой группы нот
  uint32_t second_group_start = PATTERN_LEFT_OFFSET + 
                               (NUM_PATTERNS * (3 + PATTERN_SPACING)) + 
                               GROUP_SPACING;

  for(uint8_t i = 0; i < NUM_PATTERNS; i++) {
    uint32_t x_offset = second_group_start + (i * (3 + PATTERN_SPACING));
    RenderNote(notes_.right[i], x_offset, 0);
  }
}

void PitchPage::ShowNoteModal(const Note& note) {
    modal_window_.SetPitchData(note.base, note.octave, note.sharp, state_.current_encoder);
    modal_window_.Show(ModalType::PITCH_MODAL);
}

void PitchPage::UpdateDisplay() {
    if(!state_.is_active) return;

    modal_window_.Update(system_clock.milliseconds());

    if(modal_window_.IsVisible()) {
        if(modal_window_.GetRegion().dirty) {
            modal_window_.Draw();
            UpdateRegion(modal_window_.GetRegion());
        }
        return;
    }

    if(regions_.header.dirty) {
        RenderHeader();
        UpdateRegion(regions_.header);
    }

    if(regions_.content.dirty) {
        RenderContent();
        UpdateRegion(regions_.content);
    }

    if(regions_.footer.dirty) {
        RenderFooter();
        UpdateRegion(regions_.footer);
    }
}

void PitchPage::OnEncoder(uint8_t encoder, int32_t increment) {
    if(!draw_context_.is_active) return;

    if(encoder < 8) {
        // Voice encoders
        UpdateNote(encoder, increment);
    }
    else {
        // Mod encoders
        using ENC = EncoderController::EncoderIndex;
        if(encoder == ENC::ENC_MOD_A || encoder == ENC::ENC_MOD_B) {
            // Сразу закрываем модальное окно при работе с футером
            modal_window_.Hide();
            
            if(encoder == ENC::ENC_MOD_A) {
                footer_.value_a = fclamp(footer_.value_a + increment, -12, 12);
                UpdateAllVoicePitches();
            }
            else if(encoder == ENC::ENC_MOD_B) {
                footer_.value_b = fclamp(footer_.value_b + increment, -12, 12);
            }
            regions_.footer.dirty = 1;
        }
    }
}

void PitchPage::OnSwitch(uint8_t sw, bool pressed) {
    if(!state_.is_active) return;
    state_.needs_redraw = true;
}

void PitchPage::OnEnterPage() {
    state_.is_active = true;
    draw_context_.is_active = true;
    
    // Clear all regions
    region_fill(&regions_.header, 0x0);
    region_fill(&regions_.content, 0x0);
    region_fill(&regions_.footer, 0x0);

    // Mark all regions as dirty
    regions_.header.dirty = 1;
    regions_.content.dirty = 1;
    regions_.footer.dirty = 1;

    // Draw initial content
    region_string(&regions_.header, "PITCH", 2, 0, 0xf, 0x0, 0);
    region_string(&regions_.header, "C MINOR", 34, 0, 0xf, 0x0, 0);

    // Draw all content region elements before updating display
    DrawNotes();     // Сначала ноты
    DrawSliders();   // Затем слайдеры
    DrawIcons();     // И иконки
    
    footer_renderer_.DrawFooter(
        regions_.footer,
        footer_.labelA,
        footer_.labelB,
        footer_.blockLabelA,
        footer_.blockLabelB,
        footer_.value_a,
        footer_.value_b
    );

    // Force immediate display update after all drawing is complete
    display_->DrawRegion(regions_.header.x, regions_.header.y,
                      regions_.header.w, regions_.header.h,
                      regions_.header.data);
    
    display_->DrawRegion(regions_.content.x, regions_.content.y,
                      regions_.content.w, regions_.content.h,
                      regions_.content.data);
    
    display_->DrawRegion(regions_.footer.x, regions_.footer.y,
                      regions_.footer.w, regions_.footer.h,
                      regions_.footer.data);

}

void PitchPage::OnExitPage() {
    state_.is_active = false;
    draw_context_.is_active = false;
}

void PitchPage::OnClick(uint8_t encoder) {
    if(!state_.is_active) return;
    state_.needs_redraw = true;
}

void PitchPage::OnLongClick(uint8_t encoder) {
    if(!state_.is_active) return;
    state_.needs_redraw = true;
}

void PitchPage::OnIdle() {
  // Обработка простоя
}

void PitchPage::GetNoteFromIndex(uint8_t index, Note& note) {
  static const char bases[] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
  static const bool sharps[] = {false, true, false, true, false, false, true, false, true, false, true, false};
  
  uint8_t octave = MIN_OCTAVE + (index / NOTES_PER_OCTAVE);
  uint8_t note_in_octave = index % NOTES_PER_OCTAVE;
  
  note.base = bases[note_in_octave];
  note.sharp = sharps[note_in_octave];
  note.octave = octave;
}

uint8_t PitchPage::GetIndexFromNote(const Note& note) {
  uint8_t base_index = 0;
  switch(note.base) {
    case 'C': base_index = 0; break;
    case 'D': base_index = 2; break;
    case 'E': base_index = 4; break;
    case 'F': base_index = 5; break;
    case 'G': base_index = 7; break;
    case 'A': base_index = 9; break;
    case 'B': base_index = 11; break;
  }
  
  return (note.octave - MIN_OCTAVE) * NOTES_PER_OCTAVE + base_index + (note.sharp ? 1 : 0);
}

void PitchPage::RenderNote(const Note& note, uint32_t x_offset, uint32_t y_offset) {
    // Формируем строку с нотой, включая # для диеза
    char note_str[4];
    if(note.sharp) {
        note_str[0] = note.base;
        note_str[1] = '#';
        note_str[2] = static_cast<char>('0' + note.octave);
        note_str[3] = 0;
        // Диезные ноты рисуем без дополнительного смещения
        region_string(&regions_.content, note_str, x_offset, y_offset, 0xf, 0x0, 0);
    } else {
        note_str[0] = note.base;
        note_str[1] = static_cast<char>('0' + note.octave);
        note_str[2] = 0;
        // Не диезные ноты сдвигаем на 4 пикселя вправо для центрирования
        region_string(&regions_.content, note_str, x_offset + 3, y_offset, 0xf, 0x0, 0);
    }
}

void PitchPage::UpdatePatchNote(const Note& note, plaits::Patch* patch) {
    int base_note = 60; // C4 = 60 в MIDI
    int octave_offset = (note.octave - 4) * 12;
    int note_offset = 0;
    
    switch(note.base) {
        case 'C': note_offset = 0; break;
        case 'D': note_offset = 2; break;
        case 'E': note_offset = 4; break;
        case 'F': note_offset = 5; break;
        case 'G': note_offset = 7; break;
        case 'A': note_offset = 9; break;
        case 'B': note_offset = 11; break;
    }
    
    if(note.sharp) note_offset++;
    // Use footer_.value_a instead of footer_value_a_
    patch->note = base_note + octave_offset + note_offset + footer_.value_a;
}

void PitchPage::UpdateAllVoicePitches() {
    for(uint8_t i = 0; i < NUM_VOICES; i++) {
        Note& note = i < 4 ? notes_.left[i] : notes_.right[i-4];
        UpdatePatchNote(note, patches_[i]);
    }
}

void PitchPage::InitRegions() {
    // Initialize header
    regions_.header.w = DisplayLayout::SCREEN_WIDTH;
    regions_.header.h = DisplayLayout::HEADER_HEIGHT;
    regions_.header.x = 0;
    regions_.header.y = 0;
    region_alloc(&regions_.header);

    // Initialize content
    regions_.content.w = DisplayLayout::SCREEN_WIDTH;
    regions_.content.h = DisplayLayout::CONTENT_HEIGHT;
    regions_.content.x = 0;
    regions_.content.y = regions_.header.h;
    region_alloc(&regions_.content);

    // Initialize footer
    regions_.footer.w = DisplayLayout::SCREEN_WIDTH;
    regions_.footer.h = DisplayLayout::SCREEN_HEIGHT - (regions_.header.h + regions_.content.h);
    regions_.footer.x = 0;
    regions_.footer.y = regions_.header.h + regions_.content.h;
    region_alloc(&regions_.footer);
}

void PitchPage::InitNotes() {
    // Initialize left notes
    notes_.left[0] = {.base = 'C', .octave = 2, .sharp = false};
    notes_.left[1] = {.base = 'G', .octave = 2, .sharp = false};
    notes_.left[2] = {.base = 'A', .octave = 2, .sharp = false};
    notes_.left[3] = {.base = 'C', .octave = 4, .sharp = false};

    // Initialize right notes
    notes_.right[0] = {.base = 'C', .octave = 4, .sharp = false};
    notes_.right[1] = {.base = 'G', .octave = 2, .sharp = false};
    notes_.right[2] = {.base = 'A', .octave = 2, .sharp = false};
    notes_.right[3] = {.base = 'C', .octave = 4, .sharp = false};
}

void PitchPage::UpdateNote(uint8_t encoder_index, int32_t increment) {
    Note& current_note = encoder_index < 4 ? 
                        notes_.left[encoder_index] : 
                        notes_.right[encoder_index - 4];
    
    UpdateNoteByEncoder(encoder_index, increment);
    regions_.content.dirty = 1;
    UpdatePatchNote(current_note, patches_[encoder_index]);
}

void PitchPage::DrawIcons() {
    static constexpr uint8_t ICON_Y_OFFSET = 27;
    icons_renderer_.Draw(ICON_Y_OFFSET);
}

void PitchPage::UpdateNoteByEncoder(uint8_t encoder, int32_t increment) {
    state_.current_encoder = encoder;  // Сохраняем текущий энкодер
    
    Note& current_note = encoder < 4 ? notes_.left[encoder] : notes_.right[encoder - 4];
    uint8_t current_index = GetIndexFromNote(current_note);
    
    int new_index = current_index + increment;
    if(new_index < 0) new_index = 0;
    uint8_t max_index = (MAX_OCTAVE - MIN_OCTAVE + 1) * NOTES_PER_OCTAVE - 1;
    if(new_index > max_index) new_index = max_index;
    
    GetNoteFromIndex(new_index, current_note);
    ShowNoteModal(current_note); // Show modal when note changes
}

void PitchPage::RenderContent() {
    region_fill(&regions_.content, 0x0);
    DrawNotes();     
    DrawSliders();   
    DrawIcons();     
}

void PitchPage::RenderHeader() {
    region_fill(&regions_.header, 0x0);
    region_string(&regions_.header, "PITCH", 2, 0, 0xf, 0x0, 0);
    region_string(&regions_.header, "C MINOR", 34, 0, 0xf, 0x0, 0);
}

void PitchPage::RenderFooter() {
    region_fill(&regions_.footer, 0x0);
    footer_renderer_.DrawFooter(
        regions_.footer,
        footer_.labelA,
        footer_.labelB,
        footer_.blockLabelA,
        footer_.blockLabelB,
        footer_.value_a,
        footer_.value_b
    );
}

void PitchPage::UpdateRegion(const region& reg) {
    display_->DrawRegion(reg.x, reg.y, reg.w, reg.h, reg.data);
    const_cast<region&>(reg).dirty = 0;
}

void PitchPage::MarkAllRegionsDirty() {
    regions_.header.dirty = 1;
    regions_.content.dirty = 1;
    regions_.footer.dirty = 1;
}

}  // namespace t8synth

