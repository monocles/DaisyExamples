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
Patch patch = {};
Modulations modulations = {};

char shared_buffer[16384] = {};

#define BLOCK_SIZE 32
#define MAX_BLOCK_SIZE 64
Voice::Frame outputPlaits[BLOCK_SIZE];


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	// loadMeter.OnBlockStart();
	encoders.UpdateHardware();  // Только SPI операции
  	// pots.Update();
	// Читаем сглаженные значения потенциометров
	patch.harmonics = pots.GetPotValue(0);
    patch.timbre = pots.GetPotValue(1);
    patch.morph = pots.GetPotValue(2);

	// float pitch = -3.0f;  // Смещение на 2 октавы вниз от C4

	// patch.note = 60.f + pitch * 12.f;
	patch.lpg_colour =  pots.GetPotValue(3);

	patch.decay =  pots.GetPotValue(4);
	patch.frequency_modulation_amount = 0.f;
	patch.timbre_modulation_amount = 0.f;
	patch.morph_modulation_amount = 0.f;

	float voct_cv = pots.GetPotValue(5);
	float voct    = fmap(voct_cv, 0.f, 60.f);
	modulations.note = voct;

	// modulations.trigger = 0.f;
	// modulations.trigger_patched = true;

	voice.Render(patch, modulations, outputPlaits, size);
	for (size_t i = 0; i < size; i++)
	{
		OUT_L[i] = outputPlaits[i].out / 32768.f;
		OUT_R[i] = outputPlaits[i].out / 32768.f;
	}
	// loadMeter.OnBlockEnd();
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
	ui.Init(&encoders, &display, &pots, &patch, &voice, &modulations);  // Передаем указатель на patch

	patch.engine = 2;
	stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
    voice.Init(&allocator);

	hw.StartAudio(AudioCallback);

	while(1) {
		ui.DoEvents();
	}
}
