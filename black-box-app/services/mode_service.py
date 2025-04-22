from .modes.box_one_mode import BoxOneMode
from .modes.box_two_mode import BoxTwoMode

class ModeService:
    def __init__(self, midi_service):
        self.modes = {
            'Box 1': BoxOneMode(midi_service),
            'Box 2': BoxTwoMode(midi_service),
            
            # 'Other1': OtherMode1(midi_service),
            # 'Other2': OtherMode2(midi_service),
        }
        self.active_mode = self.modes['Box 1']  # Default mode

    def set_mode(self, name):
        if name in self.modes:
            self.active_mode = self.modes[name]

    def process(self, data):
        self.active_mode.process(data)

    def tick(self):
        """Call this regularly from the main loop."""
        self.active_mode.tick()

    def list_modes(self):
        return list(self.modes.keys())
