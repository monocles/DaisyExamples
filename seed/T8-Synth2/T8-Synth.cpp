#include "daisy_seed.h"
#include "daisysp.h"
#include "dsp/voice.h"
#include "ui.h"

#include "drivers/encoder_controller.h"
#include "drivers/spi_manager.h"
#include "drivers/display_controller.h"
#include "drivers/pot_controller.h"

using namespace daisy;
using namespace daisysp;
using namespace plaits;

DaisySeed hw;
// CpuLoadMeter loadMeter;

SpiManager spi_manager;
EncoderController encoders;
PotController pots;
DisplayController display;
Ui ui;

static daisy::TimerHandle tim;
static daisy::GPIO led;

// const float avgLoad = loadMeter.GetAvgCpuLoad();

static void TimerCallback(void* data)
{
	ui.Poll();
}

Voice voice = {};
Voice voice2 = {};  // Добавляем дополнительные голоса
Voice voice3 = {};
Voice voice4 = {};

Patch patch = {};
Patch patch2 = {};  // И патчи для них
Patch patch3 = {};
Patch patch4 = {};

Modulations modulations = {};

// Отдельные буферы для каждого голоса
char shared_buffer[16384] = {};
char shared_buffer2[16384] = {};
char shared_buffer3[16384] = {};
char shared_buffer4[16384] = {};

#define BLOCK_SIZE 32
#define MAX_BLOCK_SIZE 64
Voice::Frame outputPlaits[BLOCK_SIZE];


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	encoders.UpdateHardware();

    // Читаем значения потенциометров для всех голосов
    float harmonics = pots.GetPotValue(0);
    float timbre = pots.GetPotValue(1);
    float morph = pots.GetPotValue(2);
    float lpg_colour = pots.GetPotValue(3);
    float decay = pots.GetPotValue(4);

    // Обновляем параметры для всех патчей
    patch.harmonics = patch2.harmonics = patch3.harmonics = patch4.harmonics = harmonics;
    patch.timbre = patch2.timbre = patch3.timbre = patch4.timbre = timbre;
    patch.morph = patch2.morph = patch3.morph = patch4.morph = morph;
    patch.lpg_colour = patch2.lpg_colour = patch3.lpg_colour = patch4.lpg_colour = lpg_colour;
    patch.decay = patch2.decay = patch3.decay = patch4.decay = decay;

    // Общие параметры модуляции
    patch.frequency_modulation_amount = patch2.frequency_modulation_amount = 
    patch3.frequency_modulation_amount = patch4.frequency_modulation_amount = 0.f;
    
    patch.timbre_modulation_amount = patch2.timbre_modulation_amount = 
    patch3.timbre_modulation_amount = patch4.timbre_modulation_amount = 0.f;
    
    patch.morph_modulation_amount = patch2.morph_modulation_amount = 
    patch3.morph_modulation_amount = patch4.morph_modulation_amount = 0.f;

    Voice::Frame outputPlaits[BLOCK_SIZE];
    Voice::Frame outputPlaits2[BLOCK_SIZE];
    Voice::Frame outputPlaits3[BLOCK_SIZE];
    Voice::Frame outputPlaits4[BLOCK_SIZE];

    voice.Render(patch, modulations, outputPlaits, size);
    voice2.Render(patch2, modulations, outputPlaits2, size);
    voice3.Render(patch3, modulations, outputPlaits3, size);
    voice4.Render(patch4, modulations, outputPlaits4, size);

    for(size_t i = 0; i < size; i++) {
        // Смешиваем все голоса
        float mix = (outputPlaits[i].out + 
                    outputPlaits2[i].out + 
                    outputPlaits3[i].out + 
                    outputPlaits4[i].out) / (4.0f * 32768.0f);
        OUT_L[i] = mix;
        OUT_R[i] = mix;
    }
}

void InitPollTimer() {
	daisy::TimerHandle::Config tim_cfg;
    tim_cfg.periph     = daisy::TimerHandle::Config::Peripheral::TIM_5;
    tim_cfg.dir        = daisy::TimerHandle::Config::CounterDir::UP;
    tim_cfg.enable_irq = true;
    tim_cfg.period     = 120000; // 480MHz / 120000 = 1kHz (1ms period)
    tim.Init(tim_cfg);
    tim.SetCallback(TimerCallback);
    tim.Start();
}

int main(void)
{
	hw.Init(true);
	hw.SetAudioBlockSize(BLOCK_SIZE); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
  	hw.StartLog();
    // loadMeter.Init(hw.AudioSampleRate(), BLOCK_SIZE);
	InitPollTimer();

	spi_manager.Init();
	pots.Init(hw, spi_manager);
	display.Init(hw, spi_manager);
	encoders.Init();
	ui.Init(&encoders, &display, &pots, 
            &patch, &patch2, &patch3, &patch4,  // Передаем все патчи
            &voice, &voice2, &voice3, &voice4,  // И все голоса
            &modulations);

	patch.engine = patch2.engine = patch3.engine = patch4.engine = 2;
	stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
    stmlib::BufferAllocator allocator2(shared_buffer2, sizeof(shared_buffer2));
    stmlib::BufferAllocator allocator3(shared_buffer3, sizeof(shared_buffer3));
    stmlib::BufferAllocator allocator4(shared_buffer4, sizeof(shared_buffer4));
    
    voice.Init(&allocator);
    voice2.Init(&allocator2);
    voice3.Init(&allocator3);
    voice4.Init(&allocator4);

	hw.StartAudio(AudioCallback);

	while(1) {
		ui.DoEvents();
	}
}
