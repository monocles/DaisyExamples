#include "daisysp.h"
#include "volume_page.h" // Исправляем include
#include "drivers/region.h"
#include "drivers/font.h"
#include "stmlib/system/system_clock.h"

namespace t8synth {
using namespace daisysp;
using namespace stmlib;

// // Определяем массив всех доступных нот
// const char* const VolumePage::available_notes_[] = {
//     "C2", "D2", "E2", "F2", "G2", "A2", "B2", 
//     "C3", "D3", "E3", "F3", "G3", "A3", "B3",
//     "C4", "D4", "E4", "F4", "G4", "A4", "B4"
// };

void VolumePage::OnInit() {
    state_.is_active = true;
    InitRegions();
    // InitNotes();
    
    // modal_window_.Init(display_);
    // modal_window_.SetOnCloseCallback([](void* context) {
    //     auto page = static_cast<VolumePage*>(context);
    //     page->MarkAllRegionsDirty();
    // }, this);
    
    icons_renderer_.Init(&regions_.content);
    sliders_renderer_.Init(&regions_.content);
    // notes_renderer_.Init(&regions_.content);
    
    for(uint8_t i = 0; i < IconsRenderer::NUM_ICONS; i++) {
        icons_renderer_.SetGlyph(i, '0' + i);
    }
}

void VolumePage::DrawSliders() {
    static constexpr uint8_t SLIDER_Y_OFFSET = 9;
    sliders_renderer_.Draw(SLIDER_Y_OFFSET);
}

// void VolumePage::ShowNoteModal(const Note& note) {
//     modal_window_.SetPitchData(note.base, note.octave, note.sharp, state_.current_encoder);
//     modal_window_.Show(ModalType::PITCH_MODAL);
// }

void VolumePage::UpdateDisplay() {
    if(!state_.is_active) return;

    // modal_window_.Update(system_clock.milliseconds());

    // if(modal_window_.IsVisible()) {
    //     if(modal_window_.GetRegion().dirty) {
    //         modal_window_.Draw();
    //         UpdateRegion(modal_window_.GetRegion());
    //     }
    //     return;
    // }

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

void VolumePage::OnEncoder(uint8_t encoder, int32_t increment) {
    if(!draw_context_.is_active) return;

    if(encoder < 8) {
        UpdateVoiceVolume(encoder, increment);
        regions_.content.dirty = 1;
    }
    else {
        // Mod encoders
        using ENC = EncoderController::EncoderIndex;
        if(encoder == ENC::ENC_MOD_A || encoder == ENC::ENC_MOD_B) {
            // Сразу закрываем модальное окно при работе с футером
            // modal_window_.Hide();
            
            if(encoder == ENC::ENC_MOD_A) {
                // footer_.value_a = fclamp(footer_.value_a + increment, -12, 12);
                // UpdateAllVoicePitches();
            }
            else if(encoder == ENC::ENC_MOD_B) {
                // footer_.value_b = fclamp(footer_.value_b + increment, -12, 12);
            }
            regions_.footer.dirty = 1;
        }
    }
}

void VolumePage::OnSwitch(uint8_t sw, bool pressed) {
    if(!state_.is_active) return;
    state_.needs_redraw = true;
}

void VolumePage::OnEnterPage() {
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
    region_string(&regions_.header, "MIXER", 2, 0, 0xf, 0x0, 0);
    // region_string(&regions_.header, "C MINOR", 34, 0, 0xf, 0x0, 0);

    // Draw all content region elements before updating display
    // notes_renderer_.Draw(notes_.left, notes_.right); // Сначала ноты
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

    SyncVolumes();  // Синхронизируем значения при входе
}

void VolumePage::OnExitPage() {
    state_.is_active = false;
    draw_context_.is_active = false;
}

void VolumePage::OnClick(uint8_t encoder) {
    if(!state_.is_active) return;
    state_.needs_redraw = true;
}

void VolumePage::OnLongClick(uint8_t encoder) {
    if(!state_.is_active) return;
    state_.needs_redraw = true;
}

void VolumePage::OnIdle() {
  // Обработка простоя
}

// void VolumePage::UpdatePatchNote(const Note& note, plaits::Patch* patch) {
//     int base_note = 60; // C4 = 60 в MIDI
//     int octave_offset = (note.octave - 4) * 12;
//     int note_offset = 0;
    
//     switch(note.base) {
    //         case 'C': note_offset = 0; break;
//         case 'D': note_offset = 2; break;
//         case 'E': note_offset = 4; break;
//         case 'F': note_offset = 5; break;
//         case 'G': note_offset = 7; break;
//         case 'A': note_offset = 9; break;
//         case 'B': note_offset = 11; break;
//     }
    
//     if(note.sharp) note_offset++;
//     // Use footer_.value_a instead of footer_value_a_
//     patch->note = base_note + octave_offset + note_offset + footer_.value_a;
// }

// void VolumePage::UpdateAllVoicePitches() {
//     for(uint8_t i = 0; i < NUM_VOICES; i++) {
//         uint8_t midi_note = i < 4 ? notes_.left[i] : notes_.right[i-4];
//         Note note = MidiNoteToNote(midi_note);
//         UpdatePatchNote(note, patches_[i]);
//     }
// }

void VolumePage::InitRegions() {
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

// void VolumePage::InitNotes() {
//     // Инициализируем ноты сразу как MIDI значения
//     notes_.left[0] = 36;  // C2
//     notes_.left[1] = 43;  // G2
//     notes_.left[2] = 48;  // C3
//     notes_.left[3] = 55;  // G3

//     notes_.right[0] = 60;  // C4
//     notes_.right[1] = 67;  // G4
//     notes_.right[2] = 72;  // C5
//     notes_.right[3] = 79;  // G5
// }

// void VolumePage::UpdateNote(uint8_t encoder_index, int32_t increment) {
//     uint8_t& current_midi_note = encoder_index < 4 ? 
//                               notes_.left[encoder_index] : 
//                               notes_.right[encoder_index - 4];
    
//     UpdateNoteByEncoder(encoder_index, increment);
//     regions_.content.dirty = 1;
    
//     Note note = MidiNoteToNote(current_midi_note);
//     UpdatePatchNote(note, patches_[encoder_index]);
// }

void VolumePage::DrawIcons() {
    static constexpr uint8_t ICON_Y_OFFSET = 27;
    icons_renderer_.Draw(ICON_Y_OFFSET);
}

// void VolumePage::UpdateNoteByEncoder(uint8_t encoder, int32_t increment) {
//     state_.current_encoder = encoder;  // Сохраняем текущий энкодер
    
//     // Получаем указатель на нужную ноту
//     uint8_t& current_midi_note = encoder < 4 ? 
//                                 notes_.left[encoder] : 
//                                 notes_.right[encoder - 4];
    
//     // Изменяем MIDI ноту напрямую
//     int new_midi = static_cast<int>(current_midi_note) + increment;
    
//     // Используем простое ограничение вместо std::clamp
//     if(new_midi < kC2) new_midi = kC2;
//     if(new_midi > kC7) new_midi = kC7;
    
//     current_midi_note = static_cast<uint8_t>(new_midi);
    
//     // Показываем модальное окно с преобразованной нотой
//     ShowNoteModal(MidiNoteToNote(current_midi_note));
//     regions_.content.dirty = 1;
// }

// Note VolumePage::MidiNoteToNote(uint8_t midi_note) {
//     static const char NOTE_NAMES[] = "CCDDEFFGGAAB";
//     static const bool SHARP_TABLE[] = {0,1,0,1,0,0,1,0,1,0,1,0};
    
//     uint8_t note_in_octave = midi_note % 12;
//     uint8_t octave = (midi_note / 12) - 1;
    
//     return Note{
//         .base = NOTE_NAMES[note_in_octave],
//         .octave = octave,
//         .sharp = SHARP_TABLE[note_in_octave]
//     };
// }

// uint8_t VolumePage::NoteToMidiNote(const Note& note) {
//     static const uint8_t NOTE_VALUES[] = {
//         0,  // C
//         2,  // D
//         4,  // E
//         5,  // F
//         7,  // G
//         9,  // A
//         11  // B
//     };
    
//     uint8_t base_value = NOTE_VALUES[note.base - 'C'];
//     return ((note.octave + 1) * 12) + base_value + (note.sharp ? 1 : 0);
// }

void VolumePage::RenderContent() {
    region_fill(&regions_.content, 0x0);
    // notes_renderer_.Draw(notes_.left, notes_.right);
    DrawSliders();   
    DrawIcons();     
}

void VolumePage::RenderHeader() {
    region_fill(&regions_.header, 0x0);
    region_string(&regions_.header, "MIXER", 2, 0, 0xf, 0x0, 0);
    // region_string(&regions_.header, "C MINOR", 34, 0, 0xf, 0x0, 0);
}

void VolumePage::RenderFooter() {
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

void VolumePage::UpdateRegion(const region& reg) {
    // display_->DrawRegion(reg.x, reg.y, reg.w, reg.h, reg.data);
    const_cast<region&>(reg).dirty = 0;
}

void VolumePage::MarkAllRegionsDirty() {
    regions_.header.dirty = 1;
    regions_.content.dirty = 1;
    regions_.footer.dirty = 1;
}

void VolumePage::UpdateVoiceVolume(uint8_t voice_index, int32_t increment) {
    float normalized_volume = encoder_volumes_[voice_index];
    
    // Изменяем громкость с учетом инкремента
    if(increment < 0) {
        normalized_volume = daisysp::fmax(0.0f, normalized_volume - VOLUME_STEP);
    } else {
        normalized_volume = daisysp::fmin(1.0f, normalized_volume + VOLUME_STEP);
    }
    
    encoder_volumes_[voice_index] = normalized_volume;
    
    // Устанавливаем громкость с экспоненциальным маппингом
    voice_manager.SetVoiceVolume(voice_index, 
        daisysp::fmap(normalized_volume, 0.0f, 1.0f, daisysp::Mapping::EXP));
    
    // Отображаем значение в процентах
    int volume_percent = static_cast<int>(normalized_volume * 100.0f);
    daisy::DaisySeed::PrintLine("Voice %d volume: %d%%", voice_index, volume_percent);
}

// void VolumePage::SyncVolumes() {
//     // Синхронизируем значения с текущими громкостями голосов
//     for(size_t i = 0; i < VoiceManager::NUM_VOICES; i++) {
//         encoder_volumes_[i] = voice_manager.GetVoice(i).volume;
//         daisy::DaisySeed::PrintLine("Voice %d volume: %d%%", i, 
//             static_cast<int>(encoder_volumes_[i] * 100.0f));
//     }
// }

// Удаляем SetContext или оставляем базовую версию без voice_manager
// void VolumePage::SetContext(plaits::Patch** patches, 
//                           plaits::Voice** voices,
//                           plaits::Modulations* modulations,
//                           DisplayController* display,
//                           PotController* pots,
//                           void* context_data) {
//     // Since we're using global voice_manager now, we don't need anything special here
//     UiPage::SetContext(patches, voices, modulations, display, pots, context_data);
// }

}  // namespace t8synth

