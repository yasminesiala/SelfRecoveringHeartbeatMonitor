# Self-Recovering Heartbeat Monitor

An Arduino-based heartbeat monitor designed to remain reliable under noisy signals, sensor faults, and timing failures.

## Features
- Fixed-rate sampling with real-time beat detection and BPM calculation
- Adaptive thresholding to handle changing signal quality
- Fault detection:
  - no-beat timeouts
  - stuck or disconnected sensor signals
  - timing overruns
- Self-recovery mechanisms:
  - automatic signal recalibration
  - safe-mode operation under repeated faults
  - watchdog-enforced reset as a last resort
- In-memory event log (ring buffer) for capturing recent system state
- Optional SD card logging for persistent event reports
- Optional fault-injection mode to test recovery behavior

## Wiring (Arduino Nano & analog pulse sensor)
- Sensor VCC → 3.3V
- Sensor GND → GND
- Sensor SIG → A0
- LED → D6 (or set `PIN_LED = 13` for built-in LED)
- Optional buzzer → D8
- Optional SD module:
  - CS → D10, MOSI → D11, MISO → D12, SCK → D13

## Configuration
Edit `Pins.hpp` to:
- assign hardware pins
- enable or disable SD logging
- enable or disable fault-injection mode
