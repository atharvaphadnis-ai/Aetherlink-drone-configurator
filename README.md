# AetherLink Firmware

**Universal RC Receiver and Protocol Translator for ESP32-S3**  
Author and IP Owner: **Atharva Phadnis**  
License: **GPL-3.0-or-later** with commercial exception (see `LICENSE`)

---

## 1) Project Vision

The RC and robotics ecosystem is highly fragmented: one receiver for ELRS, another for legacy toy protocols, another for SBUS/PPM output compatibility, and often no clean bridge between modern radio links and older flight controllers.

**AetherLink** solves this by turning a single ESP32-S3 into a universal, real-time receiver-translator firmware. It detects active input links, normalizes control frames into a unified internal representation, and re-emits deterministic outputs (PWM, CPPM, SBUS, iBUS, CRSF) for broad compatibility across drones, planes, boats, and robotics platforms.

This repository is intended as an industrial-grade starting point for:
- universal receiver designs,
- integration in mixed-generation autopilot stacks,
- resilient field systems requiring predictable fail-safe behavior,
- certifiable architecture progression (safety-oriented development practices).

---

## 2) Key Capabilities

- **Multi-protocol input layer** (modular):
  - ELRS-like serial receiver path (CRC-protected frame ingestion)
  - ESP-NOW low-latency peer link with PMK setup and payload integrity checks
  - NRF24L01+ receive path with compact payload CRC checks
  - BLE initialization and GATT scaffolding for configurability

- **Universal output layer**:
  - Up to **16 PWM channels**
  - **CPPM/PPM** combined stream
  - **SBUS**
  - **iBUS**
  - **CRSF**-style serial output framing

- **Real-time scheduling model (FreeRTOS)**:
  - dedicated receiver task
  - high-priority output task
  - telemetry task
  - control/configuration task

- **Fail-safe framework**:
  - HOLD / CUT / CUSTOM fail-safe policies
  - timeout-driven deterministic actuation behavior

- **Telemetry hub**:
  - RSSI forwarding
  - battery voltage from ADC
  - IMU-assisted roll/pitch estimates (MPU6050 path in current implementation)

- **Configuration channels**:
  - serial CLI
  - BLE service scaffold
  - Wi-Fi softAP web status/toggle interface

- **State persistence**:
  - NVS (Preferences) restore/save for key runtime configuration.

---

## 3) Repository Layout

```text
AetherLink_Firmware/
├── platformio.ini
├── README.md
├── LICENSE
├── lib/
│   ├── Protocol_ELRS/
│   ├── Protocol_SBUS_ibus/
│   ├── Protocol_ESPNOW/
│   ├── Protocol_NRF24/
│   ├── Output_PWM_PPM/
│   └── Telemetry/
├── src/
│   ├── main.cpp
│   ├── AetherLink_Config.h
│   ├── hardware_init.cpp
│   ├── hardware_init.h
│   ├── failsafe_manager.cpp
│   ├── failsafe_manager.h
│   ├── web_interface.cpp
│   └── web_interface.h
├── docs/
│   ├── wiring_guide.md
│   └── api_reference.md
└── tests/
    └── test_sbus_parser/
        └── test_main.cpp
```

---

## 4) Platform and Toolchain

### Target hardware
- **ESP32-S3 DevKitC-1** (default in `platformio.ini`)

### Framework
- Arduino core on Espressif32 platform (with ESP-IDF internals leveraged via Arduino environment)

### Build system
- PlatformIO

### Dependencies (declared)
- `ArduinoJson`
- `NimBLE-Arduino`
- `RF24`
- `CRC`
- `ESP32Servo`

> Note: Ensure `platformio` CLI is installed and available in your shell path for command-line builds.

---

## 5) Firmware Architecture Overview

The system follows an **event-driven, preemptive multitasking model**:

1. **ReceiverTask** acquires frames from the active input protocol.
2. Frames are placed into RTOS queues (overwrite semantics ensure newest control frame dominates).
3. **OutputTask** consumes frames and writes output protocol(s) with fail-safe guarding.
4. **TelemetryTask** periodically packages downlink status.
5. **ControlTask** handles web requests, CLI commands, and bridge servicing.

