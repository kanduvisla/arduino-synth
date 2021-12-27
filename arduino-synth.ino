#include <MozziGuts.h>
#include <Oscil.h>
#include <Smooth.h>
#include <mozzi_rand.h>
#include <mozzi_midi.h>
#include <EventDelay.h>
#include <ADSR.h>
#include <tables/sin2048_int8.h>

const int testPin1 = A1;
const int testPin2 = A2;

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aCarrier(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aModulator(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aModulator2(SIN2048_DATA);

#define CONTROL_RATE 64 // Defines how many times per second updateControl() is called

float smoothness = 0.95f;
Smooth <long> aSmoothIntensity(smoothness);

// for triggering the envelope
EventDelay noteDelay;

ADSR <AUDIO_RATE, AUDIO_RATE> envelope;

void setup(){
  Serial.begin(115200);
  startMozzi(CONTROL_RATE);
  noteDelay.set(2000);  // 2 seconds countdown
}

unsigned int duration, attack, decay, sustain, release_ms;
int carrierFreq;                                                 // The frequency of the carrier
byte midi_note = 30;

Q16n16 deviation; // Figure out how to describe this :-P

void updateControl(){
  // For testing purposes, we use EventDelay:
  if(noteDelay.ready()) {
    byte attack_level = 255;                                    // TODO: Assign this to a pot
    byte release_level = 0;                                     // TODO: Assign this to a pot
    envelope.setLevels(attack_level, 255, 255, release_level);    // We only play with attack and Release
    envelope.setTimes(25, 0, 0, 450);                            // attack = 0.5 seconds, release = 2 seconds
    envelope.noteOn();
    // Play a random note:
    //byte midi_note = rand(40)+40;
    carrierFreq = (int) mtof(midi_note);
    midi_note += 1;
    if (midi_note > 42) {
      midi_note = 30;
    }
    aCarrier.setFreq(carrierFreq);
    noteDelay.start(500);                                        // notedelay is set to the total duration of the envelope times
  }
  
  // int carrierFreq = mozziAnalogRead(testPin1);
  int modRatio = (mozziAnalogRead(testPin2) >> 6) + 1;  // Convert range 0-1023 to range 1-8
  int modFreq = modRatio * carrierFreq;

  int modRatio2 = (mozziAnalogRead(testPin1) >> 6) + 1;
  int modFreq2 = modRatio2 * modFreq;                   // This can get interesting

  deviation = float_to_Q16n16((float) modFreq);
  
  aModulator.setFreq(modFreq);
  aModulator2.setFreq(modFreq2 >> 8);
}

AudioOutput_t updateAudio(){
  envelope.update();
  // return MonoOutput::from16Bit((int) (envelope.next() * aCarrier.next()));      // = normal sound wave output
  // Let's try some phase modulation:
  Q15n16 modulation = deviation * aModulator.next() >> 16;
  // Modulate the modulation:
  Q15n16 modulation2 = deviation * aModulator2.phMod(modulation) >> 16;
  
  return MonoOutput::from16Bit((int) (envelope.next() * aCarrier.phMod(modulation2)));
}


/*
void setFrequencies() {
    int carrier_freq = kMapCarrierFreq(freqs[0]);
    int mod_freq = carrier_freq * mod_ratio;

    int intensity_calibrated = kMapIntensity(freqs[2]);
    fm_intensity = ((long)intensity_calibrated * (modDepth.next()+128))>>8;

    float mod_speed = (float)kMapModSpeed(freqs[1])/1000;

    carrier.setFreq(carrier_freq);
    modulator.setFreq(mod_freq);
    modDepth.setFreq(mod_speed);    
}
*/

//int updateAudio() {
//  int audio;
//
//  // long mod = aSmoothIntensity.next(fm_intensity) * aModulator.next();
//  long mod = aSmoothIntensity.next(aModulator.next()) << 6;
//  audio = aCarrier.phMod(mod);
//
//  // Feedback:
//  long mod2 = aSmoothIntensity.next(audio) << 4;
//  return aCarrier.phMod(mod2);
//  
//  // return audio;
//}

//AudioOutput_t updateAudio(){
//  Q15n16 modulation = (1024 * aModulator.next()) >> 2;
//  // return MonoOutput::from8Bit(aCarrier.next());
//  return MonoOutput::from8Bit(aCarrier.phMod(modulation) >> 2);
//}

//int updateAudio(){
//  Q15n16 modulation = (1024 * aModulator.next()) >> 2;
//  // return MonoOutput::from8Bit(aCarrier.next());
//  return aSmoothGain(aCarrier.phMod(modulation));
//}


void loop(){
  audioHook();
}
