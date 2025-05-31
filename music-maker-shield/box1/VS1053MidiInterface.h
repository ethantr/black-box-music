#include <stdint.h>
#include "MidiInterface.h"
#include "midi_helpers.h"

class VS1053MidiInterface : public MidiInterface {
public:
  void noteOn(uint8_t chan, uint8_t note, uint8_t vel) override {
    midiNoteOn(chan, note, vel);
  }
  
  void noteOff(uint8_t chan, uint8_t note, uint8_t vel) override {
    midiNoteOff(chan, note, vel);
  }

  void controlChange(uint8_t chan, uint8_t control, uint8_t value) override {
    midiCC(chan, control, value);
  }

  void programChange(uint8_t chan, uint8_t program) override {
    midiSetInstrument(chan, program);
  }

  void setBank(uint8_t channel, uint8_t bank) override {
    midiSetChannelBank(channel, bank);
  }

  void setVolume(uint8_t channel, uint8_t volume){
    midiSetChannelVolume(channel, volume);
  }


};
