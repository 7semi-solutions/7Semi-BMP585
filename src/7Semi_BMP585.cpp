#include "7Semi_BMP585.h"

BMP585_7Semi::BMP585_7Semi() {}

bool BMP585_7Semi::beginI2C(TwoWire &wire,
                             uint8_t i2cAddr,
                             uint32_t i2cSpeed,
                             uint8_t i2cSDA,
                             uint8_t i2cSCL)
{
  /**
   * Start sensor on I2C
   * - Stores Wire instance + address
   * - Sets bus speed
   * - Verifies address ACK and CHIP_ID
   * - Applies default sensor configuration so it works out-of-box
   *
   * Pin notes:
   * - Most Arduino cores have fixed I2C pins
   * - ESP32 allows custom SDA/SCL via Wire.begin(sda,scl)
   * - Since i2cSDA/i2cSCL are uint8_t, use 255 as "not provided"
   */
  bus = Interface::I2C;
  i2c = &wire;
  address = i2cAddr;

  #if defined(ESP32)
    /**
     * ESP32 I2C pin mapping
     * - If i2cSDA/i2cSCL are not provided, use default pins
     */
    if ((i2cSDA != 255u) && (i2cSCL != 255u)) i2c->begin(i2cSDA, i2cSCL);
    else i2c->begin();
  #else
    /**
     * Generic Arduino-core behavior
     * - I2C pin routing is defined by the board core / Wire instance
     */
    (void)i2cSDA;
    (void)i2cSCL;
    i2c->begin();
  #endif

  i2c->setClock(i2cSpeed);

  /**
   * Address ACK check
   * - If device does not ACK, wiring/address is wrong
   */
  i2c->beginTransmission(address);
  if (i2c->endTransmission() != 0) return false;

  /**
   * CHIP_ID validation
   * - Prevents false "connected" states
   */
  uint8_t chip_id = 0;
  if (!chipID(chip_id)) return false;

  return initSensor();
}

bool BMP585_7Semi::beginSPI(SPIClass &spiBus,
                            uint8_t csPin,
                            SPISettings settings,
                            int sck,
                            int miso,
                            int mosi)
{
  /**
   * Start sensor on SPI
   * - Stores SPI instance + CS pin + SPI settings
   * - Initializes SPI bus
   * - Verifies CHIP_ID
   * - Applies default sensor configuration so it works out-of-box
   *
   * Notes:
   * - On most Arduino cores, SPI pins are fixed by the board/SPI instance
   * - ESP32 supports runtime pin mapping using begin(sck,miso,mosi,cs)
   */
  bus = Interface::SPI;
  spi = &spiBus;
  cs_pin = (int8_t)csPin;
  spiSettings = settings;

  pinMode(cs_pin, OUTPUT);
  digitalWrite(cs_pin, HIGH);

  #if defined(ESP32)
    /**
     * ESP32 SPI pin mapping
     * - If pins are not provided, use default SPI pins
     */
    if ((sck != -1) && (miso != -1) && (mosi != -1)) spi->begin(sck, miso, mosi, cs_pin);
    else spi->begin();
  #else
    /**
     * Generic Arduino-core behavior
     * - SPI pin routing is defined by the board core / SPI instance
     */
    (void)sck;
    (void)miso;
    (void)mosi;
    spi->begin();
  #endif

  delay(10);

  /**
   * CHIP_ID validation before applying defaults
   * - Helps users quickly detect CS/wiring problems
   */
  uint8_t chip_id = 0;
  if (!chipID(chip_id)) return false;

  return initSensor();
}

bool BMP585_7Semi::initSensor() {
  /**
   * Apply safe default configuration so the sensor works without user setup
   * - reset(): restores defaults and handles SPI re-entry (dummy read)
   * - OSR: x8/x8 with pressure enabled for stable readings
   * - IIR: off by default (user can enable later)
   * - ODR: 10 Hz, Normal mode, deep standby disabled for reliability
   */
  if (!reset()) return false;
  delay(10);

  if (!setOSR(OSR::OSR_8, OSR::OSR_8, true)) return false;
  if (!setIIR(IIRCoeff::IIRCoeff_127, IIRCoeff::IIRCoeff_127)) return false;

  /**
   * deepStandbyDisable=true
   * - Makes “continuous printing” sketches behave consistently
   * - User can enable deep standby later for low-power applications
   */
  if (!setODR(ODR::ODR_10_Hz, PowerMode::Normal, true)) return false;

  delay(20);
  return true;
}


