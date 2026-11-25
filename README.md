# ESP32 Device Test Automation

An embedded device testing project using **ESP32-C3, ESP-IDF, C++, Python and Robot Framework**.

The project demonstrates how to communicate with an embedded device over USB Serial/JTAG, control hardware through a command interface, and automate functional tests using Python and Robot Framework.

## Overview

The ESP32-C3 runs custom C++ firmware that exposes a simple serial command interface. A Python library communicates with the device over USB, while Robot Framework executes automated test cases and verifies the responses.

The project includes an external LED connected to GPIO10, allowing the tests to exercise actual hardware control.

## Architecture

```text
Robot Framework
      |
      v
Python Serial Library
      |
      v
USB Serial/JTAG
      |
      v
ESP32-C3 Firmware (C++)
      |
      v
GPIO10 -> External LED
```

## Hardware

- Seeed Studio XIAO ESP32-C3
- External LED
- 220 Ω current-limiting resistor
- USB cable
- Jumper wires

### LED wiring

Connect the LED in series with the resistor:

```text
GPIO10 ---- 220 Ω resistor ---- LED anode (+)
                                  LED cathode (-)
                                      |
                                     GND
```

The LED is active-high:
- GPIO HIGH: LED ON
- GPIO LOW: LED OFF

## Technologies

| Technology | Purpose |
|---|---|
| C++ | Embedded firmware |
| ESP-IDF 6.0 | Firmware development framework |
| ESP32-C3 | Target hardware |
| USB Serial/JTAG | Device communication |
| Python | Serial communication library |
| PySerial | USB serial interface |
| Robot Framework | Automated testing |

## Firmware commands

| Command | Expected response | Function |
|---|---|---|
| `PING` | `PONG` | Communication check |
| `GET_VERSION` | `VERSION:1.0.0` | Firmware version |
| `GET_UPTIME` | `UPTIME:<milliseconds>` | Device uptime |
| `LED ON` | `LED:ON` | Switch LED on |
| `LED OFF` | `LED:OFF` | Switch LED off |
| `GET_LED` | `LED:ON` or `LED:OFF` | Read LED state |
| Invalid command | `ERROR:UNKNOWN_COMMAND` | Error handling |

## Project structure

```text
esp32-device-test/
├── firmware/
│   ├── CMakeLists.txt
│   └── main/
│       ├── CMakeLists.txt
│       └── main.cpp
├── libraries/
│   └── ESP32Serial.py
├── tests/
│   └── device_test.robot
├── results/
├── requirements.txt
├── .gitignore
└── README.md
```

## Setup

### 1. Requirements

- ESP-IDF 6.0
- Python 3
- Robot Framework
- PySerial

### 2. Install Python dependencies

From the project root:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### 3. Build and flash firmware

Activate the ESP-IDF environment, then:

```bash
cd firmware

idf.py set-target esp32c3
idf.py build
idf.py -p /dev/cu.usbmodem101 flash
```

The serial device path may differ depending on your operating system and USB connection.

### 4. Run automated tests

Return to the project root and activate the Python virtual environment.

```bash
robot --pythonpath libraries -d results tests/device_test.robot
```

Robot Framework generates HTML and XML test reports in the `results/` directory.

## Automated tests

The test suite currently covers:

- Serial communication and device response
- Firmware version verification
- Uptime reporting
- LED ON/OFF control
- LED state reporting
- Invalid command handling
- LED blinking at different frequencies

## Test results

Initial test suite:

**6 tests passed, 0 failed.**

The LED blinking test was added afterwards and should be included in the next verified test run.

## Project goals

This project was developed to gain practical experience in:

- Embedded firmware testing
- Hardware-software interaction
- Serial communication protocols
- Python test automation
- Robot Framework test development
- Functional and negative testing

## Future improvements

- Add CI integration with Jenkins
- Improve serial communication error handling
- Expand automated hardware test coverage
- Add configurable test parameters

---

**Author:** Hannu  
**Focus:** Embedded Systems · Software Testing · IoT
