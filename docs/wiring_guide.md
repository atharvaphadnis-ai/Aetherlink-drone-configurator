# AetherLink Wiring Guide

Owner and author: **Atharva Phadnis**

## Target Board
- ESP32-S3-DevKitC-1

## Core Buses
- `SPI` (for SX1276/SX1280 or NRF24L01+): `SCK=GPIO36`, `MOSI=GPIO35`, `MISO=GPIO37`
- `UART1` (ELRS/CRSF): `TX=GPIO43`, `RX=GPIO44`
- `I2C` (MPU6050/9250): `SDA=GPIO8`, `SCL=GPIO18`

## RF Modules
- ELRS module power: `3V3`, `GND`, `MISO/MOSI/SCK`, `NSS`, `DIO0`, `RST`
- NRF24L01+ power: use regulated `3.3V` with local 10uF capacitor
- NRF24L01+ control pins used by default firmware: `CE=GPIO16`, `CSN=GPIO15`

## Output Signals
- PWM channels 1..8 default: `GPIO1,2,41,42,45,46,47,48`
- Additional PWM channels 9..16 are configurable in CLI/web config
- SBUS output: use a hardware inverter transistor stage for flight controllers expecting inverted SBUS

## Analog Inputs
- Battery ADC default: `GPIO4`
- Recommended divider ratio: `11:1` for 6S systems

## Bring-Up Sequence
1. Power board via USB first for safe bring-up.
2. Verify SoftAP `AetherLink-Setup` appears.
3. Open `http://192.168.4.1/status`.
4. Bind transmitter for desired active protocol.
5. Verify auto-lock and output channels.