bool BMP585_7Semi::reset() {
  /**
   * Soft reset command
   * - Write 0xB6 to CMD register (0x7E)
   * - After reset, user settings go back to defaults
   *
   * SPI-specific behavior (BMP585):
   * - After soft reset, device returns to I2C/I3C primary interface
   * - To re-enter SPI mode, perform a dummy SPI read while CS is low
   *   - Dummy read data is invalid
   *   - Follow-up reads are valid
   */
  if (!writeReg(REG_CMD, 0xB6)) return false;
  delay(10);

  if (bus == Interface::SPI) {
    /**
     * Dummy SPI read to switch interface back to SPI
     * - Reading CHIP_ID is safe and simple
     */
    uint8_t dummy = 0;
    if (!readReg(REG_CHIP_ID, dummy)) return false;
    delayMicroseconds(5);
  }

  return true;
}

bool BMP585_7Semi::chipID(uint8_t &chip_id) {
  /**
   * Read and validate CHIP_ID
   * - Returns false if bus read fails or ID mismatch
   * - Copies read value into chip_id for debugging/logging
   */
  if (!readReg(REG_CHIP_ID, chip_id)) return false;
  return (chip_id == CHIP_ID_EXPECTED);
}

bool BMP585_7Semi::revisionID(uint8_t &rev_id) {
  /**
   * Read ASIC revision id (0x02)
   * - Returns false if read fails
   */
  uint8_t v = 0;
  if (!readReg(REG_REV_ID, v)) return false;
  rev_id = v;
  return true;
}

bool BMP585_7Semi::setOSR(OSR osr_t, OSR osr_p, bool pressEnable) {
  /**
   * OSR_CONFIG (0x36)
   * - bits [2:0] osr_t
   * - bits [5:3] osr_p
   * - bit 6 press_en
   */
  uint8_t v = 0;
  if(!readReg(REG_OSR_CONFIG, v)) 
    return false;
  v &= ~(0x3F); // Clear all OSR bits
  v |= (uint8_t(osr_t) & 0x07u);
  v |= (uint8_t(osr_p) & 0x07u) << 3;
  if (pressEnable) v |= (1u << 6);
  return writeReg(REG_OSR_CONFIG, v);
}

bool BMP585_7Semi::setODR(ODR odr, PowerMode mode, bool deepStandbyDisable) {
  /**
   * ODR_CONFIG (0x37)
   * - bits [1:0] pwr_mode
   * - bits [6:2] odr
   * - bit 7 deep_dis (1 disables deep-standby)
   */
  uint8_t v = 0;
  if(!readReg(REG_ODR_CONFIG, v)) 
    return false;
  v &= ~(0x83); // Clear all ODR_CONFIG bits
  v |= (uint8_t(mode) & 0x03u);
  v |= (uint8_t(odr) & 0x1Fu) << 2;
  if (deepStandbyDisable) v |= (1u << 7);
  return writeReg(REG_ODR_CONFIG, v);
}

bool BMP585_7Semi::setPowerMode(PowerMode mode) {
  /**
   * Update only pwr_mode bits in ODR_CONFIG
   * - Keeps ODR + deep_dis unchanged
   */
  uint8_t v = 0;
  if (!readReg(REG_ODR_CONFIG, v)) return false;
  v = (v & ~0x03u);
  v |= (uint8_t(mode) & 0x03u);
  return writeReg(REG_ODR_CONFIG, v);
}

