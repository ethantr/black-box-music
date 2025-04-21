import sys
import time
from services.serial_service import SerialService, list_serial_ports
from services.midi_service import MidiService, list_midi_outputs
from services.mode_service import ModeService

def launch_app():
    print("=== Sensor to MIDI CLI Interface ===")

    # Services
    midi_service = MidiService()
    mode_service = ModeService(midi_service)
    serial_service = SerialService(mode_service.process)

    current_serial = None
    current_midi = None

    # --- CLI Loop ---
    while True:
        print("\nOptions:")
        print("[S] Start connection")
        print("[D] Disconnect")
        print("[M] Set MIDI output")
        print("[P] Set Serial port")
        print("[L] List sensor data (live)")
        print("[Q] Quit")

        choice = input("Select: ").strip().lower()

        if choice == 's':
            if not current_serial or not current_midi:
                print("⚠️  Please set both MIDI and Serial ports first.")
                continue
            try:
                midi_service.connect(current_midi)
                serial_service.connect(current_serial)
                print(f"✅ Connected {current_serial} → {current_midi}")
            except Exception as e:
                print(f"❌ Connection error: {e}")

        elif choice == 'd':
            serial_service.disconnect()
            midi_service.disconnect()
            print("🔌 Disconnected")

        elif choice == 'm':
            outputs = list_midi_outputs()
            print("\nAvailable MIDI Outputs:")
            for i, port in enumerate(outputs):
                print(f"[{i}] {port}")
            idx = input("Select MIDI port #: ").strip()
            try:
                current_midi = outputs[int(idx)]
                print(f"🎹 Selected MIDI: {current_midi}")
            except Exception:
                print("❌ Invalid selection.")

        elif choice == 'p':
            ports = list_serial_ports()
            print("\nAvailable Serial Ports:")
            for i, port in enumerate(ports):
                print(f"[{i}] {port}")
            idx = input("Select Serial port #: ").strip()
            try:
                current_serial = ports[int(idx)]
                print(f"🧭 Selected Serial: {current_serial}")
            except Exception:
                print("❌ Invalid selection.")

        elif choice == 'l':
            print("🔄 Listening for sensor values (Ctrl+C to stop)...")
            try:
                while True:
                    time.sleep(1)
            except KeyboardInterrupt:
                print("\n⏹️ Stopped live display.")

        elif choice == 'q':
            serial_service.disconnect()
            midi_service.disconnect()
            print("👋 Exiting.")
            sys.exit()

        else:
            print("❓ Invalid option.")
