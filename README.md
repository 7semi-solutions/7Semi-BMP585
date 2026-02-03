# 7Semi BMP585 Arduino Library

The 7Semi BMP585 Arduino Library is a lightweight, register-level driver for the Bosch Sensortec BMP585 barometric pressure sensor. It supports both I2C and SPI communication on Arduino-core platforms and applies a safe default configuration automatically, allowing the sensor to work out-of-box without extra setup. The library also exposes advanced features such as oversampling control, output data rate, IIR filtering, interrupts, FIFO, out-of-range detection, and user NVM access.

---

##  Features

- Supports **I2C and SPI**
- Configurable:
  - Oversampling (OSR)
  - Output Data Rate (ODR)
  - IIR filtering
  - FIFO
  - Interrupts (DRDY, FIFO, OOR)
  - Out-of-Range (OOR) window
  - User NVM read/write
- Optional **custom I2C / SPI pins on ESP32**

---

##  Connections / Wiring

###  I²C Connection (Most Common)

| BMP585 Pin | MCU Pin | Notes |
|-----------|--------|------|
| VDD | 3.3V | **3.3V only** (do NOT use 5V) |
| GND | GND | Common ground |
| SDI | SDA | I²C data |
| SCK | SCL | I²C clock |
| INT | GPIO (optional) | Required only for interrupts |

---

###  SPI Connection

| BMP585 Pin | MCU Pin | Notes |
|-----------|--------|------|
| VDD | 3.3V | **3.3V only** |
| GND | GND | Common ground |
| SCK | SPI SCK | Clock |
| SDI | SPI MOSI | Master → sensor |
| SDO | SPI MISO | Sensor → master |
| CS | GPIO | Chip Select (active-low) |
| INT | GPIO (optional) | Interrupt output |

**SPI Notes**
- SPI mode **MODE0** required  
- Dummy byte is required after address (handled by library)  
- CS must stay LOW during the entire transaction  

Notes

SPI mode: MODE0

Start with 1 MHz SPI clock

Keep SPI wires short for reliability

##  Installation

### Arduino Library Manager
1. Open **Arduino IDE**
2. Go to **Library Manager**
3. Search for **7Semi BMP585**
4. Install

### Manual
1. Download this repository as ZIP
2. Arduino IDE → **Sketch → Include Library → Add .ZIP Library**

---
### Initialization

beginI2C() – Initializes BMP585 over I²C and verifies the chip ID.

beginSPI() – Initializes BMP585 over SPI (MODE0) and verifies the chip ID.

sensorInit() – Applies recommended default configuration (OSR, ODR, filters, power mode).

reset() – Performs a soft reset and restores default register values.

chipID() – Reads and returns the BMP585 chip ID.

revisionID() – Reads and returns the silicon revision ID.

### Configuration

setOSR() – Sets temperature and pressure oversampling and enables pressure measurement.

setODR() – Sets output data rate, power mode, and deep-standby behavior.

setPowerMode() – Changes the sensor power mode (Standby / Normal / Forced).

setIIR() – Configures IIR low-pass filters for temperature and pressure.

### Data Reading

readTemperatureC() – Reads temperature in degrees Celsius.

readPressurePa() – Reads pressure in Pascals.

readTemperaturePressure() – Reads temperature and pressure in one burst (recommended).

readAltitudeM() – Calculates altitude in meters using sea-level pressure reference.

#### FIFO

configureFIFO() – Configures FIFO frame type, decimation, threshold, and mode.

getFIFOCount() – Returns the number of frames currently stored in FIFO.

readFIFOFrame() – Reads one FIFO frame (temperature, pressure, or both).

clearFIFO() – Flushes all FIFO data.

### Interrupts

configureIntPin() – Configures interrupt pin behavior (latched, polarity, open-drain).

setIntSources() – Enables specific interrupt sources (DRDY, FIFO, OOR, POR).

readIntStatus() – Reads and clears interrupt status flags.

### Out-of-Range (OOR)

setOORWindow() – Configures pressure out-of-range detection window and trigger count.

### NVM (Non-Volatile Memory)

readNVMStatus() – Reads NVM ready/error status.

waitNVMReady() – Waits until NVM operation completes or timeout occurs.

readNVMUserRow() – Reads a user-programmable NVM row.

writeNVMUserRow() – Writes data to a user-programmable NVM row.

## ⚡ Quick Start (I2C)

```cpp
#include <Wire.h>
#include "7Semi_BMP585.h"

BMP585_7Semi bmp;

void setup() {
  Serial.begin(115200);

  if (!bmp.beginI2C(Wire, 0x46, 400000)) {
    Serial.println("BMP585 not found!");
    while (1);
  }
}

void loop() {
  float tC, pPa;

  if (bmp.readTemperaturePressure(tC, pPa)) {
    Serial.print("T = ");
    Serial.print(tC, 2);
    Serial.print(" C, P = ");
    Serial.print(pPa, 2);
    Serial.println(" Pa");
  }

  delay(200);
}