bool BMP585_7Semi::setIIR(IIRCoeff iir_t, IIRCoeff iir_p) {
  /**
   * Configure IIR filtering (DSP_IIR 0x31)
   * - Datasheet requirement: change IIR settings in STANDBY
   * - This function automatically switches to STANDBY and restores mode
   */
  uint8_t odr = 0;
  if (!readReg(REG_ODR_CONFIG, odr)) return false;

  uint8_t prevMode = (odr & 0x03u);

  /**
   * Enter STANDBY if needed
   * - Minimizes measurement glitches during filter update
   */
  if (prevMode != uint8_t(PowerMode::Standby)) {
    uint8_t tmp = (odr & ~0x03u) | uint8_t(PowerMode::Standby);
    if (!writeReg(REG_ODR_CONFIG, tmp)) return false;
    delay(2);
  }

  /**
   * DSP_IIR layout (library mapping)
   * - bits [2:0] temperature coefficient
   * - bits [5:3] pressure coefficient
   */
  uint8_t v = 0;
  v |= (uint8_t(iir_t) & 0x07u);
  v |= (uint8_t(iir_p) & 0x07u) << 3;

  if (!writeReg(REG_DSP_IIR, v)) return false;

  /**
   * Restore original mode
   * - Keeps user-selected ODR settings unchanged
   */
  if (prevMode != uint8_t(PowerMode::Standby)) {
    uint8_t tmp = (odr & ~0x03u) | prevMode;
    if (!writeReg(REG_ODR_CONFIG, tmp)) return false;
  }

  return true;
}

bool BMP585_7Semi::readTemperatureC(float &tempC) {
  /**
   * Read temperature in °C
   * - Reads temperature + pressure in one burst for a consistent snapshot
   */
  float t = NAN, p = NAN;
  if (!readTemperaturePressure(t, p)) return false;
  tempC = t;
  return true;
}

bool BMP585_7Semi::readPressurePa(float &pressPa) {
  /**
   * Read pressure in Pa
   * - Reads temperature + pressure in one burst for a consistent snapshot
   */
  float t = NAN, p = NAN;
  if (!readTemperaturePressure(t, p)) return false;
  pressPa = p;
  return true;
}

bool BMP585_7Semi::readTemperaturePressure(float &tempC, float &pressPa) {
  /**
   * Burst read temperature + pressure for a consistent snapshot
   * - TEMP:  0x1D..0x1F (3 bytes)
   * - PRESS: 0x20..0x22 (3 bytes)
   */
  uint8_t buf[6] = {0};
  if (!readNReg(REG_TEMP_XLSB, buf, sizeof(buf))) return false;

  uint32_t t24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
  uint32_t p24u = (uint32_t(buf[5]) << 16) | (uint32_t(buf[4]) << 8) | uint32_t(buf[3]);

  tempC   = tempFromRaw(signExtend24(t24u));
  pressPa = pressFromRaw(signExtend24(p24u));
  return true;
}

bool BMP585_7Semi::readAltitudeM(float seaLevel_hPa, float &altitude_m) {
  /**
   * Altitude estimate using standard barometric approximation
   * - Uses pressure in hPa for computation
   * - seaLevel_hPa must be > 0
   */
  if (seaLevel_hPa <= 0.0f) return false;

  float p_pa = 0.0f;
  if (!readPressurePa(p_pa)) return false;
  if (!isfinite(p_pa)) return false;

  float p_hpa = p_pa / 100.0f;
  altitude_m = 44330.0f * (1.0f - powf(p_hpa / seaLevel_hPa, 0.19029495f));
  return true;
}

bool BMP585_7Semi::writeReg(uint8_t reg, uint8_t value) {
  if (bus == Interface::I2C) {
    /**
     * I2C write
     * - Writes register address, then 1 data byte
     */
    if (!i2c) return false;

    i2c->beginTransmission(address);
    i2c->write(reg & 0x7Fu);
    i2c->write(value);
    return (i2c->endTransmission() == 0);
  }

  /**
   * SPI write
   * - First byte: W + A6..A0 (W=0)
   * - Then 1 data byte
   */
  if (!spi || cs_pin < 0) return false;

  digitalWrite(cs_pin, LOW);
  spi->beginTransaction(spiSettings);

  spi->transfer(reg & 0x7Fu);
  spi->transfer(value);

  spi->endTransaction();
  digitalWrite(cs_pin, HIGH);
  return true;
}

