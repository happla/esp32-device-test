#// Author: happla

import time
import serial


class ESP32Serial:

    ROBOT_LIBRARY_SCOPE = "SUITE"

    def __init__(self):
        self.connection = None

    def connect(self, port, baudrate=115200, timeout=2):
        """Connect to the ESP32 over USB serial."""

        self.connection = serial.Serial(
            port=port,
            baudrate=int(baudrate),
            timeout=float(timeout),
            write_timeout=float(timeout)
        )

        time.sleep(0.5)
        self.connection.reset_input_buffer()

    def send_command(self, command):
        """Send a command and return the device response."""

        if self.connection is None or not self.connection.is_open:
            raise RuntimeError("ESP32 is not connected")

        self.connection.write((command + "\n").encode("utf-8"))
        self.connection.flush()

        response = self.connection.readline()

        if not response:
            raise TimeoutError(
                f"No response received for command: {command}"
            )

        return response.decode(
            "utf-8",
            errors="replace"
        ).strip()

    def close_connection(self):
        """Close the serial connection."""

        if self.connection and self.connection.is_open:
            self.connection.close()
            self.connection = None

