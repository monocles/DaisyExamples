#include "daisy_seed.h"
#include "daisysp.h"
#include "spi_manager.h"      // Include this first
#include "pot_controller.h"   // Then this
#include "display_controller.h" // And this
// #include "region.h"           // And this
// #include "font.h"           // And this
#include "encoder_controller.h" // Add after other includes
#include "ui.h"
#include <string.h>
#include "plaits/dsp/voice.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
static PotController pots; // Make it static to ensure proper initialization
DisplayController display;
static SpiManager spi_manager;
static EncoderController encoders; // Add after other global variables
static Ui ui;

plaits::Voice voice = {};
plaits::Patch patch = {};
plaits::Modulations modulations = {};
char shared_buffer[16384] = {};

#define BLOCK_SIZE 16
plaits::Voice::Frame outputPlaits[BLOCK_SIZE];

// Изначальное объявление с размером 64x64
// static region menuRegion = { .w = 64, .h = 64, .x = 0, .y = 0 };
// static scroll menuScroll;
// static int currentMenuItem = 0;
// const char* menuItems[] = {
//     "ANALOG",
//     "WSHAPE", 
//     "DUAL FM",
//     "GRAINS",
//     "HARM",
//     "WTABLE",
//     "CHORDS", 
//     "VOVEL",
//     "CLOUD",
//     "NOISE"
// };
// const int NUM_MENU_ITEMS = 10;

// // Константы для отображения меню
// const int VISIBLE_ITEMS = 5;  // Количество видимых элементов
// const int ITEM_HEIGHT = 12;   // Высота каждого элемента
// const int MENU_HEIGHT = VISIBLE_ITEMS * ITEM_HEIGHT; // Полная высота меню


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    // Update pot readings
    pots.Update();

    // Remove old menu control code
    // float scrollPos = pots.GetPotValue(0);
    // int newMenuItem = (int)(scrollPos * (NUM_MENU_ITEMS - 1));
    
    // if(newMenuItem != currentMenuItem) {
    //     currentMenuItem = newMenuItem;
    //     updateMenuDisplay();
    // }

    // Get pot values and map them to parameters
    patch.harmonics = pots.GetPotValue(0);
    patch.timbre = pots.GetPotValue(1);
    patch.morph = pots.GetPotValue(2);
    
    // Убрали выбор engine отсюда

	float pitch = -3.0f;  // Смещение на 2 октавы вниз от C4

	 patch.note = 60.f + pitch * 12.f;
	// // patch.harmonics = .5f;
	// // patch.timbre = .0f;
	// // patch.morph = .5f;
	patch.lpg_colour = 0.5f;
	patch.decay = 0.5f;
	patch.frequency_modulation_amount = 0.f;
	patch.timbre_modulation_amount = 0.f;
	patch.morph_modulation_amount = 0.f;

	float voct_cv = patch.morph = pots.GetPotValue(3);
	float voct    = fmap(voct_cv, 0.f, 60.f);
	modulations.note = voct;

	modulations.harmonics = 0.2f;
	modulations.timbre = 0.2f;
	modulations.morph =0.2f;

// if (toggle.Pressed()) {
// 	modulations.trigger = 5.f * hw.gate_in_1.State();
// 	modulations.trigger_patched = true;
// }
// else {
// 	modulations.trigger = 0.f;
// 	modulations.trigger_patched = false;
// }

	modulations.trigger = 0.f;
	modulations.trigger_patched = false;


	voice.Render(patch, modulations, outputPlaits, BLOCK_SIZE);

	for (size_t i = 0; i < size; i++) {
		OUT_L[i] = outputPlaits[i].out / 32768.f;
		OUT_R[i] = outputPlaits[i].aux / 32768.f;
	}
}

// Добавляем счетчики для всех энкодеров
// struct EncoderDebug {
//     int counter;
//     int32_t last_pos;
// };

// static EncoderDebug encoders_debug[4] = {
//     {50, 0},  // Encoder 1
//     {50, 0},  // Encoder 2
//     {50, 0},  // Encoder 3
//     {50, 0}   // Encoder 4
// };