### Why this design

- **Determinism first:** high-priority output processing avoids stale actuation.
- **Freshest frame policy:** queue overwrite behavior prevents backlog latency.
- **Isolation of concerns:** radio I/O, actuator output, telemetry, and management are decoupled tasks.
- **Safety posture:** watchdog servicing and bounded periodic loops reduce lockup risk.

---

## 6) FreeRTOS Tasking and Priority Model

Current task priorities in `AetherLink_Config.h`:

- `OutputTask`: priority **7**
- `ReceiverTask`: priority **6**
- `TelemetryTask`: priority **4**
- `ControlTask`: priority **3**

### Rationale

- Output holds highest priority to minimize actuator jitter and prevent control dropouts.
- Receiver follows closely to reduce input capture latency.
- Telemetry and control are lower priority because they are non-critical relative to control loop actuation.

### Timing assumptions and latency

With current task loop delays:
- Receiver poll period: ~`2 ms`
- Output loop period: ~`4 ms`
- Queue transfer overhead: sub-millisecond

Approximate worst-case ELRS frame to PWM actuation:
- **~6.1 ms**

This is appropriate for most RC control use cases and leaves headroom for further optimization.

---

## 7) Protocol Pipeline

### Input side

#### ELRS module path
- UART ingestion at high baud
- per-frame CRC verification
- channel normalization to internal microsecond representation

#### ESP-NOW path
- callback-driven receive
- PMK setup (security baseline)
- compact payload integrity verification

#### NRF24 path
- RF24 listening pipe
- payload extraction and CRC validation
- channel limit enforcement

### Unified internal frame

All inputs map to:
- `timestamp_ms`
- `channels[16]`
- `rssi`
- `frame_valid`

This common frame allows clean protocol translation without output-specific radio coupling.

### Output side

- PWM writes via servo abstraction
- CPPM via pulse framing on configured pin
- SBUS/iBUS/CRSF via UART packet formatting

---

## 8) Auto-Detect and Input Lock Behavior

At startup, firmware attempts to detect active links and lock onto a protocol based on link health checks:
- ELRS linked?
- ESP-NOW bound?
- NRF24 linked?

When detected, the active input lock is set in runtime config and used by receiver logic. Runtime persistence is handled in NVS for retained configuration behavior across reboots.

---

## 9) Fail-Safe Model

The fail-safe manager enforces deterministic behavior when signal age exceeds timeout:

- `HOLD` – preserve last known control state
- `CUT` – force throttle channel low
- `CUSTOM` – apply preconfigured channel positions

### Safety note

For aircraft and high-energy robotics, define explicit custom fail-safe values for all critical channels before field deployment.

---

## 10) Telemetry and Sensor Integration

Telemetry currently includes:
- RSSI
- battery voltage (ADC + divider scaling)
- roll/pitch from MPU6050-derived accelerometer estimate (smoothed)

If IMU is unavailable, fallback estimates are derived from channel deltas. This maintains continuity for telemetry payload generation while exposing degraded sensor confidence behavior.

---

## 11) Wi-Fi and Web Interface

Firmware brings up a softAP:
- SSID: `AetherLink-Setup`
- Password: `AetherLink123`

Web endpoints:
- `GET /status` -> JSON with input mode/output mode/fail-safe mode/bridge state
- `POST /bridge/toggle` -> toggles UART-over-Wi-Fi bridge mode

Bridge mode allows forwarding UART traffic through TCP server behavior for ground station workflows.

---

## 12) CLI Commands (Current Baseline)

Over serial (`115200`), supported commands include:

- `set failsafe hold`
- `set failsafe cut`
- `set output sbus`
- `set output ibus`
- `set output crsf`
- `set output cppm`
- `save`

Suggested enhancement: migrate to tokenized parser with explicit argument validation and response codes for production tooling integration.

---

## 13) Build, Flash, and Monitor

## Prerequisites
- Python 3.x
- PlatformIO CLI (`pip install platformio`) or PlatformIO IDE extension

### Build

```bash
platformio run
```

### Upload

