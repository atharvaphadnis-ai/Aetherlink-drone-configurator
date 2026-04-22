# AetherLink API Reference

Owner and author: **Atharva Phadnis**

## Key Types
- `AetherLink::RxFrame`: canonical 16-channel receiver frame with timestamp and RSSI.
- `AetherLink::RuntimeConfig`: runtime configuration persisted in NVS.

## Protocol Interfaces
- `ELRS::Init(HardwareSerial&)`
- `ELRS::Poll(RxFrame&)`
- `ESPNOWProto::Init()`
- `ESPNOWProto::Poll(RxFrame&)`
- `NRF24Proto::Init(uint8_t ce_pin, uint8_t csn_pin)`
- `NRF24Proto::Poll(RxFrame&)`

## Output Interfaces
- `Output::Init(const RuntimeConfig&)`
- `Output::WriteChannels(const RxFrame&, const RuntimeConfig&)`
- `Output::WriteSbus(const RxFrame&, HardwareSerial&)`
- `Output::WriteIbus(const RxFrame&, HardwareSerial&)`

## Failsafe
- `ApplyFailsafeIfNeeded(RxFrame& frame, uint32_t last_rx_ms, const RuntimeConfig& config)`

## Web API
- `GET /status`: runtime mode, failsafe mode, bridge status.
- `POST /bridge/toggle`: toggles Wi-Fi bridge mode state.

## Timing and Latency Budget
Given the current scheduler periods:
- Receiver task polling period: `2 ms`
- Output task period: `4 ms`
- Queue handoff: `< 0.1 ms` in-core

Worst-case ELRS-to-PWM latency approximation:
`Tmax = 2.0 + 0.1 + 4.0 = 6.1 ms`

This satisfies low-latency fixed-wing and rover control expectations.