bool BMP585_7Semi::readReg(uint8_t reg, uint8_t &value) {
  return readNReg(reg, &value, 1);
}

bool BMP585_7Semi::readNReg(uint8_t reg, uint8_t *buf, size_t len) {
  if (!buf || len == 0) return false;

  if (bus == Interface::I2C) {
    /**
     * I2C burst read
     * - Write register address (no stop)
     * - Request N bytes
     */
    if (!i2c) return false;

    i2c->beginTransmission(address);
    i2c->write(reg & 0x7Fu);
    if (i2c->endTransmission(false) != 0) return false;

    size_t got = i2c->requestFrom((int)address, (int)len);
    if (got != len) return false;

    for (size_t i = 0; i < len; i++) buf[i] = i2c->read();
    return true;
  }

  /**
   * SPI burst read
   * - First byte: R + A6..A0 (R=1)
   * - Then read N bytes by sending dummy bytes
   */
  if (!spi || cs_pin < 0) return false;

  digitalWrite(cs_pin, LOW);
  spi->beginTransaction(spiSettings);

  spi->transfer((reg & 0x7Fu) | 0x80u);
  for (size_t i = 0; i < len; i++) buf[i] = spi->transfer(0x00);

  spi->endTransaction();
  digitalWrite(cs_pin, HIGH);
  return true;
}

/* =========================
 * Raw conversions
 * ========================= */

int32_t BMP585_7Semi::signExtend24(uint32_t x) {
  /**
   * Sign extend a 24-bit two's complement value into int32_t
   */
  x &= 0xFFFFFFu;
  if (x & 0x800000u) return (int32_t)(x | 0xFF000000u);
  return (int32_t)x;
}

float BMP585_7Semi::tempFromRaw(int32_t raw24) {
  /**
   * Temp_Data arithmetic representation: signed (24,16) [degC]
   * - value = raw / 2^16
   */
  return float(raw24) / 65536.0f;
}

float BMP585_7Semi::pressFromRaw(int32_t raw24) {
  /**
   * Press_Data arithmetic representation: signed (24,6) [Pa]
   * - value = raw / 2^6
   */
  return float(raw24) / 64.0f;
}

bool BMP585_7Semi::configureInterruptPin(bool latched, bool activeHigh, bool openDrain, bool enable) {
  /**
   * INT_CONFIG (0x14)
   * - bit0 int_mode: 0 pulsed, 1 latched
   * - bit1 int_pol:  0 active low, 1 active high
   * - bit2 int_od:   0 push-pull, 1 open-drain
   * - bit3 int_en:   0 disabled, 1 enabled
   */
  uint8_t v = 0;
  if (latched)    v |= (1u << 0);
  if (activeHigh) v |= (1u << 1);
  if (openDrain)  v |= (1u << 2);
  if (enable)     v |= (1u << 3);
  return writeReg(REG_INT_CONFIG, v);
}

bool BMP585_7Semi::setInterruptSources(bool drdy, bool fifo_full, bool fifo_ths, bool oor_p) {
  /**
   * INT_SOURCE (0x15)
   * - bit0 drdy_data_reg_en
   * - bit1 fifo_full_en
   * - bit2 fifo_ths_en
   * - bit3 oor_p_en
   */
  uint8_t v = 0;
  if (drdy)      v |= (1u << 0);
  if (fifo_full) v |= (1u << 1);
  if (fifo_ths)  v |= (1u << 2);
  if (oor_p)     v |= (1u << 3);
  return writeReg(REG_INT_SOURCE, v);
}

InterruptStatus BMP585_7Semi::readInterruptStatus() {
  /**
   * Read interrupt status (INT_STATUS 0x27)
   * - Clear-on-read bits for event flags
   */
  InterruptStatus st;
  uint8_t v = 0;
  if (!readReg(REG_INT_STATUS, v)) return st;

  st.drdy_data_reg = (v & (1u << 0));
  st.fifo_full     = (v & (1u << 1));
  st.fifo_ths      = (v & (1u << 2));
  st.oor_p         = (v & (1u << 3));
  st.por           = (v & (1u << 4));
  return st;
}

