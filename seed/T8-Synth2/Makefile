# Project Name
TARGET = T8-Synth
APP_TYPE = BOOT_SRAM

# Sources
TOPLEVEL_SOURCES = \
T8-Synth.cpp \
dsp/voice.cpp \
resources.cpp \
ui.cpp \
drivers/encoder_controller.cpp \
drivers/display_controller.cpp \
drviers/region.cpp \
drivers/font.cpp \
$(STMLIB_DIR)/dsp/units.cpp \
$(STMLIB_DIR)/system/system_clock.cpp \
$(STMLIB_DIR)/utils/random.cpp

TEST_SOURCES = \
dsp/engine/wavetable_engine.cpp \
dsp/engine2/string_machine_engine.cpp \
dsp/engine/string_engine.cpp \
dsp/engine2/virtual_analog_vcf_engine.cpp \

ENGINE_1_SOURCES = \
dsp/engine/additive_engine.cpp \
dsp/engine/bass_drum_engine.cpp \
dsp/engine/chord_engine.cpp \
dsp/engine/fm_engine.cpp \
dsp/engine/grain_engine.cpp \
dsp/engine/hi_hat_engine.cpp \
dsp/engine/modal_engine.cpp \
dsp/engine/noise_engine.cpp \
dsp/engine/particle_engine.cpp \
dsp/engine/snare_drum_engine.cpp \
dsp/engine/speech_engine.cpp \
dsp/engine/string_engine.cpp \
dsp/engine/swarm_engine.cpp \
dsp/engine/virtual_analog_engine.cpp \
dsp/engine/waveshaping_engine.cpp \
dsp/engine/wavetable_engine.cpp

ENGINE_2_SOURCES = \
dsp/engine2/chiptune_engine.cpp \
dsp/engine2/phase_distortion_engine.cpp \
dsp/engine2/six_op_engine.cpp \
dsp/engine2/string_machine_engine.cpp \
dsp/engine2/virtual_analog_vcf_engine.cpp \
dsp/engine2/wave_terrain_engine.cpp

CHORD_SOURCES = \
dsp/chords/chord_bank.cpp

FM_SOURCES = \
dsp/fm/algorithms.cpp \
dsp/fm/dx_units.cpp

PHYSICAL_MODELLING_SOURCES = \
dsp/physical_modelling/modal_voice.cpp \
dsp/physical_modelling/resonator.cpp \
dsp/physical_modelling/string_voice.cpp \
dsp/physical_modelling/string.cpp

SPEECH_SOURCES = \
dsp/speech/lpc_speech_synth_controller.cpp \
dsp/speech/lpc_speech_synth_phonemes.cpp \
dsp/speech/lpc_speech_synth_words.cpp \
dsp/speech/lpc_speech_synth.cpp \
dsp/speech/naive_speech_synth.cpp \
dsp/speech/sam_speech_synth.cpp

CPP_SOURCES = \
$(TOPLEVEL_SOURCES) \
$(TEST_SOURCES) \
$(CHORD_SOURCES) \
$(PHYSICAL_MODELLING_SOURCES)
# $(ENGINE_1_SOURCES) \
# $(ENGINE_2_SOURCES) \
# $(FM_SOURCES) \
# $(SPEECH_SOURCES)

C_INCLUDES += \
-I. \
-Iresources \
-Istmlib \
-I../..

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/
STMLIB_DIR = stmlib/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
