#pragma once

class MidiInterface {
public:
  virtual void noteOn(uint8_t channel, uint8_t pitch, uint8_t velocity) = 0;
  virtual void noteOff(uint8_t channel, uint8_t pitch, uint8_t velocity) = 0;
  virtual void controlChange(uint8_t channel, uint8_t control, uint8_t value) = 0;
  virtual void programChange(uint8_t channel, uint8_t program) = 0;
  virtual void setBank(uint8_t channel, uint8_t bank) = 0;
  virtual void setVolume(uint8_t channel, uint8_t volume) = 0;

  virtual ~MidiInterface() {}
};
