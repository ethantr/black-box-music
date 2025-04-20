from .base_mode import MidiMode

class BoxOneMode(MidiMode):
    def __init__(self, midi_service):
        super().__init__(midi_service)
        self.last_note = 60

    def process(self, data):
        if float(data.get('P', 0)) > 50:
            self.midi_service.send_note(note=self.last_note, velocity=100)
            self.last_note = 60 if self.last_note == 64 else self.last_note + 1