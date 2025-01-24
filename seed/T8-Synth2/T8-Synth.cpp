#include "daisy_seed.h"
#include "daisysp.h"
#include "dsp/voice.h"
#include "ui.h"

#include "drivers/encoder_controller.h"
#include "drivers/spi_manager.h"
#include "drivers/display_controller.h"
// #include "drivers/pot_controller.h"
#include "voice_manager.h"

using namespace daisy;
using namespace daisysp;
using namespace plaits;

DaisySeed hw;
// CpuLoadMeter loadMeter;

SpiManager spi_manager;
EncoderController encoders;
// PotController pots;
DisplayController display;
Ui ui;

static daisy::TimerHandle tim;
static daisy::GPIO led;

// const float avgLoad = loadMeter.GetAvgCpuLoad();

static void TimerCallback(void* data)
{
	ui.Poll();
}

VoiceManager voice_manager;
Modulations modulations = {};

#define BLOCK_SIZE 32
#define MAX_BLOCK_SIZE 64

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	encoders.UpdateHardware();
    // pots.Update();

    // Update common parameters with template method
    // voice_manager.SetCommonParameter(&plaits::Patch::harmonics, pots.GetPotValue(0));
    // voice_manager.SetCommonParameter(&plaits::Patch::timbre, pots.GetPotValue(1));
    // voice_manager.SetCommonParameter(&plaits::Patch::morph, pots.GetPotValue(2));
    // voice_manager.SetCommonParameter(&plaits::Patch::lpg_colour, pots.GetPotValue(3));
    // voice_manager.SetCommonParameter(&plaits::Patch::decay, pots.GetPotValue(4));

    Voice::Frame outputs[VoiceManager::NUM_VOICES][BLOCK_SIZE];

    // Рендеринг голосов
    for(size_t v = 0; v < VoiceManager::NUM_VOICES; v++) {
        auto& voice_unit = voice_manager.GetVoice(v);
        voice_unit.voice.Render(voice_unit.patch, modulations, outputs[v], size);
    }

    // Раздельное микширование основного и вспомогательного выходов
    for(size_t i = 0; i < size; i++) {
        float mix_main = 0.0f;
        float mix_aux = 0.0f;
        
        for(size_t v = 0; v < VoiceManager::NUM_VOICES; v++) {
            mix_main += outputs[v][i].out;
            mix_aux += outputs[v][i].aux;
        }
        
        // Нормализуем каждый канал отдельно
        OUT_L[i] = mix_main / (VoiceManager::NUM_VOICES * 32768.0f);
        OUT_R[i] = mix_aux / (VoiceManager::NUM_VOICES * 32768.0f);
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
  	hw.StartLog(true);
	InitPollTimer();

    // Initialize SPI manager with hardware reference
    spi_manager.Init(&hw);
    
    // Pass SPI manager to encoders
    encoders.Init(spi_manager);
    display.Init(hw, spi_manager);
	voice_manager.Init();
	ui.Init(&encoders, &display, &voice_manager, &modulations);

	hw.StartAudio(AudioCallback);
    hw.PrintLine("Log started");

	while(1) {
		ui.DoEvents();
	}
}
