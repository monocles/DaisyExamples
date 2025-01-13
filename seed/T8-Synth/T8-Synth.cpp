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
CpuLoadMeter loadMeter;

SpiManager spi_manager;
EncoderController encoders;
PotController pots;
DisplayController display;
Ui ui;

static daisy::TimerHandle tim;
static daisy::GPIO led;

const float avgLoad = loadMeter.GetAvgCpuLoad();

static void TimerCallback(void* data)
{
	ui.Poll();
}

Voice voice = {};
Patch patch = {};
Modulations modulations = {};

char shared_buffer[16384] = {};

#define BLOCK_SIZE 64
#define MAX_BLOCK_SIZE 128
Voice::Frame outputPlaits[BLOCK_SIZE];


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	loadMeter.OnBlockStart();
	encoders.UpdateHardware();  // Только SPI операции
	pots.Update();
	
	// patch.harmonics = pots.GetPotValue(0);
    // patch.timbre = pots.GetPotValue(1);
    // patch.morph = pots.GetPotValue(2);
	patch.harmonics = .2f;
    patch.timbre = .2f;
    patch.morph = .2f;

	float pitch = -2.0f;  // Смещение на 2 октавы вниз от C4

	patch.note = 60.f + pitch * 12.f;
	patch.lpg_colour = 0.5f;
	patch.decay = 0.5f;
	patch.frequency_modulation_amount = 0.f;
	patch.timbre_modulation_amount = 0.f;
	patch.morph_modulation_amount = 0.f;

	float voct_cv = 0.2f;
	float voct    = fmap(voct_cv, 0.f, 60.f);
	modulations.note = voct;

	modulations.trigger = 0.f;
	modulations.trigger_patched = false;

	// modulations.harmonics = 0.2f;
	// modulations.timbre = 0.2f;
	// modulations.morph =0.2f;


	voice.Render(patch, modulations, outputPlaits, size);
	for (size_t i = 0; i < size; i++)
	{
		OUT_L[i] = outputPlaits[i].out / 32768.f;
		OUT_R[i] = outputPlaits[i].aux / 32768.f;
	}
	loadMeter.OnBlockEnd();
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
    loadMeter.Init(hw.AudioSampleRate(), BLOCK_SIZE);
	InitPollTimer();

	spi_manager.Init();
	pots.Init(hw, spi_manager);
	encoders.Init();
	display.Init(hw, spi_manager);
	ui.Init(&encoders, &display);

	stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
    voice.Init(&allocator);

	hw.StartAudio(AudioCallback);

	while(1) {
		ui.DoEvents();
		float raw_value = pots.GetPotValue(0) * 100.0f;
		int int_part = (int)raw_value;
		int frac_part = (int)((raw_value - int_part) * 100);
		hw.PrintLine("POT 0: %d.%02d%%", int_part, frac_part);
		// float raw_value2 = pots.GetPotValue(1) * 100.0f;
		// int int_part2 = (int)raw_value2;
		// int frac_part2 = (int)((raw_value2 - int_part2) * 100);
		// hw.PrintLine("POT 1: %d.%02d%%", int_part2, frac_part2);
		// float raw_value3 = pots.GetPotValue(2) * 100.0f;
		// int int_part3 = (int)raw_value3;
		// int frac_part3 = (int)((raw_value3 - int_part3) * 100);
		// hw.PrintLine("POT 2: %d.%02d%%", int_part3, frac_part3);

	}
}
