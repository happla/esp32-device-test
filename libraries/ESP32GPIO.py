# Author: happla
from robot.libraries.BuiltIn import BuiltIn

class ESP32GPIO:
    """Robot Framework keywords for ESP32 GPIO measurements."""

    ROBOT_LIBRARY_SCOPE = "SUITE"

    def __init__(self, serial_library=None):
        self.serial_library = serial_library

    def measure_led_pulses(self, duration):
        """Count GPIO rising edges during a measurement window."""

        if self.serial_library is None:
            raise RuntimeError("ESP32Serial library is not configured")

        response = self.serial_library.send_command(
            f"MEASURE_PULSES {duration}"
        )

        if not response.startswith("PULSES:"):
            raise RuntimeError(
                f"Unexpected pulse measurement response: {response}"
            )

        return int(response.split(":", 1)[1])

    def count_led_blinks(self):
        """Count complete LED blink cycles without frequency validation."""

        if self.serial_library is None:
            raise RuntimeError("ESP32Serial library is not configured")

        response = self.serial_library.send_command("COUNT_BLINKS")

        if not response.startswith("BLINKS:"):
            raise RuntimeError(
                f"Unexpected blink count response: {response}"
            )

        return int(response.split(":", 1)[1])


from robot.libraries.BuiltIn import BuiltIn


class ESP32GPIO:
    """Robot Framework keywords for ESP32 GPIO measurements."""

    ROBOT_LIBRARY_SCOPE = "SUITE"

    def __init__(self):
        self.serial_library = None

    def _get_serial_library(self):
        """Get the existing ESP32Serial library instance."""

        if self.serial_library is None:
            self.serial_library = (
                BuiltIn().get_library_instance("ESP32Serial")
            )

        return self.serial_library

    def measure_led_pulses(self, duration, frequency=5):
        """Count GPIO rising edges during a measurement window."""

        command = f"MEASURE_PULSES {duration} {frequency}"
        response = self._get_serial_library().send_command(command)

        if not response.startswith("PULSES:"):
            raise RuntimeError(
                f"Unexpected pulse measurement response: {response}"
            )

        try:
            return int(response.split(":", 1)[1])
        except ValueError as exc:
            raise RuntimeError(
                f"Invalid pulse count: {response}"
            ) from exc

    def count_led_blinks(self, cycles=5):
        """Count complete LED blink cycles without frequency validation."""

        command = f"COUNT_BLINKS {cycles}"
        response = self._get_serial_library().send_command(command)

        if not response.startswith("BLINKS:"):
            raise RuntimeError(
                f"Unexpected blink count response: {response}"
            )

        try:
            return int(response.split(":", 1)[1])
        except ValueError as exc:
            raise RuntimeError(
                f"Invalid blink count: {response}"
            ) from exc

