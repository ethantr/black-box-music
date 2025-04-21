import time
from .base_mode import MidiMode
from ..midi_service import MidiService

class BoxTwoMode(MidiMode):
    def __init__(self, midi_service: MidiService):
        super().__init__(midi_service)
        self.active = False
        self.note_on = False
        self.step_index = 0
        self.next_time = time.time()
        self.latest_data = None
        self.pattern = [60, 62, 63, 64]  # C4, D4, D#4, E4

    def process(self, data):
        self.latest_data = data
        now = time.time()

        # Only trigger when distance is high
        distance = float(data.get('D', 0))
        if distance > 50:
            self.active = True
        else:
            if self.active:
                self._turn_off_note()  # turn off current note
                self.active = False
            return

        # If not time yet, skip
        if now < self.next_time:
            return

        if not self.note_on:
            note = self.pattern[self.step_index % len(self.pattern)]
            velocity = min(max(int(distance / 8), 30), 127)  # Add a lower limit
            self.midi_service.send_note(note=note, velocity=velocity)
            self.note_on = True
            self.next_time = now + 0.3  # note duration
        else:
            note = self.pattern[self.step_index % len(self.pattern)]
            self.midi_service.send_note_off(note=note)
            self.note_on = False
            self.step_index += 1
            self.next_time = now + 0.2  # pause between notes

    def _turn_off_note(self):
        """Ensures note is off when deactivating."""
        if self.note_on:
            note = self.pattern[self.step_index % len(self.pattern)]
            self.midi_service.send_note_off(note=note)
            self.note_on = False