// int encoder1_counter = 50; // Начальное значение по центру (0-100)
// const int ENCODER_MIN = 0;
// const int ENCODER_MAX = 100;
// static int32_t last_encoder1_pos = 0;
// static uint32_t last_encoder_update = 0; // Добавляем переменную для отслеживания времени

int main(void) {
    hw.Init();
    hw.SetAudioBlockSize(BLOCK_SIZE);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    System::Config sys_config;
    sys_config.Boost();

    // Initialize SPI manager first
    spi_manager.Init(hw);
    
    // Initialize controllers
    pots.Init(hw, spi_manager);
    display.Init(hw, spi_manager);
    encoders.Init(hw);
    
    // Initialize UI last, passing all controllers (including hw)
    ui.Init(&encoders, &pots, &display, &hw);
    

    // Enable logging
    hw.StartLog(true);
    // hw.PrintLine("T8-VOICE ADC Debug");

	patch.engine = 0;
	modulations.engine = 0;

	stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
    voice.Init(&allocator);

    hw.StartAudio(AudioCallback);

    uint32_t last_print = System::GetNow();
    
    // Remove old menu initialization code and duplicate UI init
    
    while(1) {
        // Update encoders first
        encoders.Update();
        
        // Then poll UI
        ui.Poll();
        ui.DoEvents();
        // hw.PrintLine(pots.GetPotValue(0));
        
        // Remove or comment out the old encoder debug code since it might interfere
        /*
        for(int i = 0; i < 4; i++) {
            const EncoderController::EncoderState& enc = encoders.GetEncoder(i);
            if(enc.position != encoders_debug[i].last_pos) {
                // ...existing debug code...
            }
        }
        */
        
        uint32_t now = System::GetNow();
        
        // Обновляем энкодеры
        // encoders.Update();

        // Обновляем состояния энкодеров (без вывода)
        // for(int i = 0; i < 4; i++) {
        //     const EncoderController::EncoderState& enc = encoders.GetEncoder(i);
        //     if(enc.position != encoders_debug[i].last_pos) {
        //         if(enc.position > encoders_debug[i].last_pos) {
        //             encoders_debug[i].counter = std::min(encoders_debug[i].counter + 1, ENCODER_MAX);
        //         } else {
        //             encoders_debug[i].counter = std::max(encoders_debug[i].counter - 1, ENCODER_MIN);
        //         }
        //         encoders_debug[i].last_pos = enc.position;
        //     }
        // }
        
        // // Вывод отладочной информации с интервалом
        if(now - last_print > 100) {
            // Print encoder states
            // for(int i = 0; i < 4; i++) {
            //     const EncoderController::EncoderState& enc = encoders.GetEncoder(i);
            //     hw.PrintLine("Encoder %d: pos=%d val=%d btn=%s %s", 
            //         i + 1,
            //         enc.position,
            //         encoders_debug[i].counter,
            //         enc.button ? "PRESSED" : "RELEASED",
            //         enc.changed ? "[CHANGED]" : ""
            //     );
            // }

            // Print pots debug
            // for(int i = 0; i < PotController::NUM_ANALOG_POTS; i++) {
            //     float raw_value = pots.GetPotValue(i) * 100.0f;
            //     int int_part = (int)raw_value;
            //     int frac_part = (int)((raw_value - int_part) * 100);
            //     hw.PrintLine("POT %d: %d.%02d%%", i, int_part, frac_part);
            // }

            // // Print buttons and switches
            // for(int i = 0; i < PotController::NUM_BUTTONS; i++) {
            //     hw.PrintLine("BUTTON %d: %s", i, pots.GetButtonValue(i) ? "ON" : "OFF");
            // }
            
            // for(int i = 0; i < PotController::NUM_SWITCHES; i++) {
            //     hw.PrintLine("SWITCH %d: %s", i, pots.GetSwitchValue(i) ? "ON" : "OFF");
            // }

            // hw.PrintLine("-------------------");
            last_print = now;
        }
    }
}