// Copyright 2016 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Naive speech synth - made from "synthesizer" building blocks (pulse
// oscillator and zero-delay SVF).

#ifndef PLAITS_DSP_SPEECH_NAIVE_SPEECH_SYNTH_H_
#define PLAITS_DSP_SPEECH_NAIVE_SPEECH_SYNTH_H_

#include "stmlib/dsp/filter.h"

#include "dsp/dsp.h"
#include "dsp/oscillator/oscillator.h"

namespace plaits {

const int kNaiveSpeechNumFormants = 5;
const int kNaiveSpeechNumPhonemes = 5;
const int kNaiveSpeechNumRegisters = 5;

class NaiveSpeechSynth {
 public:
  NaiveSpeechSynth() { }
  ~NaiveSpeechSynth() { }

  void Init();
  
  void Render(
      bool click,
      float frequency,
      float phoneme,
      float vocal_register,
      float* temp,
      float* excitation,
      float* output,
      size_t size);

 private:
  struct Formant {
    uint8_t frequency;
    uint8_t amplitude;
  };
  
  struct Phoneme {
    Formant formant[kNaiveSpeechNumFormants];
  };

  Oscillator pulse_;
  float frequency_;
  size_t click_duration_;
  
  stmlib::Svf filter_[kNaiveSpeechNumFormants];
  stmlib::Svf pulse_coloration_;

  static const Phoneme phonemes_[kNaiveSpeechNumPhonemes][kNaiveSpeechNumRegisters];
  
  DISALLOW_COPY_AND_ASSIGN(NaiveSpeechSynth);
};
  
}  // namespace plaits

#endif  // PLAITS_DSP_SPEECH_NAIVE_SPEECH_SYNTH_H_
