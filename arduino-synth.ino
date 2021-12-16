#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>

const int testPin1 = A1;
const int testPin2 = A2;

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aCarrier(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aModulator(SIN2048_DATA);

#define CONTROL_RATE 64

void setup(){
  Serial.begin(115200);
  startMozzi(CONTROL_RATE);
}


void updateControl(){
  int amount1 = mozziAnalogRead(testPin1);
  int amount2 = mozziAnalogRead(testPin2);
  aCarrier.setFreq(amount1);
}


AudioOutput_t updateAudio(){
  return MonoOutput::from8Bit(aCarrier.next());
}


void loop(){
  audioHook();
}
