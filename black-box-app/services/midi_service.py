import mido

def list_midi_outputs():
    return mido.get_output_names()

class MidiService:
    def __init__(self):
        self.outport = None

    def connect(self, port_name):
        self.outport = mido.open_output(port_name)

    def disconnect(self):
        if self.outport:
            self.outport.close()
            self.outport = None
        else:
            print("MIDI output port is not open or already closed.")
    
    def auto_find_midi_port(self):
        """
        Attempts to auto-detect a connected MIDI output port.
        Returns the port name (e.g., 'MIDI Device'), or None if not found.
        """
        ports = list_midi_outputs()
        for p in ports:
            # Here you can add logic to filter specific MIDI devices if needed
            return p
        return None


    def send_note(self, note, velocity, channel=0):
        if self.outport:
            msg = mido.Message('note_on', note=note, velocity=velocity, channel=channel)
            self.outport.send(msg)

    def send_note_off(self, note, velocity=0, channel=0):
        if self.outport:
            msg = mido.Message('note_off', note=note, velocity=velocity, channel=channel)
            self.outport.send(msg)


    def send_control_change(self, control, value, channel=0):
        msg = mido.Message('control_change', control=control, value=value, channel=channel)
        self.outport.send(msg)

    def send_pitch_bend(self, value, channel=0):
    
        value = int(min(max(value, -8192), 8191)) + 8192  # Convert to 14-bit unsigned
        msg = mido.Message('pitchwheel', pitch=value, channel=channel)
        self.output.send(msg)

    