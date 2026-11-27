# Author: happla

import time
import serial
from serial.tools import list_ports


class ESP32Serial:

    ROBOT_LIBRARY_SCOPE = "SUITE"

    def __init__(self):
        self.connection = None

    def find_esp32_port(self):
        """Automatically discover the ESP32-C3 USB Serial/JTAG device."""

        ports = list_ports.comports()

        matches = [
            port for port in ports
            if port.vid == 0x303A and port.pid == 0x1001
        ]

        if not matches:
            raise RuntimeError("ESP32-C3 not found")

        if len(matches) > 1:
            devices = [port.device for port in matches]
            raise RuntimeError(
                f"Multiple ESP32 devices found: {devices}"
            )

        return matches[0].device

    def connect_to_device(self, baudrate=115200, timeout=2):
        """Discover and connect to the ESP32-C3."""

        port = self.find_esp32_port()

        self.connection = serial.Serial(
            port=port,
            baudrate=int(baudrate),
            timeout=float(timeout),
            write_timeout=float(timeout)
        )

        time.sleep(0.5)
        self.connection.reset_input_buffer()

        print(f"Connected to ESP32-C3 on {port}")

    def send_command(self, command):
        """Send a command and return the device response."""

        if self.connection is None or not self.connection.is_open:
            raise RuntimeError("Device is not connected")

        self.connection.write((command + "\n").encode("utf-8"))
        self.connection.flush()

        response = self.connection.readline()

        if not response:
            raise TimeoutError(
                f"No response received for command: {command}"
            )

        return response.decode("utf-8", errors="replace").strip()

    def close_connection(self):
        """Close the serial connection."""

        if self.connection and self.connection.is_open:
            self.connection.close()

        self.connection = None

