import time
from .base_mode import MidiMode
from ..midi_service import MidiService

class BoxTwoMode(MidiMode):
    def __init__(self, midi_service: MidiService):
        super().__init__(midi_service)
        self.last_note_time = 0
        self.note_on = False
        self.next_event_time = time.time()
        self.latest_data = None
        self.active = False

    def process(self, data):
        """Called frequently with new sensor data."""
        self.latest_data = data
        distance = float(data.get('D', 0))
        now = time.time()

        # Activate or deactivate pattern based on sensor
        if distance > 50:
            self.active = True
        else:
            if self.active:
                # Stop note if it was playing
                if self.note_on:
                    self.midi_service.send_note(note=60, velocity=0)
                    self.note_on = False
                self.active = False
            return

        # If it's not time to do anything, skip
        if now < self.next_event_time:
            return

        # Generate velocity from distance
        velocity = min(max(int(distance / 8), 0), 127)

        if not self.note_on:
            self.midi_service.send_note(note=60, velocity=velocity)
            self.note_on = True
            self.next_event_time = now + 0.2  # note length
        else:
            self.midi_service.send_note(note=60, velocity=0)
            self.note_on = False
            self.next_event_time = now + 0.3  # rest time
