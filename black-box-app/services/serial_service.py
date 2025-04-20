import serial
import serial.tools.list_ports
from threading import Thread


def list_serial_ports():
    """
    Return a list of available serial ports with their descriptions.
    Format: "COM3 - Arduino Uno" or "/dev/ttyACM0 - Arduino".
    """
    ports = serial.tools.list_ports.comports()
    return [f"{p.device} - {p.description}" for p in ports]


def auto_find_port():
    """
    Attempt to auto-detect a connected Arduino serial port.
    Returns the port name (e.g., 'COM3' or '/dev/ttyACM0'), or None if not found.
    """
    ports = serial.tools.list_ports.comports()
    for p in ports:
        desc = p.description.lower()
        dev = p.device.lower()
        if 'arduino' in desc or 'ttyacm' in dev or 'ttyusb' in dev:
            return f"{p.device} - {p.description}"
    return None

class SerialService:
    def __init__(self, callback):
        self.callback = callback
        self.ser = None

    def connect(self, port=None, baudrate=9600):
        """
        Connect to the given serial port. If port is None, attempt auto-find.
        """
        if port is None:
            port = auto_find_port()
            if port is None:
                raise IOError("No Arduino-like device found on serial ports.")
        # If passed a display string "COM3 - ...", extract actual device
        if ' - ' in port:
            port = port.split(' - ')[0]
        self.ser = serial.Serial(port, baudrate)
        Thread(target=self._read_loop, daemon=True).start()

    def disconnect(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            self.ser = None
        else:
            print("Serial port is not open or already closed.")

    def _read_loop(self):
        while True:
            if not self.ser or not self.ser.is_open:
                break
            try:
                line = self.ser.readline().decode('utf-8').strip()
                print(f"Serial read: {line}")  # Debugging line
                data = dict(item.split(":") for item in line.split(",") if ":" in item)
                self.callback(data)
            except Exception as e:
                print(f"Serial read error: {e}")
