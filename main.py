import tkinter as tk
from tkinter import ttk
import serial
import mido
from threading import Thread
import serial.tools.list_ports

class MidiSensorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Sensor to MIDI Interface")

        # Serial port dropdown
        self.serial_var = tk.StringVar()
        self.serial_dropdown = ttk.Combobox(root, textvariable=self.serial_var, width=30)
        self.serial_dropdown['values'] = self.find_serial_ports()
        self.serial_dropdown.pack(pady=5)

        # MIDI output dropdown
        self.midi_var = tk.StringVar()
        self.midi_dropdown = ttk.Combobox(root, textvariable=self.midi_var, width=30)
        self.midi_dropdown['values'] = mido.get_output_names()
        self.midi_dropdown.pack(pady=5)

        # Start button
        self.start_button = tk.Button(root, text="Start", command=self.start)
        self.start_button.pack(pady=5)


        # Sensor value displays
        self.pressure_val = tk.StringVar(value="Pressure: -")
        self.distance_val = tk.StringVar(value="Distance: -")
        self.noise_val = tk.StringVar(value="Noise: -")

        tk.Label(root, textvariable=self.pressure_val).pack(pady=2)
        tk.Label(root, textvariable=self.distance_val).pack(pady=2)
        tk.Label(root, textvariable=self.noise_val).pack(pady=2)


        # Status display
        self.status_text = tk.StringVar()
        self.status_label = tk.Label(root, textvariable=self.status_text)
        self.status_label.pack(pady=5)

        self.running = False

    def find_serial_ports(self):
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def start(self):
        port = self.serial_var.get()
        midi_port = self.midi_var.get()
        if not port or not midi_port:
            self.status_text.set("Please select both serial and MIDI ports.")
            return

        try:
            self.ser = serial.Serial(port, 9600)
            self.midi_out = mido.open_output(midi_port)
            self.running = True
            self.status_text.set(f"Connected to {port} & {midi_port}")
            Thread(target=self.read_serial).start()
        except Exception as e:
            self.status_text.set(f"Error: {e}")

    def read_serial(self):
        while self.running:
            try:
                line = self.ser.readline().decode('utf-8').strip()
                data = dict(item.split(":") for item in line.split(","))
                pressure = float(data["P"])
                distance = float(data["D"])
                noise = float(data["N"])
                velocity = min(max(int(pressure / 8), 0), 127)
                self.midi_out.send(mido.Message('note_on', note=60, velocity=velocity))
                self.pressure_val.set(f"Pressure: {pressure}")
                self.distance_val.set(f"Distance: {distance}")
                self.noise_val.set(f"Noise: {noise}")
            except Exception as e:
                self.status_text.set(f"Read error: {e}")

root = tk.Tk()
app = MidiSensorApp(root)
root.mainloop()
