import time
from .base_mode import MidiMode
from ..midi_service import MidiService

class BoxTwoMode(MidiMode):
    def __init__(self, midi_service: MidiService, bpm=120):
        super().__init__(midi_service)
        self.bpm = bpm
        self.step_duration = 60.0 / self.bpm  # in seconds

        self.pattern = [
            [48,60],           # C4
            [62],           # D4
            [60, 64, 67],   # C major chord (C4, E4, G4)
            [65],           # F4
        ]

        self.active = False
        self.step_index = 0
        self.last_step_time = time.time()
        self.note_on = False
        self.latest_data = None

    def process(self, data):
        self.latest_data = data
        distance = float(data.get('D', 0))

        if distance < 100:
            if not self.active:
                self.active = True
                self.step_index = 0
                self.last_step_time = time.time()
        else:
            if self.active:
                self._turn_off_current_notes()
                self.active = False

    def tick(self):
        """Call this regularly from the main loop."""
        if not self.active:
            return

        now = time.time()
        if now - self.last_step_time >= self.step_duration:
            # Step forward
            self._turn_off_current_notes()

            notes = self.pattern[self.step_index % len(self.pattern)]
            velocity = self._get_velocity_from_distance()
            # self._pitch_bend_from_noise()
            for note in notes:
                self.midi_service.send_note(note=note, velocity=velocity)

            self.note_on = True
            self.last_step_time = now
            self.step_index += 1

    def _turn_off_current_notes(self):
        if not self.note_on:
            return
        notes = self.pattern[self.step_index % len(self.pattern)]
        for note in notes:
            self.midi_service.send_note_off(note=note)
        self.note_on = False

    def _get_velocity_from_distance(self):
        if self.latest_data:
            distance = float(self.latest_data.get('D', 0))

            return 127 - int(distance/2)  # Inverse relationship
        return 64
    
    def _pitch_bend_from_noise(self):
        if self.latest_data:
            noise = float(self.latest_data.get("N", 0))
            # Clamp and scale noise: assume noise is 0–100
            norm_noise = min(max(noise, 0), 100) / 100  # 0.0 to 1.0
            pitch_bend = int((norm_noise * 2 - 1) * 8191)  # -8191 to +8191
            self.midi_service.send_pitch_bend(value=pitch_bend)
