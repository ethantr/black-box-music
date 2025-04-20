import tkinter as tk
from tkinter import ttk
from services.serial_service import SerialService, auto_find_port, list_serial_ports
from services.midi_service import MidiService, list_midi_outputs


def launch_app():
    root = tk.Tk()
    root.title("Sensor to MIDI Interface")
    root.configure(bg="#f4f4f4")  # Soft background

    style = ttk.Style(root)
    style.theme_use('clam')

    # Base styles
    style.configure('TFrame', background="#f4f4f4")
    style.configure('TLabelframe', background="#f4f4f4", foreground="#333", font=('Segoe UI', 10, 'bold'))
    style.configure('TLabelframe.Label', background="#f4f4f4", foreground="#444")
    style.configure('TLabel', background="#f4f4f4", foreground="#333", font=('Segoe UI', 10))
    style.configure('TButton', font=('Segoe UI', 10), padding=6)
    style.configure('Accent.TButton', background='#007acc', foreground='white')
    style.map('Accent.TButton', background=[('active', '#005f99')])
    style.configure('TCombobox', padding=4)

    # Sunken status bar
    style.configure('Status.TLabel', relief='sunken', anchor='w', background="#e0e0e0", padding=(6, 4))
    app = MidiSensorApp(root)
    root.mainloop()


class MidiSensorApp:
    def __init__(self, root):
        self.root = root
        self.running = False
        # Services
        self.serial_service = SerialService(self.handle_sensor_data)
        self.midi_service = MidiService()

        # Variables
        self.serial_var = tk.StringVar()
        self.midi_var = tk.StringVar()
        self.pressure_val = tk.StringVar(value='-')
        self.distance_val = tk.StringVar(value='-')
        self.noise_val = tk.StringVar(value='-')
        self.status_text = tk.StringVar(value='Ready')

        self._configure_grid()
        self._build_ui()

        # Auto-select detected Arduino port if available
        auto_port = auto_find_port()
        if auto_port:
            self.serial_var.set(auto_port)

        auto_midi_port = self.midi_service.auto_find_midi_port()
        if auto_midi_port:
            self.midi_var.set(auto_midi_port)

    def _configure_grid(self):
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)

    def _build_ui(self):
        # Main container
        container = ttk.Frame(self.root, padding=12)
        container.grid(row=0, column=0, sticky='nsew')
        container.columnconfigure(0, weight=1)

        # Connection frame
        conn_frame = ttk.Labelframe(container, text='Connection', padding=10)
        conn_frame.grid(row=0, column=0, sticky='ew', pady=8)
        conn_frame.columnconfigure(1, weight=1)

        ttk.Label(conn_frame, text='Serial Port:').grid(row=0, column=0, sticky='w')
        self.serial_dropdown = ttk.Combobox(
            conn_frame, textvariable=self.serial_var, values=list_serial_ports())
        self.serial_dropdown.grid(row=0, column=1, sticky='ew', padx=6)

        ttk.Label(conn_frame, text='MIDI Output:').grid(row=1, column=0, sticky='w', pady=(8,0))
        self.midi_dropdown = ttk.Combobox(
            conn_frame, textvariable=self.midi_var, values=list_midi_outputs())
        self.midi_dropdown.grid(row=1, column=1, sticky='ew', padx=6, pady=(8,0))
        ttk.Button(conn_frame, text='Start',style='Accent.TButton', command=self.start).grid(
            row=2, column=0, columnspan=2, sticky='ew', pady=(12,0))
        ttk.Button(conn_frame, text='Disconnect', command=self.stop).grid(
            row=3, column=0, columnspan=2, sticky='ew', pady=(6,0))

        # Sensor values frame
        sensor_frame = ttk.Labelframe(container, text='Sensor Values', padding=10)
        sensor_frame.grid(row=1, column=0, sticky='ew', pady=8)
        sensor_frame.columnconfigure(1, weight=1)

        sensors = [('Pressure:', self.pressure_val),
                    ('Distance:', self.distance_val),
                    ('Noise:', self.noise_val)]
        for idx, (label, var) in enumerate(sensors):
            ttk.Label(sensor_frame, text=label).grid(row=idx, column=0, sticky='w')
            ttk.Label(sensor_frame, textvariable=var).grid(row=idx, column=1, sticky='e', padx=6)

        # Status bar
        status_bar = ttk.Label(self.root, textvariable=self.status_text, style='Status.TLabel')
        status_bar.grid(row=1, column=0, sticky='ew')

    def start(self):
        port = self.serial_var.get()
        midi_port = self.midi_var.get()
        if not port or not midi_port:
            self.status_text.set("Please select both ports.")
            return
        try:
            self.midi_service.connect(midi_port)
            self.serial_service.connect(port)
            self.status_text.set(f"Connected: {port} → {midi_port}")
        except Exception as e:
            self.status_text.set(f"Connection error: {e}")

    def stop(self):
        try:
            self.serial_service.disconnect()
            self.midi_service.disconnect()
            self.status_text.set('Disconnected')
        except Exception as e:
            self.status_text.set(f"Error: {e}")

    def handle_sensor_data(self, data):
        try:
            pressure = float(data.get('P', 0))
            distance = float(data.get('D', 0))
            noise = float(data.get('N', 0))

            # Map pressure to MIDI velocity
            velocity = min(max(int(pressure / 8), 0), 127)
            self.midi_service.send_note(note=60, velocity=velocity)

            # Update UI
            self.pressure_val.set(f"{pressure:.1f}")
            self.distance_val.set(f"{distance:.1f}")
            self.noise_val.set(f"{noise:.1f}")
        except Exception as e:
            self.status_text.set(f"Read error: {e}")
