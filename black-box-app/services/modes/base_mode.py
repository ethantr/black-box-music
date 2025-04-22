"""
Base class for all modes in the MIDI service."""
class MidiMode:
    def __init__(self, midi_service):
        self.midi_service = midi_service

    def process(self, data: dict):
        raise NotImplementedError("This should be implemented by subclasses")
    
    def tick(self):
        """Call this regularly from the main loop."""
        raise NotImplementedError("This should be implemented by subclasses")