```bash
platformio run --target upload
```

### Serial monitor

```bash
platformio device monitor -b 115200
```

If your shell uses `pio` instead:

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

---

## 14) Unit Testing

A Unity test validates SBUS parser behavior:
- accepts valid frame structure,
- rejects malformed header frame.

Run tests:

```bash
platformio test
```

Test location:
- `tests/test_sbus_parser/test_main.cpp`

---

## 15) Wiring and API Documentation

Detailed references:
- Wiring: `docs/wiring_guide.md`
- APIs and timing notes: `docs/api_reference.md`

These documents are intended to be maintained alongside firmware evolution and hardware revisions.

---

## 16) Security Posture (Current and Recommended)

### Implemented baseline
- payload integrity checks (CRC/XOR style in current protocol modules)
- ESP-NOW PMK setup
- explicit protocol and configuration boundaries

### Recommended hardening roadmap
- protocol-specific cryptographic authentication for bind/session establishment
- key rotation and secure storage strategy
- replay protection (nonce/counter verification)
- signed configuration updates
- authenticated web/CLI control plane

For IEC 62443-aligned deployments, treat this repo as a strong foundation but continue with product-level threat modeling and secure lifecycle controls.

---

## 17) Memory and Reliability Strategy

- static/global buffers in core datapaths
- no heap-heavy control loops in fast tasks
- watchdog feeds in every major task
- bounded periodic loops with deterministic delays

Further improvement opportunities:
- explicit stack watermark telemetry per task
- static analysis integration in CI
- long-duration soak tests with RF impairment simulation

---

## 18) MISRA and Safety-Certification Direction

The codebase is structured with safety intent:
- clear module boundaries
- explicit enums and fixed-size channel arrays
- deterministic fail-safe logic
- minimal hidden state coupling across modules

To advance toward strict MISRA conformance and certification artifacts:
- eliminate dynamic string usage in runtime command handling,
- tighten implicit conversions and cast annotations,
- add formal requirements traceability,
- document unit/integration test coverage against safety requirements,
- enforce static analysis gates in CI (e.g., Cppcheck/clang-tidy configured for MISRA profiles).

---

## 19) Known Limitations and Scope Notes

This project is an advanced implementation baseline and reference architecture. Depending on your final product requirements, you should validate and potentially extend:

- exact framing details against target transmitter/FC expectations for all protocols,
- hardware inversion stage requirements for specific SBUS consumers,
- full ELRS v3 feature parity (including complete telemetry dialect behavior),
- IMU fusion depth (currently lightweight accelerometer-derived estimate),
- BLE HID profile completeness beyond current BLE service scaffold.

---

## 20) Production Deployment Checklist

Before flight or field use:

1. Verify input protocol lock is stable under RF stress.
2. Verify output protocol timing with logic analyzer.
3. Validate fail-safe action physically on propulsion-disarmed bench setup.
4. Calibrate battery divider and ADC scaling against known reference.
5. Run extended soak test (temperature and brownout scenarios included).
6. Confirm watchdog recovery behavior in induced fault cases.
7. Persist and reboot-verify all required configuration values.

---

## 21) Contribution and Ownership

All original work and IP attribution in this project belongs to:

**Atharva Phadnis**

If you plan to commercialize derivatives, obtain an explicit commercial exception/license from the owner as described in `LICENSE`.

For technical contributions:
- keep module boundaries clean,
- preserve deterministic behavior in control paths,
- include tests for any parser/protocol modifications,
- update docs for every externally visible behavior change.

---

## 22) Quick Start (Recommended First Boot)

1. Build and flash firmware to ESP32-S3.
2. Connect to `AetherLink-Setup` AP.
3. Open `http://192.168.4.1/status`.
4. Power transmitter/module and confirm protocol lock.
5. Observe actuator outputs without propellers attached.
6. Set desired output mode via CLI and `save`.
7. Reboot and verify persisted behavior.

---

## 23) Disclaimer

This firmware is provided in good faith for advanced embedded and RC integration use. You are responsible for validating behavior in your final electrical, mechanical, and operational environment. Always perform staged safety testing before mission or flight deployment.
