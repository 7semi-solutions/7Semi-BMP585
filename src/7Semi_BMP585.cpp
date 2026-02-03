// // // // #include "7Semi_BMP585.h"

// // // // BMP585_7Semi::BMP585_7Semi() {}

// // // // /* =========================
// // // //  * Begin / Interface select
// // // //  * ========================= */

// // // // bool BMP585_7Semi::beginI2C(TwoWire &wire, uint8_t i2cAddr, uint32_t i2cSpeed) {
// // // //   /**
// // // //    * Start sensor on I2C
// // // //    * - Sets bus type + stores wire/address
// // // //    * - Performs a simple ACK test on the address
// // // //    * - Verifies CHIP_ID matches expected value
// // // //    */
// // // //   bus = Interface::I2C;
// // // //   i2c = &wire;
// // // //   address = i2cAddr;

// // // //   i2c->begin();
// // // //   i2c->setClock(i2cSpeed);

// // // //   i2c->beginTransmission(address);
// // // //   if (i2c->endTransmission() != 0) return false;

// // // //   uint8_t id = chipID();
// // // //   return (id == CHIP_ID_EXPECTED);
// // // // }

// // // // bool BMP585_7Semi::beginSPI(SPIClass &spiBus,
// // // //                             uint8_t csPin,
// // // //                             SPISettings settings,
// // // //                             int sck,
// // // //                             int miso,
// // // //                             int mosi)
// // // // {
// // // //   /**
// // // //    * Start sensor on SPI
// // // //    * - Stores SPI bus pointer + CS pin + SPI settings
// // // //    * - Initializes SPI bus (ESP32 uses begin(sck,miso,mosi,cs))
// // // //    * - Reads CHIP_ID to verify device presence
// // // //    *
// // // //    * Note:
// // // //    * - Do NOT beginTransaction() here, because chipID()/readNReg()
// // // //    *   already manage transactions internally.
// // // //    */
// // // //   bus = Interface::SPI;
// // // //   spi = &spiBus;

// // // //   cs_pin = csPin;
// // // //   spiSettings = settings;

// // // //   pinMode(cs_pin, OUTPUT);
// // // //   digitalWrite(cs_pin, HIGH);

// // // //   #if defined(ESP32)
// // // //   /**
// // // //    * ESP32 requires explicit pin mapping when using VSPI/HSPI
// // // //    * - If you pass -1 for pins, the core will use defaults
// // // //    */
// // // //   spi->begin(sck, miso, mosi, cs_pin);
// // // //   #else
// // // //   spi->begin();
// // // //   #endif

// // // //   delay(10);

// // // //   /**
// // // //    * Read chip id
// // // //    * - If wiring is correct, CHIP_ID should be stable
// // // //    */
// // // //   uint8_t id = chipID();
// // // //   return (id == CHIP_ID_EXPECTED);
// // // // }

// // // // /* =========================
// // // //  * Basic operations
// // // //  * ========================= */

// // // // bool BMP585_7Semi::reset() {
// // // //   /**
// // // //    * Soft reset command
// // // //    * - Write 0xB6 to CMD register (0x7E)
// // // //    * - After reset, user settings go back to defaults
// // // //    *
// // // //    * SPI note (BMP585 behavior):
// // // //    * - After soft reset, device returns to I2C/I3C primary interface
// // // //    * - To re-enter SPI mode, perform a dummy SPI read
// // // //    *   - Data from the dummy read is invalid
// // // //    *   - A follow-up normal read is valid
// // // //    */
// // // //   if (!writeReg(REG_CMD, 0xB6)) return false;
// // // //   delay(5);

// // // //   if (bus == Interface::SPI) {
// // // //     /**
// // // //      * Dummy read to switch interface back to SPI mode
// // // //      * - Using CHIP_ID register is safe and simple
// // // //      */
// // // //     uint8_t junk = 0;
// // // //     (void)readReg(REG_CHIP_ID, junk);
// // // //     delayMicroseconds(5);
// // // //   }

// // // //   return true;
// // // // }

// // // // uint8_t BMP585_7Semi::chipID() {
// // // //   uint8_t v = 0;
// // // //   (void)readReg(REG_CHIP_ID, v);
// // // //   return v;
// // // // }

// // // // uint8_t BMP585_7Semi::revisionID() {
// // // //   uint8_t v = 0;
// // // //   (void)readReg(REG_REV_ID, v);
// // // //   return v;
// // // // }

// // // // /* =========================
// // // //  * Configuration
// // // //  * ========================= */

// // // // bool BMP585_7Semi::setOSR(OSR osr_t, OSR osr_p, bool pressEnable) {
// // // //   /**
// // // //    * OSR_CONFIG (0x36)
// // // //    * - bits [2:0] osr_t
// // // //    * - bits [5:3] osr_p
// // // //    * - bit 6 press_en
// // // //    */
// // // //   uint8_t v = 0;
// // // //   v |= (uint8_t(osr_t) & 0x07);
// // // //   v |= (uint8_t(osr_p) & 0x07) << 3;
// // // //   if (pressEnable) v |= (1u << 6);
// // // //   return writeReg(REG_OSR_CONFIG, v);
// // // // }

// // // // bool BMP585_7Semi::setODR(ODR odr, PowerMode mode, bool deepStandbyDisable) {
// // // //   /**
// // // //    * ODR_CONFIG (0x37)
// // // //    * - bits [1:0] pwr_mode
// // // //    * - bits [6:2] odr
// // // //    * - bit 7 deep_dis (1 disables deep-standby)
// // // //    */
// // // //   uint8_t v = 0;
// // // //   v |= (uint8_t(mode) & 0x03);
// // // //   v |= (uint8_t(odr) & 0x1F) << 2;
// // // //   if (deepStandbyDisable) v |= (1u << 7);
// // // //   return writeReg(REG_ODR_CONFIG, v);
// // // // }

// // // // bool BMP585_7Semi::setPowerMode(PowerMode mode) {
// // // //   /**
// // // //    * Update only pwr_mode bits in ODR_CONFIG
// // // //    * - Keeps ODR + deep_dis unchanged
// // // //    */
// // // //   uint8_t reg = 0;
// // // //   if (!readReg(REG_ODR_CONFIG, reg)) return false;
// // // //   reg = (reg & ~0x03);
// // // //   reg |= (uint8_t(mode) & 0x03);
// // // //   return writeReg(REG_ODR_CONFIG, reg);
// // // // }

// // // // bool BMP585_7Semi::setIIR(uint8_t iir_t, uint8_t iir_p) {
// // // //   /**
// // // //    * DSP_IIR (0x31)
// // // //    * - Library exposes raw fields for full control.
// // // //    * - Practical mapping:
// // // //    *   - iir_t in bits [2:0]
// // // //    *   - iir_p in bits [5:3]
// // // //    */
// // // //   uint8_t v = 0;
// // // //   v |= (iir_t & 0x07);
// // // //   v |= (iir_p & 0x07) << 3;
// // // //   return writeReg(REG_DSP_IIR, v);
// // // // }

// // // // /* =========================
// // // //  * Data read helpers
// // // //  * ========================= */

// // // // float BMP585_7Semi::readTemperatureC() {
// // // //   float t = NAN, p = NAN;
// // // //   if (!readTemperaturePressure(t, p)) return NAN;
// // // //   return t;
// // // // }

// // // // float BMP585_7Semi::readPressurePa() {
// // // //   float t = NAN, p = NAN;
// // // //   if (!readTemperaturePressure(t, p)) return NAN;
// // // //   return p;
// // // // }

// // // // bool BMP585_7Semi::readTemperaturePressure(float &tempC, float &pressPa) {
// // // //   /**
// // // //    * Burst read from TEMP_XLSB through PRESS_MSB
// // // //    * - Temp:  0x1D..0x1F (3 bytes)
// // // //    * - Press: 0x20..0x22 (3 bytes)
// // // //    */
// // // //   uint8_t buf[6] = {0};
// // // //   if (!readNReg(REG_TEMP_XLSB, buf, sizeof(buf))) return false;

// // // //   uint32_t t24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
// // // //   uint32_t p24u = (uint32_t(buf[5]) << 16) | (uint32_t(buf[4]) << 8) | uint32_t(buf[3]);

// // // //   int32_t t24 = signExtend24(t24u);
// // // //   int32_t p24 = signExtend24(p24u);

// // // //   tempC = tempFromRaw(t24);
// // // //   pressPa = pressFromRaw(p24);
// // // //   return true;
// // // // }

// // // // float BMP585_7Semi::readAltitudeM(float seaLevel_hPa) {
// // // //   /**
// // // //    * Standard barometric formula approximation
// // // //    * - Uses pressure in hPa for computation
// // // //    */
// // // //   float p_pa = readPressurePa();
// // // //   if (!isfinite(p_pa) || seaLevel_hPa <= 0.0f) return NAN;

// // // //   float p_hpa = p_pa / 100.0f;
// // // //   return 44330.0f * (1.0f - powf(p_hpa / seaLevel_hPa, 0.19029495f));
// // // // }

// // // // /* =========================
// // // //  * FIFO
// // // //  * ========================= */

// // // // bool BMP585_7Semi::configureFIFO(FIFOFrame frameSel, uint8_t decimationPow2, uint8_t thresholdFrames, FIFOMode mode) {
// // // //   /**
// // // //    * FIFO config uses:
// // // //    * - FIFO_CONFIG (0x16): fifo_threshold [4:0], fifo_mode bit 5
// // // //    * - FIFO_SEL    (0x18): fifo_frame_sel [1:0], fifo_dec_sel [4:2]
// // // //    *
// // // //    * Note:
// // // //    * - Datasheet says FIFO_SEL must be changed in STANDBY mode.
// // // //    */
// // // //   decimationPow2 &= 0x07;
// // // //   thresholdFrames &= 0x1F;

// // // //   uint8_t fifo_cfg = 0;
// // // //   fifo_cfg |= (thresholdFrames & 0x1F);
// // // //   fifo_cfg |= (uint8_t(mode) & 0x01) << 5;

// // // //   uint8_t fifo_sel = 0;
// // // //   fifo_sel |= (uint8_t(frameSel) & 0x03);
// // // //   fifo_sel |= (decimationPow2 & 0x07) << 2;

// // // //   if (!writeReg(REG_FIFO_CONFIG, fifo_cfg)) return false;
// // // //   if (!writeReg(REG_FIFO_SEL, fifo_sel)) return false;
// // // //   return true;
// // // // }

// // // // uint8_t BMP585_7Semi::getFIFOCount() {
// // // //   uint8_t v = 0;
// // // //   (void)readReg(REG_FIFO_COUNT, v);
// // // //   return (v & 0x3F);
// // // // }

// // // // BMP585_7Semi::FIFORead BMP585_7Semi::readFIFOFrame() {
// // // //   FIFORead out;

// // // //   uint8_t fifo_sel = 0;
// // // //   if (!readReg(REG_FIFO_SEL, fifo_sel)) return out;

// // // //   FIFOFrame frame = FIFOFrame(fifo_sel & 0x03);
// // // //   out.frame = frame;

// // // //   size_t frameBytes = 0;
// // // //   if (frame == FIFOFrame::TempOnly) frameBytes = 3;
// // // //   else if (frame == FIFOFrame::PressOnly) frameBytes = 3;
// // // //   else if (frame == FIFOFrame::PressTemp) frameBytes = 6;
// // // //   else return out;

// // // //   uint8_t buf[6] = {0};
// // // //   if (!readNReg(REG_FIFO_DATA, buf, frameBytes)) return out;

// // // //   /**
// // // //    * Empty frame is 0x7F (returned when FIFO empty/disabled/turns empty)
// // // //    * - If first byte is 0x7F, treat as empty
// // // //    */
// // // //   if (buf[0] == 0x7F) {
// // // //     out.empty = true;
// // // //     out.valid = false;
// // // //     return out;
// // // //   }

// // // //   if (frame == FIFOFrame::TempOnly) {
// // // //     uint32_t t24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
// // // //     out.temperature_c = tempFromRaw(signExtend24(t24u));
// // // //     out.valid = true;
// // // //   } else if (frame == FIFOFrame::PressOnly) {
// // // //     uint32_t p24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
// // // //     out.pressure_pa = pressFromRaw(signExtend24(p24u));
// // // //     out.valid = true;
// // // //   } else { /* PressTemp */
// // // //     uint32_t t24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
// // // //     uint32_t p24u = (uint32_t(buf[5]) << 16) | (uint32_t(buf[4]) << 8) | uint32_t(buf[3]);

// // // //     out.temperature_c = tempFromRaw(signExtend24(t24u));
// // // //     out.pressure_pa   = pressFromRaw(signExtend24(p24u));
// // // //     out.valid = true;
// // // //   }

// // // //   return out;
// // // // }

// // // // bool BMP585_7Semi::clearFIFO() {
// // // //   /**
// // // //    * FIFO is flushed on changes of FIFO_SEL.fifo_frame_sel or cfg_fifo_dec_sel
// // // //    * - We do a disable -> restore sequence
// // // //    */
// // // //   uint8_t prev = 0;
// // // //   if (!readReg(REG_FIFO_SEL, prev)) return false;

// // // //   uint8_t disabled = (prev & ~0x03) | uint8_t(FIFOFrame::Disabled);
// // // //   if (!writeReg(REG_FIFO_SEL, disabled)) return false;
// // // //   delay(2);

// // // //   if (!writeReg(REG_FIFO_SEL, prev)) return false;
// // // //   delay(2);
// // // //   return true;
// // // // }

// // // // /* =========================
// // // //  * Interrupts / OOR
// // // //  * ========================= */

// // // // bool BMP585_7Semi::configureIntPin(bool latched, bool activeHigh, bool openDrain, bool enable) {
// // // //   /**
// // // //    * INT_CONFIG (0x14)
// // // //    * - bit0 int_mode: 0 pulsed, 1 latched
// // // //    * - bit1 int_pol:  0 active low, 1 active high
// // // //    * - bit2 int_od:   0 push-pull, 1 open-drain
// // // //    * - bit3 int_en:   0 disabled, 1 enabled
// // // //    */
// // // //   uint8_t v = 0;
// // // //   if (latched)    v |= (1u << 0);
// // // //   if (activeHigh) v |= (1u << 1);
// // // //   if (openDrain)  v |= (1u << 2);
// // // //   if (enable)     v |= (1u << 3);
// // // //   return writeReg(REG_INT_CONFIG, v);
// // // // }

// // // // bool BMP585_7Semi::setIntSources(bool drdy, bool fifo_full, bool fifo_ths, bool oor_p) {
// // // //   /**
// // // //    * INT_SOURCE (0x15)
// // // //    * - bit0 drdy_data_reg_en
// // // //    * - bit1 fifo_full_en
// // // //    * - bit2 fifo_ths_en
// // // //    * - bit3 oor_p_en
// // // //    */
// // // //   uint8_t v = 0;
// // // //   if (drdy)      v |= (1u << 0);
// // // //   if (fifo_full) v |= (1u << 1);
// // // //   if (fifo_ths)  v |= (1u << 2);
// // // //   if (oor_p)     v |= (1u << 3);
// // // //   return writeReg(REG_INT_SOURCE, v);
// // // // }

// // // // BMP585_7Semi::InterruptStatus BMP585_7Semi::readIntStatus() {
// // // //   InterruptStatus st;
// // // //   uint8_t v = 0;
// // // //   if (!readReg(REG_INT_STATUS, v)) return st;

// // // //   st.drdy_data_reg = (v & (1u << 0)) != 0;
// // // //   st.fifo_full     = (v & (1u << 1)) != 0;
// // // //   st.fifo_ths      = (v & (1u << 2)) != 0;
// // // //   st.oor_p         = (v & (1u << 3)) != 0;
// // // //   st.por           = (v & (1u << 4)) != 0;
// // // //   return st;
// // // // }

// // // // bool BMP585_7Semi::setOORWindow(uint32_t refPressurePa, uint8_t rangePa, uint8_t countLimitCode) {
// // // //   /**
// // // //    * OOR threshold is 17-bit in Pa:
// // // //    * - OOR_THR_P_LSB (0x32): bits [7:0]
// // // //    * - OOR_THR_P_MSB (0x33): bits [15:8]
// // // //    * - OOR_CONFIG    (0x35): bit0 is oor_thr_p_16 (bit16)
// // // //    *
// // // //    * OOR_RANGE (0x34): 8-bit range in Pa (window around reference)
// // // //    * OOR_CONFIG (0x35): cnt_lim stored in bits [7:4] in this library
// // // //    * - If you want exact control, you can also write registers directly.
// // // //    */
// // // //   refPressurePa &= 0x1FFFF;

// // // //   uint8_t thr_lsb = uint8_t(refPressurePa & 0xFF);
// // // //   uint8_t thr_msb = uint8_t((refPressurePa >> 8) & 0xFF);
// // // //   uint8_t thr_b16 = uint8_t((refPressurePa >> 16) & 0x01);

// // // //   if (!writeReg(REG_OOR_THR_LSB, thr_lsb)) return false;
// // // //   if (!writeReg(REG_OOR_THR_MSB, thr_msb)) return false;
// // // //   if (!writeReg(REG_OOR_RANGE, rangePa)) return false;

// // // //   uint8_t cfg = 0;
// // // //   cfg |= (thr_b16 & 0x01);
// // // //   cfg |= (countLimitCode & 0x0F) << 4;
// // // //   return writeReg(REG_OOR_CONFIG, cfg);
// // // // }

// // // // /* =========================
// // // //  * Low-level bus operations
// // // //  * ========================= */

// // // // bool BMP585_7Semi::writeReg(uint8_t reg, uint8_t value) {
// // // //   if (bus == Interface::I2C) {
// // // //     i2c->beginTransmission(address);
// // // //     i2c->write(reg & 0x7F);
// // // //     i2c->write(value);
// // // //     return (i2c->endTransmission() == 0);
// // // //   }

// // // //   if (!spi || cs_pin < 0) return false;

// // // //   /**
// // // //    * SPI write frame
// // // //    * - First byte: W + A6..A0 (W=0)
// // // //    * - Then 1 data byte
// // // //    */
// // // //   digitalWrite(cs_pin, LOW);
// // // //   spi->beginTransaction(spiSettings);

// // // //   spi->transfer(reg & 0x7F);
// // // //   spi->transfer(value);

// // // //   spi->endTransaction();
// // // //   digitalWrite(cs_pin, HIGH);
// // // //   return true;
// // // // }

// // // // bool BMP585_7Semi::readReg(uint8_t reg, uint8_t &value) {
// // // //   return readNReg(reg, &value, 1);
// // // // }

// // // // bool BMP585_7Semi::readNReg(uint8_t reg, uint8_t *buf, size_t len) {
// // // //   if (!buf || len == 0) return false;

// // // //   if (bus == Interface::I2C) {
// // // //     i2c->beginTransmission(address);
// // // //     i2c->write(reg & 0x7F);
// // // //     if (i2c->endTransmission(false) != 0) return false;

// // // //     size_t got = i2c->requestFrom((int)address, (int)len);
// // // //     if (got != len) return false;

// // // //     for (size_t i = 0; i < len; i++) {
// // // //       buf[i] = i2c->read();
// // // //     }
// // // //     return true;
// // // //   }

// // // //   if (!spi || cs_pin < 0) return false;

// // // //   /**
// // // //    * SPI read frame
// // // //    * - First byte: R + A6..A0 (R=1)
// // // //    * - Then read out bytes on MISO while clocking dummy bytes
// // // //    */
// // // //   digitalWrite(cs_pin, LOW);
// // // //   spi->beginTransaction(spiSettings);

// // // //   spi->transfer((reg & 0x7F) | 0x80);
// // // //   for (size_t i = 0; i < len; i++) {
// // // //     buf[i] = spi->transfer(0x00);
// // // //   }

// // // //   spi->endTransaction();
// // // //   digitalWrite(cs_pin, HIGH);
// // // //   return true;
// // // // }

// // // // bool BMP585_7Semi::setBits(uint8_t reg, uint8_t mask, uint8_t valueShifted) {
// // // //   uint8_t v = 0;
// // // //   if (!readReg(reg, v)) return false;
// // // //   v = (v & ~mask) | (valueShifted & mask);
// // // //   return writeReg(reg, v);
// // // // }

// // // // /* =========================
// // // //  * Raw conversions
// // // //  * ========================= */

// // // // int32_t BMP585_7Semi::signExtend24(uint32_t x) {
// // // //   x &= 0xFFFFFFu;
// // // //   if (x & 0x800000u) {
// // // //     return (int32_t)(x | 0xFF000000u);
// // // //   }
// // // //   return (int32_t)x;
// // // // }

// // // // float BMP585_7Semi::tempFromRaw(int32_t raw24) {
// // // //   /**
// // // //    * Temp_Data arithmetic representation: signed (24,16) [degC]
// // // //    * - value = raw / 2^16
// // // //    */
// // // //   return float(raw24) / 65536.0f;
// // // // }

// // // // float BMP585_7Semi::pressFromRaw(int32_t raw24) {
// // // //   /**
// // // //    * Press_Data arithmetic representation: signed (24,6) [Pa]
// // // //    * - value = raw / 2^6
// // // //    */
// // // //   return float(raw24) / 64.0f;
// // // // }

// // // // /* =========================
// // // //  * NVM helpers
// // // //  * ========================= */

// // // // BMP585_7Semi::NVMStatus BMP585_7Semi::readNVMStatus() {
// // // //   NVMStatus st;
// // // //   uint8_t v = 0;
// // // //   if (!readReg(REG_STATUS, v)) return st;

// // // //   st.rdy     = (v & (1u << 1)) != 0;
// // // //   st.err     = (v & (1u << 2)) != 0;
// // // //   st.cmd_err = (v & (1u << 3)) != 0;
// // // //   return st;
// // // // }

// // // // bool BMP585_7Semi::waitNVMReady(uint32_t timeout_us) {
// // // //   /** Wait until STATUS.status_nvm_rdy is 1
// // // //    *
// // // //    * - Timeout is host-side safety
// // // //    * - NVM read is ~200us, NVM program is ~10ms per datasheet
// // // //    */
// // // //   const uint32_t start = micros();
// // // //   while ((uint32_t)(micros() - start) < timeout_us) {
// // // //     auto st = readNVMStatus();
// // // //     if (st.rdy) return true;
// // // //     delayMicroseconds(50);
// // // //   }
// // // //   return false;
// // // // }

// // // // bool BMP585_7Semi::writeCMDSequence(uint8_t b0, uint8_t b1) {
// // // //   /** CMD sequence must be a dedicated write transaction
// // // //    *
// // // //    * - Datasheet explicitly says: don't combine with other register writes in a burst
// // // //    */
// // // //   if (!writeReg(REG_CMD, b0)) return false;
// // // //   if (!writeReg(REG_CMD, b1)) return false;
// // // //   return true;
// // // // }

// // // // bool BMP585_7Semi::readNVMUserRow(uint8_t rowAddr, uint16_t &out) {
// // // //   out = 0;

// // // //   /** User range is rows 0x20..0x22 (2 bytes each) */
// // // //   if (rowAddr < 0x20 || rowAddr > 0x22) return false;

// // // //   /** Switch to STANDBY and disable deep standby */
// // // //   if (!setPowerMode(PowerMode::Standby)) return false;

// // // //   uint8_t odr = 0;
// // // //   if (!readReg(REG_ODR_CONFIG, odr)) return false;
// // // //   odr |= (1u << 7); /* deep_dis = 1 */
// // // //   if (!writeReg(REG_ODR_CONFIG, odr)) return false;

// // // //   /** Wait for NVM ready */
// // // //   if (!waitNVMReady(20000)) return false;

// // // //   /** Write NVM_ADDR with prog_en=0 and row address */
// // // //   uint8_t nvm_addr = (rowAddr & 0x3F); /* bits[5:0] */
// // // //   if (!writeReg(REG_NVM_ADDR, nvm_addr)) return false;

// // // //   /** CMD = USR_READ sequence (0x5D, 0xA5) */
// // // //   if (!writeCMDSequence(0x5D, 0xA5)) return false;

// // // //   /** Wait until ready again (~200us typical) */
// // // //   if (!waitNVMReady(20000)) return false;

// // // //   /** Check status error flags */
// // // //   auto st = readNVMStatus();
// // // //   if (st.err || st.cmd_err) return false;

// // // //   /** Read data regs */
// // // //   uint8_t l = 0, h = 0;
// // // //   if (!readReg(REG_NVM_DATA_LSB, l)) return false;
// // // //   if (!readReg(REG_NVM_DATA_MSB, h)) return false;

// // // //   out = (uint16_t(h) << 8) | uint16_t(l);
// // // //   return true;
// // // // }

// // // // bool BMP585_7Semi::writeNVMUserRow(uint8_t rowAddr, uint16_t value) {
// // // //   /** User range is rows 0x20..0x22 (2 bytes each) */
// // // //   if (rowAddr < 0x20 || rowAddr > 0x22) return false;

// // // //   /** Switch to STANDBY and disable deep standby */
// // // //   if (!setPowerMode(PowerMode::Standby)) return false;

// // // //   uint8_t odr = 0;
// // // //   if (!readReg(REG_ODR_CONFIG, odr)) return false;
// // // //   odr |= (1u << 7); /* deep_dis = 1 */
// // // //   if (!writeReg(REG_ODR_CONFIG, odr)) return false;

// // // //   /** Wait for NVM ready */
// // // //   if (!waitNVMReady(20000)) return false;

// // // //   /** Set NVM_ADDR with prog_en=1 and row address */
// // // //   uint8_t nvm_addr = (rowAddr & 0x3F) | (1u << 6); /* prog_en bit6 */
// // // //   if (!writeReg(REG_NVM_ADDR, nvm_addr)) return false;

// // // //   /** Write data regs */
// // // //   uint8_t l = uint8_t(value & 0xFF);
// // // //   uint8_t h = uint8_t((value >> 8) & 0xFF);
// // // //   if (!writeReg(REG_NVM_DATA_LSB, l)) return false;
// // // //   if (!writeReg(REG_NVM_DATA_MSB, h)) return false;

// // // //   /** CMD = USR_PROG sequence (0x5D, 0xA0) */
// // // //   if (!writeCMDSequence(0x5D, 0xA0)) return false;

// // // //   /** Wait until ready (~10ms typical) */
// // // //   if (!waitNVMReady(50000)) return false;

// // // //   /** Check status error flags */
// // // //   auto st = readNVMStatus();
// // // //   if (st.err || st.cmd_err) return false;

// // // //   /** Reset prog_en to 0 */
// // // //   uint8_t nvm_addr_clear = (rowAddr & 0x3F);
// // // //   if (!writeReg(REG_NVM_ADDR, nvm_addr_clear)) return false;

// // // //   return true;
// // // // }
// // // #include "7Semi_BMP585.h"

// // // /* =========================
// // //  * Construction / probing
// // //  * ========================= */

// // // BMP585_7Semi::BMP585_7Semi() {}

// // // bool BMP585_7Semi::probe() {
// // //   /**
// // //    * Basic presence check
// // //    * - Reads CHIP_ID and compares with expected value
// // //    * - Works for both I2C and SPI (depending on selected bus)
// // //    */
// // //   return (chipID() == CHIP_ID_EXPECTED);
// // // }

// // // /* =========================
// // //  * Begin / Interface select
// // //  * ========================= */

// // // bool BMP585_7Semi::beginI2C(TwoWire &wire, uint8_t i2cAddr, uint32_t i2cSpeed) {
// // //   /**
// // //    * Start sensor on I2C
// // //    * - Stores Wire instance + address
// // //    * - Sets bus speed
// // //    * - Verifies address ACK and CHIP_ID
// // //    */
// // //   bus = Interface::I2C;
// // //   i2c = &wire;
// // //   address = i2cAddr;

// // //   i2c->begin();
// // //   i2c->setClock(i2cSpeed);

// // //   i2c->beginTransmission(address);
// // //   if (i2c->endTransmission() != 0) return false;

// // //   return probe();
// // // }

// // // bool BMP585_7Semi::beginSPI(SPIClass &spiBus,
// // //                             uint8_t csPin,
// // //                             SPISettings settings,
// // //                             int sck,
// // //                             int miso,
// // //                             int mosi)
// // // {
// // //   /**
// // //    * Start sensor on SPI
// // //    * - Stores SPI instance + CS pin + SPI settings
// // //    * - Initializes SPI bus
// // //    * - Verifies CHIP_ID
// // //    *
// // //    * Notes:
// // //    * - Do not call beginTransaction() here, because read/write helpers
// // //    *   already manage beginTransaction()/endTransaction().
// // //    * - On ESP32, begin(sck,miso,mosi,cs) ensures correct pin routing.
// // //    */
// // //   bus = Interface::SPI;
// // //   spi = &spiBus;
// // //   cs_pin = (int8_t)csPin;
// // //   spiSettings = settings;

// // //   pinMode(cs_pin, OUTPUT);
// // //   digitalWrite(cs_pin, HIGH);

// // //   #if defined(ESP32)
// // //   spi->begin(sck, miso, mosi, cs_pin);
// // //   #else
// // //   (void)sck;
// // //   (void)miso;
// // //   (void)mosi;
// // //   spi->begin();
// // //   #endif

// // //   delay(10);

// // //   /**
// // //    * BMP585 can come up in I2C/I3C primary interface mode
// // //    * - A normal CHIP_ID read often works directly
// // //    * - If user later calls reset(), reset() will perform the required
// // //    *   dummy SPI read to re-enter SPI mode.
// // //    */
// // //   return probe();
// // // }

// // // /* =========================
// // //  * Basic operations
// // //  * ========================= */

// // // bool BMP585_7Semi::reset() {
// // //   /**
// // //    * Soft reset command
// // //    * - Write 0xB6 to CMD register (0x7E)
// // //    * - After reset, user settings go back to defaults
// // //    *
// // //    * SPI-specific behavior (BMP585):
// // //    * - After soft reset, device returns to I2C/I3C primary interface
// // //    * - To re-enter SPI mode, perform a dummy SPI read while CS is low
// // //    *   - Dummy read data is invalid
// // //    *   - Follow-up reads are valid
// // //    */
// // //   if (!writeReg(REG_CMD, 0xB6)) return false;
// // //   delay(5);

// // //   if (bus == Interface::SPI) {
// // //     /**
// // //      * Dummy SPI read to switch interface back to SPI
// // //      * - Reading CHIP_ID is safe and simple
// // //      */
// // //     uint8_t junk = 0;
// // //     (void)readReg(REG_CHIP_ID, junk);
// // //     delayMicroseconds(5);
// // //   }

// // //   return true;
// // // }

// // // uint8_t BMP585_7Semi::chipID() {
// // //   uint8_t v = 0;
// // //   (void)readReg(REG_CHIP_ID, v);
// // //   return v;
// // // }

// // // uint8_t BMP585_7Semi::revisionID() {
// // //   uint8_t v = 0;
// // //   (void)readReg(REG_REV_ID, v);
// // //   return v;
// // // }

// // // /* =========================
// // //  * Configuration
// // //  * ========================= */

// // // bool BMP585_7Semi::setOSR(OSR osr_t, OSR osr_p, bool pressEnable) {
// // //   /**
// // //    * OSR_CONFIG (0x36)
// // //    * - bits [2:0] osr_t
// // //    * - bits [5:3] osr_p
// // //    * - bit 6 press_en
// // //    */
// // //   uint8_t v = 0;
// // //   v |= (uint8_t(osr_t) & 0x07);
// // //   v |= (uint8_t(osr_p) & 0x07) << 3;
// // //   if (pressEnable) v |= (1u << 6);
// // //   return writeReg(REG_OSR_CONFIG, v);
// // // }

// // // bool BMP585_7Semi::setODR(ODR odr, PowerMode mode, bool deepStandbyDisable) {
// // //   /**
// // //    * ODR_CONFIG (0x37)
// // //    * - bits [1:0] pwr_mode
// // //    * - bits [6:2] odr
// // //    * - bit 7 deep_dis (1 disables deep-standby)
// // //    */
// // //   uint8_t v = 0;
// // //   v |= (uint8_t(mode) & 0x03);
// // //   v |= (uint8_t(odr) & 0x1F) << 2;
// // //   if (deepStandbyDisable) v |= (1u << 7);
// // //   return writeReg(REG_ODR_CONFIG, v);
// // // }

// // // bool BMP585_7Semi::setPowerMode(PowerMode mode) {
// // //   /**
// // //    * Update only pwr_mode bits in ODR_CONFIG
// // //    * - Keeps ODR + deep_dis unchanged
// // //    */
// // //   uint8_t reg = 0;
// // //   if (!readReg(REG_ODR_CONFIG, reg)) return false;
// // //   reg = (reg & ~0x03u) | (uint8_t(mode) & 0x03u);
// // //   return writeReg(REG_ODR_CONFIG, reg);
// // // }

// // // bool BMP585_7Semi::setIIR(uint8_t iir_t, uint8_t iir_p) {
// // //   /**
// // //    * DSP_IIR (0x31)
// // //    * - Raw field access for temperature and pressure IIR strength
// // //    * - Practical mapping in this library:
// // //    *   - iir_t in bits [2:0]
// // //    *   - iir_p in bits [5:3]
// // //    */
// // //   uint8_t v = 0;
// // //   v |= (iir_t & 0x07u);
// // //   v |= (iir_p & 0x07u) << 3;
// // //   return writeReg(REG_DSP_IIR, v);
// // // }

// // // /* =========================
// // //  * Reads
// // //  * ========================= */

// // // float BMP585_7Semi::readTemperatureC() {
// // //   float t = NAN, p = NAN;
// // //   if (!readTemperaturePressure(t, p)) return NAN;
// // //   return t;
// // // }

// // // float BMP585_7Semi::readPressurePa() {
// // //   float t = NAN, p = NAN;
// // //   if (!readTemperaturePressure(t, p)) return NAN;
// // //   return p;
// // // }

// // // bool BMP585_7Semi::readTemperaturePressure(float &tempC, float &pressPa) {
// // //   /**
// // //    * Burst read temperature + pressure for a consistent snapshot
// // //    * - TEMP:  0x1D..0x1F (3 bytes)
// // //    * - PRESS: 0x20..0x22 (3 bytes)
// // //    *
// // //    * Notes:
// // //    * - If pressure is not enabled (OSR_CONFIG.press_en = 0),
// // //    *   pressure registers may be stale or not updated.
// // //    */
// // //   uint8_t buf[6] = {0};
// // //   if (!readNReg(REG_TEMP_XLSB, buf, sizeof(buf))) return false;

// // //   uint32_t t24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
// // //   uint32_t p24u = (uint32_t(buf[5]) << 16) | (uint32_t(buf[4]) << 8) | uint32_t(buf[3]);

// // //   tempC  = tempFromRaw(signExtend24(t24u));
// // //   pressPa = pressFromRaw(signExtend24(p24u));
// // //   return true;
// // // }

// // // float BMP585_7Semi::readAltitudeM(float seaLevel_hPa) {
// // //   /**
// // //    * Altitude estimate using standard barometric approximation
// // //    * - Uses pressure in hPa for computation
// // //    * - seaLevel_hPa must be > 0
// // //    */
// // //   float p_pa = readPressurePa();
// // //   if (!isfinite(p_pa) || seaLevel_hPa <= 0.0f) return NAN;

// // //   float p_hpa = p_pa / 100.0f;
// // //   return 44330.0f * (1.0f - powf(p_hpa / seaLevel_hPa, 0.19029495f));
// // // }

// // // /* =========================
// // //  * FIFO
// // //  * ========================= */

// // // bool BMP585_7Semi::configureFIFO(FIFOFrame frameSel, uint8_t decimationPow2, uint8_t thresholdFrames, FIFOMode mode) {
// // //   /**
// // //    * FIFO configuration
// // //    * - FIFO_CONFIG (0x16): threshold [4:0], mode bit5
// // //    * - FIFO_SEL    (0x18): frame_sel [1:0], decimation [4:2]
// // //    *
// // //    * Notes:
// // //    * - Datasheet recommends changing FIFO_SEL in STANDBY mode.
// // //    */
// // //   decimationPow2 &= 0x07u;
// // //   thresholdFrames &= 0x1Fu;

// // //   uint8_t fifo_cfg = 0;
// // //   fifo_cfg |= (thresholdFrames & 0x1Fu);
// // //   fifo_cfg |= (uint8_t(mode) & 0x01u) << 5;

// // //   uint8_t fifo_sel = 0;
// // //   fifo_sel |= (uint8_t(frameSel) & 0x03u);
// // //   fifo_sel |= (decimationPow2 & 0x07u) << 2;

// // //   if (!writeReg(REG_FIFO_CONFIG, fifo_cfg)) return false;
// // //   if (!writeReg(REG_FIFO_SEL, fifo_sel)) return false;
// // //   return true;
// // // }

// // // uint8_t BMP585_7Semi::getFIFOCount() {
// // //   uint8_t v = 0;
// // //   (void)readReg(REG_FIFO_COUNT, v);
// // //   return (v & 0x3Fu);
// // // }

// // // BMP585_7Semi::FIFORead BMP585_7Semi::readFIFOFrame() {
// // //   FIFORead out;

// // //   uint8_t fifo_sel = 0;
// // //   if (!readReg(REG_FIFO_SEL, fifo_sel)) return out;

// // //   FIFOFrame frame = FIFOFrame(fifo_sel & 0x03u);
// // //   out.frame = frame;

// // //   size_t frameBytes = 0;
// // //   if (frame == FIFOFrame::TempOnly) frameBytes = 3;
// // //   else if (frame == FIFOFrame::PressOnly) frameBytes = 3;
// // //   else if (frame == FIFOFrame::PressTemp) frameBytes = 6;
// // //   else return out;

// // //   uint8_t buf[6] = {0};
// // //   if (!readNReg(REG_FIFO_DATA, buf, frameBytes)) return out;

// // //   /**
// // //    * Empty frame is reported as 0x7F
// // //    * - If first byte is 0x7F, treat as empty
// // //    */
// // //   if (buf[0] == 0x7F) {
// // //     out.empty = true;
// // //     out.valid = false;
// // //     return out;
// // //   }

// // //   if (frame == FIFOFrame::TempOnly) {
// // //     uint32_t t24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
// // //     out.temperature_c = tempFromRaw(signExtend24(t24u));
// // //     out.valid = true;
// // //   } else if (frame == FIFOFrame::PressOnly) {
// // //     uint32_t p24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
// // //     out.pressure_pa = pressFromRaw(signExtend24(p24u));
// // //     out.valid = true;
// // //   } else {
// // //     uint32_t t24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
// // //     uint32_t p24u = (uint32_t(buf[5]) << 16) | (uint32_t(buf[4]) << 8) | uint32_t(buf[3]);
// // //     out.temperature_c = tempFromRaw(signExtend24(t24u));
// // //     out.pressure_pa   = pressFromRaw(signExtend24(p24u));
// // //     out.valid = true;
// // //   }

// // //   return out;
// // // }

// // // bool BMP585_7Semi::clearFIFO() {
// // //   /**
// // //    * Best-effort FIFO clear
// // //    * - FIFO is flushed on changes of FIFO_SEL.frame_sel or decimation
// // //    * - We do a disable -> restore sequence
// // //    */
// // //   uint8_t prev = 0;
// // //   if (!readReg(REG_FIFO_SEL, prev)) return false;

// // //   uint8_t disabled = (prev & ~0x03u) | uint8_t(FIFOFrame::Disabled);
// // //   if (!writeReg(REG_FIFO_SEL, disabled)) return false;
// // //   delay(2);

// // //   if (!writeReg(REG_FIFO_SEL, prev)) return false;
// // //   delay(2);

// // //   return true;
// // // }

// // // /* =========================
// // //  * Interrupts / OOR window
// // //  * ========================= */

// // // bool BMP585_7Semi::configureIntPin(bool latched, bool activeHigh, bool openDrain, bool enable) {
// // //   /**
// // //    * INT_CONFIG (0x14)
// // //    * - bit0 int_mode: 0 pulsed, 1 latched
// // //    * - bit1 int_pol:  0 active low, 1 active high
// // //    * - bit2 int_od:   0 push-pull, 1 open-drain
// // //    * - bit3 int_en:   0 disabled, 1 enabled
// // //    */
// // //   uint8_t v = 0;
// // //   if (latched)    v |= (1u << 0);
// // //   if (activeHigh) v |= (1u << 1);
// // //   if (openDrain)  v |= (1u << 2);
// // //   if (enable)     v |= (1u << 3);
// // //   return writeReg(REG_INT_CONFIG, v);
// // // }

// // // bool BMP585_7Semi::setIntSources(bool drdy, bool fifo_full, bool fifo_ths, bool oor_p) {
// // //   /**
// // //    * INT_SOURCE (0x15)
// // //    * - bit0 drdy_data_reg_en
// // //    * - bit1 fifo_full_en
// // //    * - bit2 fifo_ths_en
// // //    * - bit3 oor_p_en
// // //    */
// // //   uint8_t v = 0;
// // //   if (drdy)      v |= (1u << 0);
// // //   if (fifo_full) v |= (1u << 1);
// // //   if (fifo_ths)  v |= (1u << 2);
// // //   if (oor_p)     v |= (1u << 3);
// // //   return writeReg(REG_INT_SOURCE, v);
// // // }

// // // BMP585_7Semi::InterruptStatus BMP585_7Semi::readIntStatus() {
// // //   InterruptStatus st;
// // //   uint8_t v = 0;
// // //   if (!readReg(REG_INT_STATUS, v)) return st;

// // //   st.drdy_data_reg = (v & (1u << 0)) != 0;
// // //   st.fifo_full     = (v & (1u << 1)) != 0;
// // //   st.fifo_ths      = (v & (1u << 2)) != 0;
// // //   st.oor_p         = (v & (1u << 3)) != 0;
// // //   st.por           = (v & (1u << 4)) != 0;
// // //   return st;
// // // }

// // // bool BMP585_7Semi::setOORWindow(uint32_t refPressurePa, uint8_t rangePa, uint8_t countLimitCode) {
// // //   /**
// // //    * OOR window configuration
// // //    * - refPressurePa is stored as 17-bit threshold
// // //    * - rangePa is a +/- window around reference
// // //    * - countLimitCode stored in bits [7:4] by this library
// // //    */
// // //   refPressurePa &= 0x1FFFFu;

// // //   uint8_t thr_lsb = uint8_t(refPressurePa & 0xFFu);
// // //   uint8_t thr_msb = uint8_t((refPressurePa >> 8) & 0xFFu);
// // //   uint8_t thr_b16 = uint8_t((refPressurePa >> 16) & 0x01u);

// // //   if (!writeReg(REG_OOR_THR_LSB, thr_lsb)) return false;
// // //   if (!writeReg(REG_OOR_THR_MSB, thr_msb)) return false;
// // //   if (!writeReg(REG_OOR_RANGE, rangePa)) return false;

// // //   uint8_t cfg = 0;
// // //   cfg |= (thr_b16 & 0x01u);
// // //   cfg |= (countLimitCode & 0x0Fu) << 4;
// // //   return writeReg(REG_OOR_CONFIG, cfg);
// // // }

// // // /* =========================
// // //  * Low-level bus operations
// // //  * ========================= */

// // // bool BMP585_7Semi::writeReg(uint8_t reg, uint8_t value) {
// // //   if (bus == Interface::I2C) {
// // //     /**
// // //      * I2C write
// // //      * - Writes 7-bit register address, then 1 data byte
// // //      */
// // //     if (!i2c) return false;

// // //     i2c->beginTransmission(address);
// // //     i2c->write(reg & 0x7Fu);
// // //     i2c->write(value);
// // //     return (i2c->endTransmission() == 0);
// // //   }

// // //   /**
// // //    * SPI write
// // //    * - First byte: W + A6..A0 (W=0)
// // //    * - Then 1 data byte
// // //    */
// // //   if (!spi || cs_pin < 0) return false;

// // //   digitalWrite(cs_pin, LOW);
// // //   spi->beginTransaction(spiSettings);

// // //   spi->transfer(reg & 0x7Fu);
// // //   spi->transfer(value);

// // //   spi->endTransaction();
// // //   digitalWrite(cs_pin, HIGH);
// // //   return true;
// // // }

// // // bool BMP585_7Semi::readReg(uint8_t reg, uint8_t &value) {
// // //   return readNReg(reg, &value, 1);
// // // }

// // // bool BMP585_7Semi::readNReg(uint8_t reg, uint8_t *buf, size_t len) {
// // //   if (!buf || len == 0) return false;

// // //   if (bus == Interface::I2C) {
// // //     /**
// // //      * I2C burst read
// // //      * - Write register address (no stop)
// // //      * - Request N bytes
// // //      */
// // //     if (!i2c) return false;

// // //     i2c->beginTransmission(address);
// // //     i2c->write(reg & 0x7Fu);
// // //     if (i2c->endTransmission(false) != 0) return false;

// // //     size_t got = i2c->requestFrom((int)address, (int)len);
// // //     if (got != len) return false;

// // //     for (size_t i = 0; i < len; i++) {
// // //       buf[i] = i2c->read();
// // //     }
// // //     return true;
// // //   }

// // //   /**
// // //    * SPI burst read
// // //    * - First byte: R + A6..A0 (R=1)
// // //    * - Then clock out N bytes by sending dummy bytes
// // //    */
// // //   if (!spi || cs_pin < 0) return false;

// // //   digitalWrite(cs_pin, LOW);
// // //   spi->beginTransaction(spiSettings);

// // //   spi->transfer((reg & 0x7Fu) | 0x80u);
// // //   for (size_t i = 0; i < len; i++) {
// // //     buf[i] = spi->transfer(0x00);
// // //   }

// // //   spi->endTransaction();
// // //   digitalWrite(cs_pin, HIGH);
// // //   return true;
// // // }

// // // bool BMP585_7Semi::setBits(uint8_t reg, uint8_t mask, uint8_t valueShifted) {
// // //   /**
// // //    * Read-modify-write helper
// // //    * - Clears bits in mask, then ORs in masked valueShifted
// // //    */
// // //   uint8_t v = 0;
// // //   if (!readReg(reg, v)) return false;
// // //   v = (v & ~mask) | (valueShifted & mask);
// // //   return writeReg(reg, v);
// // // }

// // // /* =========================
// // //  * Raw conversions
// // //  * ========================= */

// // // int32_t BMP585_7Semi::signExtend24(uint32_t x) {
// // //   /**
// // //    * Sign extend a 24-bit two's complement value into int32_t
// // //    * - Keeps raw math predictable for both temperature and pressure paths
// // //    */
// // //   x &= 0xFFFFFFu;
// // //   if (x & 0x800000u) return (int32_t)(x | 0xFF000000u);
// // //   return (int32_t)x;
// // // }

// // // float BMP585_7Semi::tempFromRaw(int32_t raw24) {
// // //   /**
// // //    * Temp_Data arithmetic representation: signed (24,16) [degC]
// // //    * - value = raw / 2^16
// // //    */
// // //   return float(raw24) / 65536.0f;
// // // }

// // // float BMP585_7Semi::pressFromRaw(int32_t raw24) {
// // //   /**
// // //    * Press_Data arithmetic representation: signed (24,6) [Pa]
// // //    * - value = raw / 2^6
// // //    */
// // //   return float(raw24) / 64.0f;
// // // }

// // // /* =========================
// // //  * NVM helpers
// // //  * ========================= */

// // // BMP585_7Semi::NVMStatus BMP585_7Semi::readNVMStatus() {
// // //   /**
// // //    * NVM status from STATUS (0x28)
// // //    * - rdy: STATUS.status_nvm_rdy
// // //    * - err: STATUS.status_nvm_err
// // //    * - cmd_err: STATUS.status_nvm_cmd_err
// // //    */
// // //   NVMStatus st;
// // //   uint8_t v = 0;
// // //   if (!readReg(REG_STATUS, v)) return st;

// // //   st.rdy     = (v & (1u << 1)) != 0;
// // //   st.err     = (v & (1u << 2)) != 0;
// // //   st.cmd_err = (v & (1u << 3)) != 0;
// // //   return st;
// // // }

// // // bool BMP585_7Semi::waitNVMReady(uint32_t timeout_us) {
// // //   /** Wait until STATUS.status_nvm_rdy is 1
// // //    *
// // //    * - Timeout is host-side safety
// // //    * - NVM read is ~200us typical, NVM program is ~10ms typical
// // //    */
// // //   const uint32_t start = micros();
// // //   while ((uint32_t)(micros() - start) < timeout_us) {
// // //     auto st = readNVMStatus();
// // //     if (st.rdy) return true;
// // //     delayMicroseconds(50);
// // //   }
// // //   return false;
// // // }

// // // bool BMP585_7Semi::writeCMDSequence(uint8_t b0, uint8_t b1) {
// // //   /** CMD sequence must be a dedicated write transaction
// // //    *
// // //    * - Datasheet requires CMD writes are not combined with other register writes in a burst
// // //    */
// // //   if (!writeReg(REG_CMD, b0)) return false;
// // //   if (!writeReg(REG_CMD, b1)) return false;
// // //   return true;
// // // }

// // // bool BMP585_7Semi::readNVMUserRow(uint8_t rowAddr, uint16_t &out) {
// // //   out = 0;

// // //   /** User range is rows 0x20..0x22 (2 bytes each) */
// // //   if (rowAddr < 0x20 || rowAddr > 0x22) return false;

// // //   /** Switch to STANDBY and disable deep standby */
// // //   if (!setPowerMode(PowerMode::Standby)) return false;

// // //   uint8_t odr = 0;
// // //   if (!readReg(REG_ODR_CONFIG, odr)) return false;
// // //   odr |= (1u << 7); /* deep_dis = 1 */
// // //   if (!writeReg(REG_ODR_CONFIG, odr)) return false;

// // //   /** Wait for NVM ready */
// // //   if (!waitNVMReady(20000)) return false;

// // //   /** Write NVM_ADDR with prog_en=0 and row address */
// // //   uint8_t nvm_addr = (rowAddr & 0x3Fu); /* bits[5:0] */
// // //   if (!writeReg(REG_NVM_ADDR, nvm_addr)) return false;

// // //   /** CMD = USR_READ sequence (0x5D, 0xA5) */
// // //   if (!writeCMDSequence(0x5D, 0xA5)) return false;

// // //   /** Wait until ready again */
// // //   if (!waitNVMReady(20000)) return false;

// // //   /** Check status error flags */
// // //   auto st = readNVMStatus();
// // //   if (st.err || st.cmd_err) return false;

// // //   /** Read data regs */
// // //   uint8_t l = 0, h = 0;
// // //   if (!readReg(REG_NVM_DATA_LSB, l)) return false;
// // //   if (!readReg(REG_NVM_DATA_MSB, h)) return false;

// // //   out = (uint16_t(h) << 8) | uint16_t(l);
// // //   return true;
// // // }

// // // bool BMP585_7Semi::writeNVMUserRow(uint8_t rowAddr, uint16_t value) {
// // //   /** User range is rows 0x20..0x22 (2 bytes each) */
// // //   if (rowAddr < 0x20 || rowAddr > 0x22) return false;

// // //   /** Switch to STANDBY and disable deep standby */
// // //   if (!setPowerMode(PowerMode::Standby)) return false;

// // //   uint8_t odr = 0;
// // //   if (!readReg(REG_ODR_CONFIG, odr)) return false;
// // //   odr |= (1u << 7); /* deep_dis = 1 */
// // //   if (!writeReg(REG_ODR_CONFIG, odr)) return false;

// // //   /** Wait for NVM ready */
// // //   if (!waitNVMReady(20000)) return false;

// // //   /** Set NVM_ADDR with prog_en=1 and row address */
// // //   uint8_t nvm_addr = (rowAddr & 0x3Fu) | (1u << 6); /* prog_en bit6 */
// // //   if (!writeReg(REG_NVM_ADDR, nvm_addr)) return false;

// // //   /** Write data regs */
// // //   uint8_t l = uint8_t(value & 0xFFu);
// // //   uint8_t h = uint8_t((value >> 8) & 0xFFu);
// // //   if (!writeReg(REG_NVM_DATA_LSB, l)) return false;
// // //   if (!writeReg(REG_NVM_DATA_MSB, h)) return false;

// // //   /** CMD = USR_PROG sequence (0x5D, 0xA0) */
// // //   if (!writeCMDSequence(0x5D, 0xA0)) return false;

// // //   /** Wait until ready (~10ms typical) */
// // //   if (!waitNVMReady(50000)) return false;

// // //   /** Check status error flags */
// // //   auto st = readNVMStatus();
// // //   if (st.err || st.cmd_err) return false;

// // //   /** Reset prog_en to 0 */
// // //   uint8_t nvm_addr_clear = (rowAddr & 0x3Fu);
// // //   if (!writeReg(REG_NVM_ADDR, nvm_addr_clear)) return false;

// // //   return true;
// // // }
// // #include "7Semi_BMP585.h"

// // /* =========================
// //  * Construction / probing
// //  * ========================= */

// // BMP585_7Semi::BMP585_7Semi() {}

// // bool BMP585_7Semi::probe() {
// //   /**
// //    * Basic presence check
// //    * - Reads CHIP_ID and compares with expected value
// //    * - Works for both I2C and SPI (depending on selected bus)
// //    */
// //   return (chipID() == CHIP_ID_EXPECTED);
// // }

// // /* =========================
// //  * Begin / Interface select
// //  * ========================= */

// // bool BMP585_7Semi::beginI2C(TwoWire &wire, uint8_t i2cAddr, uint32_t i2cSpeed) {
// //   /**
// //    * Start sensor on I2C
// //    * - Stores Wire instance + address
// //    * - Sets bus speed
// //    * - Verifies address ACK and CHIP_ID
// //    */
// //   bus = Interface::I2C;
// //   i2c = &wire;
// //   address = i2cAddr;

// //   i2c->begin();
// //   i2c->setClock(i2cSpeed);

// //   i2c->beginTransmission(address);
// //   if (i2c->endTransmission() != 0) return false;

// //   return probe();
// // }

// // bool BMP585_7Semi::beginSPI(SPIClass &spiBus,
// //                             uint8_t csPin,
// //                             SPISettings settings,
// //                             int sck,
// //                             int miso,
// //                             int mosi)
// // {
// //   /**
// //    * Start sensor on SPI (portable Arduino-core implementation)
// //    * - Works on ESP32 / AVR / STM32 / RP2040(Pico) / SAMD and similar cores
// //    *
// //    * Behavior:
// //    * - Always supports the basic path: spi->begin()
// //    * - Uses pin-mapped begin ONLY where the core supports it
// //    * - Pins are treated as "optional hints":
// //    *   - On AVR: ignored (hardware pins are fixed)
// //    *   - On ESP32: begin(sck,miso,mosi,cs) is supported
// //    *   - On RP2040 / some STM32 cores: pin routing is done via setters
// //    */
// //   bus = Interface::SPI;
// //   spi = &spiBus;
// //   cs_pin = (int8_t)csPin;
// //   spiSettings = settings;

// //   pinMode(cs_pin, OUTPUT);
// //   digitalWrite(cs_pin, HIGH);

// //   /**
// //    * Select the most compatible SPI init path
// //    * - If the user passed valid pins (>=0), attempt platform pin routing
// //    * - Otherwise fall back to plain begin()
// //    */
// //   const bool pinsProvided = (sck >= 0) && (miso >= 0) && (mosi >= 0);

// //   #if defined(ESP32)
// //     /**
// //      * ESP32 supports SPI pin mapping directly via begin(sck,miso,mosi,ss)
// //      * - This is the most reliable path on ESP32 when using non-default pins
// //      */
// //     if (pinsProvided) spi->begin(sck, miso, mosi, cs_pin);
// //     else spi->begin();

// //   #elif defined(ARDUINO_ARCH_RP2040)
// //     /**
// //      * RP2040 (Raspberry Pi Pico) commonly uses setters for pin routing
// //      * - Many cores support SPI.setSCK/setTX/setRX
// //      * - If setters are not available in a particular core fork,
// //      *   it will still compile because we guard with defined() checks below.
// //      */
// //     #if defined(SPI_HAS_PIN_REMAP) || defined(ARDUINO_ARCH_RP2040)
// //       if (pinsProvided) {
// //         #if defined(SPI_HAS_SET_SCK) || defined(SPI_HAS_SET_SCK_PIN) || defined(ARDUINO_ARCH_RP2040)
// //           /**
// //            * Try the most common RP2040 Arduino core APIs
// //            * - Some cores name these setSCK / setTX / setRX
// //            */
// //           #if __has_include(<Arduino.h>)
// //             /* nothing needed */
// //           #endif

// //           /* These calls exist on the Earle Philhower RP2040 core */
// //           spi->setSCK((uint8_t)sck);
// //           spi->setTX((uint8_t)mosi);
// //           spi->setRX((uint8_t)miso);
// //         #endif
// //       }
// //     #endif
// //     spi->begin();

// //   #elif defined(ARDUINO_ARCH_STM32)
// //     /**
// //      * STM32 cores vary:
// //      * - Many use fixed pins per SPI instance (SPI/SPI1/SPI2...)
// //      * - Some variants allow remap via board variant definitions
// //      *
// //      * Safe default:
// //      * - Always call begin()
// //      * - If the user wants alternate pins, they should select the correct SPI instance
// //      *   or configure variant/pinmap at board level.
// //      */
// //     (void)sck;
// //     (void)miso;
// //     (void)mosi;
// //     spi->begin();

// //   #else
// //     /**
// //      * Generic Arduino-core fallback
// //      * - AVR / SAMD / megaAVR / many others
// //      * - Pins are fixed by the board / SPI instance
// //      */
// //     (void)sck;
// //     (void)miso;
// //     (void)mosi;
// //     spi->begin();
// //   #endif

// //   delay(10);

// //   return probe();
// // }

// // /* =========================
// //  * Basic operations
// //  * ========================= */

// // bool BMP585_7Semi::reset() {
// //   /**
// //    * Soft reset command
// //    * - Write 0xB6 to CMD register (0x7E)
// //    * - After reset, user settings go back to defaults
// //    *
// //    * SPI-specific behavior (BMP585):
// //    * - After soft reset, device returns to I2C/I3C primary interface
// //    * - To re-enter SPI mode, perform a dummy SPI read while CS is low
// //    *   - Dummy read data is invalid
// //    *   - Follow-up reads are valid
// //    */
// //   if (!writeReg(REG_CMD, 0xB6)) return false;
// //   delay(5);

// //   if (bus == Interface::SPI) {
// //     /**
// //      * Dummy SPI read to switch interface back to SPI
// //      * - Reading CHIP_ID is safe and simple
// //      */
// //     uint8_t junk = 0;
// //     (void)readReg(REG_CHIP_ID, junk);
// //     delayMicroseconds(5);
// //   }

// //   return true;
// // }

// // uint8_t BMP585_7Semi::chipID() {
// //   uint8_t v = 0;
// //   (void)readReg(REG_CHIP_ID, v);
// //   return v;
// // }

// // uint8_t BMP585_7Semi::revisionID() {
// //   uint8_t v = 0;
// //   (void)readReg(REG_REV_ID, v);
// //   return v;
// // }

// // /* =========================
// //  * Configuration
// //  * ========================= */

// // bool BMP585_7Semi::setOSR(OSR osr_t, OSR osr_p, bool pressEnable) {
// //   /**
// //    * OSR_CONFIG (0x36)
// //    * - bits [2:0] osr_t
// //    * - bits [5:3] osr_p
// //    * - bit 6 press_en
// //    */
// //   uint8_t v = 0;
// //   v |= (uint8_t(osr_t) & 0x07u);
// //   v |= (uint8_t(osr_p) & 0x07u) << 3;
// //   if (pressEnable) v |= (1u << 6);
// //   return writeReg(REG_OSR_CONFIG, v);
// // }

// // bool BMP585_7Semi::setODR(ODR odr, PowerMode mode, bool deepStandbyDisable) {
// //   /**
// //    * ODR_CONFIG (0x37)
// //    * - bits [1:0] pwr_mode
// //    * - bits [6:2] odr
// //    * - bit 7 deep_dis (1 disables deep-standby)
// //    */
// //   uint8_t v = 0;
// //   v |= (uint8_t(mode) & 0x03u);
// //   v |= (uint8_t(odr) & 0x1Fu) << 2;
// //   if (deepStandbyDisable) v |= (1u << 7);
// //   return writeReg(REG_ODR_CONFIG, v);
// // }

// // bool BMP585_7Semi::setPowerMode(PowerMode mode) {
// //   /**
// //    * Update only pwr_mode bits in ODR_CONFIG
// //    * - Keeps ODR + deep_dis unchanged
// //    */
// //   uint8_t reg = 0;
// //   if (!readReg(REG_ODR_CONFIG, reg)) return false;
// //   reg = (reg & ~0x03u) | (uint8_t(mode) & 0x03u);
// //   return writeReg(REG_ODR_CONFIG, reg);
// // }

// // bool BMP585_7Semi::setIIR(uint8_t iir_t, uint8_t iir_p) {
// //   /**
// //    * DSP_IIR (0x31)
// //    * - Raw field access for temperature and pressure IIR strength
// //    * - Practical mapping in this library:
// //    *   - iir_t in bits [2:0]
// //    *   - iir_p in bits [5:3]
// //    */
// //   uint8_t v = 0;
// //   v |= (iir_t & 0x07u);
// //   v |= (iir_p & 0x07u) << 3;
// //   return writeReg(REG_DSP_IIR, v);
// // }

// // /* =========================
// //  * Reads
// //  * ========================= */

// // float BMP585_7Semi::readTemperatureC() {
// //   float t = NAN, p = NAN;
// //   if (!readTemperaturePressure(t, p)) return NAN;
// //   return t;
// // }

// // float BMP585_7Semi::readPressurePa() {
// //   float t = NAN, p = NAN;
// //   if (!readTemperaturePressure(t, p)) return NAN;
// //   return p;
// // }

// // bool BMP585_7Semi::readTemperaturePressure(float &tempC, float &pressPa) {
// //   /**
// //    * Burst read temperature + pressure for a consistent snapshot
// //    * - TEMP:  0x1D..0x1F (3 bytes)
// //    * - PRESS: 0x20..0x22 (3 bytes)
// //    */
// //   uint8_t buf[6] = {0};
// //   if (!readNReg(REG_TEMP_XLSB, buf, sizeof(buf))) return false;

// //   uint32_t t24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
// //   uint32_t p24u = (uint32_t(buf[5]) << 16) | (uint32_t(buf[4]) << 8) | uint32_t(buf[3]);

// //   tempC  = tempFromRaw(signExtend24(t24u));
// //   pressPa = pressFromRaw(signExtend24(p24u));
// //   return true;
// // }

// // float BMP585_7Semi::readAltitudeM(float seaLevel_hPa) {
// //   /**
// //    * Altitude estimate using standard barometric approximation
// //    * - Uses pressure in hPa for computation
// //    * - seaLevel_hPa must be > 0
// //    */
// //   float p_pa = readPressurePa();
// //   if (!isfinite(p_pa) || seaLevel_hPa <= 0.0f) return NAN;

// //   float p_hpa = p_pa / 100.0f;
// //   return 44330.0f * (1.0f - powf(p_hpa / seaLevel_hPa, 0.19029495f));
// // }

// // /* =========================
// //  * Low-level bus operations
// //  * ========================= */

// // bool BMP585_7Semi::writeReg(uint8_t reg, uint8_t value) {
// //   if (bus == Interface::I2C) {
// //     /**
// //      * I2C write
// //      * - Writes register address, then 1 data byte
// //      */
// //     if (!i2c) return false;

// //     i2c->beginTransmission(address);
// //     i2c->write(reg & 0x7Fu);
// //     i2c->write(value);
// //     return (i2c->endTransmission() == 0);
// //   }

// //   /**
// //    * SPI write
// //    * - First byte: W + A6..A0 (W=0)
// //    * - Then 1 data byte
// //    */
// //   if (!spi || cs_pin < 0) return false;

// //   digitalWrite(cs_pin, LOW);
// //   spi->beginTransaction(spiSettings);

// //   spi->transfer(reg & 0x7Fu);
// //   spi->transfer(value);

// //   spi->endTransaction();
// //   digitalWrite(cs_pin, HIGH);
// //   return true;
// // }

// // bool BMP585_7Semi::readReg(uint8_t reg, uint8_t &value) {
// //   return readNReg(reg, &value, 1);
// // }

// // bool BMP585_7Semi::readNReg(uint8_t reg, uint8_t *buf, size_t len) {
// //   if (!buf || len == 0) return false;

// //   if (bus == Interface::I2C) {
// //     /**
// //      * I2C burst read
// //      * - Write register address (no stop)
// //      * - Request N bytes
// //      */
// //     if (!i2c) return false;

// //     i2c->beginTransmission(address);
// //     i2c->write(reg & 0x7Fu);
// //     if (i2c->endTransmission(false) != 0) return false;

// //     size_t got = i2c->requestFrom((int)address, (int)len);
// //     if (got != len) return false;

// //     for (size_t i = 0; i < len; i++) buf[i] = i2c->read();
// //     return true;
// //   }

// //   /**
// //    * SPI burst read
// //    * - First byte: R + A6..A0 (R=1)
// //    * - Then read N bytes by sending dummy bytes
// //    */
// //   if (!spi || cs_pin < 0) return false;

// //   digitalWrite(cs_pin, LOW);
// //   spi->beginTransaction(spiSettings);

// //   spi->transfer((reg & 0x7Fu) | 0x80u);
// //   for (size_t i = 0; i < len; i++) buf[i] = spi->transfer(0x00);

// //   spi->endTransaction();
// //   digitalWrite(cs_pin, HIGH);
// //   return true;
// // }

// // bool BMP585_7Semi::setBits(uint8_t reg, uint8_t mask, uint8_t valueShifted) {
// //   /**
// //    * Read-modify-write helper
// //    * - Clears bits in mask, then ORs in masked valueShifted
// //    */
// //   uint8_t v = 0;
// //   if (!readReg(reg, v)) return false;
// //   v = (v & ~mask) | (valueShifted & mask);
// //   return writeReg(reg, v);
// // }

// // /* =========================
// //  * Raw conversions
// //  * ========================= */

// // int32_t BMP585_7Semi::signExtend24(uint32_t x) {
// //   /**
// //    * Sign extend a 24-bit two's complement value into int32_t
// //    */
// //   x &= 0xFFFFFFu;
// //   if (x & 0x800000u) return (int32_t)(x | 0xFF000000u);
// //   return (int32_t)x;
// // }

// // float BMP585_7Semi::tempFromRaw(int32_t raw24) {
// //   /**
// //    * Temp_Data arithmetic representation: signed (24,16) [degC]
// //    * - value = raw / 2^16
// //    */
// //   return float(raw24) / 65536.0f;
// // }

// // float BMP585_7Semi::pressFromRaw(int32_t raw24) {
// //   /**
// //    * Press_Data arithmetic representation: signed (24,6) [Pa]
// //    * - value = raw / 2^6
// //    */
// //   return float(raw24) / 64.0f;
// // }

// // /* =========================
// //  * NVM helpers
// //  * ========================= */

// // BMP585_7Semi::NVMStatus BMP585_7Semi::readNVMStatus() {
// //   /**
// //    * NVM status from STATUS (0x28)
// //    * - rdy: STATUS.status_nvm_rdy
// //    * - err: STATUS.status_nvm_err
// //    * - cmd_err: STATUS.status_nvm_cmd_err
// //    */
// //   NVMStatus st;
// //   uint8_t v = 0;
// //   if (!readReg(REG_STATUS, v)) return st;

// //   st.rdy     = (v & (1u << 1)) != 0;
// //   st.err     = (v & (1u << 2)) != 0;
// //   st.cmd_err = (v & (1u << 3)) != 0;
// //   return st;
// // }

// // bool BMP585_7Semi::waitNVMReady(uint32_t timeout_us) {
// //   /** Wait until STATUS.status_nvm_rdy is 1
// //    *
// //    * - Timeout is host-side safety
// //    * - NVM read is ~200us typical, NVM program is ~10ms typical
// //    */
// //   const uint32_t start = micros();
// //   while ((uint32_t)(micros() - start) < timeout_us) {
// //     auto st = readNVMStatus();
// //     if (st.rdy) return true;
// //     delayMicroseconds(50);
// //   }
// //   return false;
// // }

// // bool BMP585_7Semi::writeCMDSequence(uint8_t b0, uint8_t b1) {
// //   /** CMD sequence must be a dedicated write transaction
// //    *
// //    * - Do not combine CMD writes with other register writes in a burst
// //    */
// //   if (!writeReg(REG_CMD, b0)) return false;
// //   if (!writeReg(REG_CMD, b1)) return false;
// //   return true;
// // }

// // bool BMP585_7Semi::readNVMUserRow(uint8_t rowAddr, uint16_t &out) {
// //   out = 0;

// //   /** User range is rows 0x20..0x22 (2 bytes each) */
// //   if (rowAddr < 0x20 || rowAddr > 0x22) return false;

// //   /** Switch to STANDBY and disable deep standby */
// //   if (!setPowerMode(PowerMode::Standby)) return false;

// //   uint8_t odr = 0;
// //   if (!readReg(REG_ODR_CONFIG, odr)) return false;
// //   odr |= (1u << 7);
// //   if (!writeReg(REG_ODR_CONFIG, odr)) return false;

// //   /** Wait for NVM ready */
// //   if (!waitNVMReady(20000)) return false;

// //   /** Write NVM_ADDR with prog_en=0 and row address */
// //   uint8_t nvm_addr = (rowAddr & 0x3Fu);
// //   if (!writeReg(REG_NVM_ADDR, nvm_addr)) return false;

// //   /** CMD = USR_READ sequence (0x5D, 0xA5) */
// //   if (!writeCMDSequence(0x5D, 0xA5)) return false;

// //   /** Wait until ready again */
// //   if (!waitNVMReady(20000)) return false;

// //   /** Check status error flags */
// //   auto st = readNVMStatus();
// //   if (st.err || st.cmd_err) return false;

// //   /** Read data regs */
// //   uint8_t l = 0, h = 0;
// //   if (!readReg(REG_NVM_DATA_LSB, l)) return false;
// //   if (!readReg(REG_NVM_DATA_MSB, h)) return false;

// //   out = (uint16_t(h) << 8) | uint16_t(l);
// //   return true;
// // }

// // bool BMP585_7Semi::writeNVMUserRow(uint8_t rowAddr, uint16_t value) {
// //   /** User range is rows 0x20..0x22 (2 bytes each) */
// //   if (rowAddr < 0x20 || rowAddr > 0x22) return false;

// //   /** Switch to STANDBY and disable deep standby */
// //   if (!setPowerMode(PowerMode::Standby)) return false;

// //   uint8_t odr = 0;
// //   if (!readReg(REG_ODR_CONFIG, odr)) return false;
// //   odr |= (1u << 7);
// //   if (!writeReg(REG_ODR_CONFIG, odr)) return false;

// //   /** Wait for NVM ready */
// //   if (!waitNVMReady(20000)) return false;

// //   /** Set NVM_ADDR with prog_en=1 and row address */
// //   uint8_t nvm_addr = (rowAddr & 0x3Fu) | (1u << 6);
// //   if (!writeReg(REG_NVM_ADDR, nvm_addr)) return false;

// //   /** Write data regs */
// //   uint8_t l = uint8_t(value & 0xFFu);
// //   uint8_t h = uint8_t((value >> 8) & 0xFFu);
// //   if (!writeReg(REG_NVM_DATA_LSB, l)) return false;
// //   if (!writeReg(REG_NVM_DATA_MSB, h)) return false;

// //   /** CMD = USR_PROG sequence (0x5D, 0xA0) */
// //   if (!writeCMDSequence(0x5D, 0xA0)) return false;

// //   /** Wait until ready (~10ms typical) */
// //   if (!waitNVMReady(50000)) return false;

// //   /** Check status error flags */
// //   auto st = readNVMStatus();
// //   if (st.err || st.cmd_err) return false;

// //   /** Reset prog_en to 0 */
// //   uint8_t nvm_addr_clear = (rowAddr & 0x3Fu);
// //   if (!writeReg(REG_NVM_ADDR, nvm_addr_clear)) return false;

// //   return true;
// // }
// #include "7Semi_BMP585.h"

// BMP585_7Semi::BMP585_7Semi() {}

// bool BMP585_7Semi::beginI2C(TwoWire &wire, uint8_t i2cAddr, uint32_t i2cSpeed, uint8_t i2cSDA, uint8_t i2cSCL) {
//   /**
//    * Start sensor on I2C
//    * - Stores Wire instance + address
//    * - Sets bus speed
//    * - Verifies address ACK and CHIP_ID
//    * - Applies default sensor configuration so it works out-of-box
//    */
//   bus = Interface::I2C;
//   i2c = &wire;
//   address = i2cAddr;

//   #if defined(ESP32)
//     /**
//      * ESP32 supports I2C pin mapping via begin(sda,scl)
//      * - If pins are not provided, fall back to default I2C pins
//      */
//     if ((i2cSDA >= 0) && (i2cSCL >= 0)) 
//      i2c->begin(i2cSDA, i2cSCL);
//     else i2c->begin();
//   #else
//     /**
//      * Generic Arduino-core behavior
//      * - SPI pin routing is defined by the board core / SPI instance
//      */
//     (void)i2cSDA;
//     (void)i2cSCL;
//     i2c->begin();
//   #endif
//   i2c->setClock(i2cSpeed);

//   i2c->beginTransmission(address);
//   if (i2c->endTransmission() != 0) return false;

//   uint8_t chip_id = 0;
//   if (!chipID(chip_id)) return false;

//   return initSensor();
// }

// bool BMP585_7Semi::beginSPI(SPIClass &spiBus,
//                             uint8_t csPin,
//                             SPISettings settings,
//                             int sck,
//                             int miso,
//                             int mosi)
// {
//   /**
//    * Start sensor on SPI (portable Arduino-core implementation)
//    * - Stores SPI instance + CS pin + SPI settings
//    * - Initializes SPI bus
//    * - Verifies CHIP_ID
//    * - Applies default sensor configuration so it works out-of-box
//    *
//    * Notes:
//    * - On most Arduino cores, SPI pins are fixed by the board/SPI instance.
//    * - ESP32 supports runtime pin mapping using begin(sck,miso,mosi,cs).
//    */
//   bus = Interface::SPI;
//   spi = &spiBus;
//   cs_pin = (int8_t)csPin;
//   spiSettings = settings;

//   pinMode(cs_pin, OUTPUT);
//   digitalWrite(cs_pin, HIGH);

//   #if defined(ESP32)
//     /**
//      * ESP32 supports SPI pin mapping via begin(sck,miso,mosi,ss)
//      * - If pins are not provided, fall back to default SPI pins
//      */
//     if ((sck != -1) && (miso != -1) && (mosi != -1)) 
//        spi->begin(sck, miso, mosi, cs_pin);
//     else spi->begin();
//   #else
//     /**
//      * Generic Arduino-core behavior
//      * - SPI pin routing is defined by the board core / SPI instance
//      */
//     (void)sck;
//     (void)miso;
//     (void)mosi;
//     spi->begin();
//   #endif

//   delay(10);

//   return initSensor();
// }

// /* =========================
//  * Beginner-friendly defaults
//  * ========================= */

// bool BMP585_7Semi::initSensor() {
//   /**
//    * Apply safe default configuration so the sensor works without user setup
//    * - reset(): restores defaults and handles SPI re-entry (dummy read)
//    * - OSR: x8/x8 with pressure enabled for stable readings
//    * - IIR: off by default (user can enable later)
//    * - ODR: 10 Hz, Normal mode, deep standby disabled for reliability
//    */
//   // if (!reset()) return false;
//   reset();
//   delay(10);
//   if (!setOSR(OSR::x8, OSR::x8, true))
//    return false;
// if (!setIIR(IIRCoeff::OFF, IIRCoeff::OFF))
//     return false;


//   /**
//    * deepStandbyDisable=true
//    * - Makes “continuous printing” sketches behave consistently
//    * - User can enable deep standby later for low-power applications
//    */
//   if (!setODR(ODR::Hz10, PowerMode::Normal, true)) return false;

//   delay(20);
//   return true;
// }

// /* =========================
//  * Basic operations
//  * ========================= */

// bool BMP585_7Semi::reset() {
//   /**
//    * Soft reset command
//    * - Write 0xB6 to CMD register (0x7E)
//    * - After reset, user settings go back to defaults
//    *
//    * SPI-specific behavior (BMP585):
//    * - After soft reset, device returns to I2C/I3C primary interface
//    * - To re-enter SPI mode, perform a dummy SPI read while CS is low
//    *   - Dummy read data is invalid
//    *   - Follow-up reads are valid
//    */
//   // if (!writeReg(REG_CMD, 0xB6)) return false;
//   writeReg(REG_CMD, 0xB6);
//   delay(10);

//   if (bus == Interface::SPI) {
//     /**
//      * Dummy SPI read to switch interface back to SPI
//      * - Reading CHIP_ID is safe and simple
//      */
//     uint8_t chip_id = 0;
//     if(!chipID(chip_id))
//       return false;
//     delayMicroseconds(5);
//   }
//   return true;
// }

// bool BMP585_7Semi::chipID( uint8_t &chip_id) {
//   uint8_t v = 0;
//   if(!readReg(REG_CHIP_ID, chip_id))
//     return false;
//     if(chip_id != CHIP_ID_EXPECTED)
//       return false;
//   return true;
// }

// uint8_t BMP585_7Semi::revisionID() {
//   uint8_t v = 0;
//   (void)readReg(REG_REV_ID, v);
//   return v;
// }

// /* =========================
//  * Configuration
//  * ========================= */

// bool BMP585_7Semi::setOSR(OSR osr_t, OSR osr_p, bool pressEnable) {
//   /**
//    * OSR_CONFIG (0x36)
//    * - bits [2:0] osr_t
//    * - bits [5:3] osr_p
//    * - bit 6 press_en
//    */
//   uint8_t v = 0;
//   v |= (uint8_t(osr_t) & 0x07u);
//   v |= (uint8_t(osr_p) & 0x07u) << 3;
//   if (pressEnable) v |= (1u << 6);
//   return writeReg(REG_OSR_CONFIG, v);
// }

// bool BMP585_7Semi::setODR(ODR odr, PowerMode mode, bool deepStandbyDisable) {
//   /**
//    * ODR_CONFIG (0x37)
//    * - bits [1:0] pwr_mode
//    * - bits [6:2] odr
//    * - bit 7 deep_dis (1 disables deep-standby)
//    */
//   uint8_t v = 0;
//   v |= (uint8_t(mode) & 0x03u);
//   v |= (uint8_t(odr) & 0x1Fu) << 2;
//   if (deepStandbyDisable) v |= (1u << 7);
//   return writeReg(REG_ODR_CONFIG, v);
// }

// bool BMP585_7Semi::setPowerMode(PowerMode mode) {
//   /**
//    * Update only pwr_mode bits in ODR_CONFIG
//    * - Keeps ODR + deep_dis unchanged
//    */
//   uint8_t reg = 0;
//   if (!readReg(REG_ODR_CONFIG, reg)) return false;
//   reg = (reg & ~0x03u) | (uint8_t(mode) & 0x03u);
//   return writeReg(REG_ODR_CONFIG, reg);
// }

// bool BMP585_7Semi::setIIR(IIRCoeff iir_t, IIRCoeff iir_p)
// {
//   // Datasheet: IIR config must be changed in STANDBY
//   // Save current power mode
//   uint8_t odr = 0;
//   if (!readReg(REG_ODR_CONFIG, odr)) return false;

//   uint8_t prevMode = odr & 0x03;

//   // Force STANDBY if needed
//   if (prevMode != uint8_t(PowerMode::Standby)) {
//     odr = (odr & ~0x03) | uint8_t(PowerMode::Standby);
//     if (!writeReg(REG_ODR_CONFIG, odr)) return false;
//     delay(2);
//   }

//   // Build DSP_IIR value
//   uint8_t v = 0;
//   v |= (uint8_t(iir_t) & 0x07u);        // bits [2:0] temperature
//   v |= (uint8_t(iir_p) & 0x07u) << 3;   // bits [5:3] pressure

//   if (!writeReg(REG_DSP_IIR, v)) return false;

//   // Restore previous power mode
//   if (prevMode != uint8_t(PowerMode::Standby)) {
//     odr = (odr & ~0x03) | prevMode;
//     if (!writeReg(REG_ODR_CONFIG, odr)) return false;
//   }

//   return true;
// }


// /* =========================
//  * Reads
//  * ========================= */

// bool  BMP585_7Semi::readTemperatureC( float & tempC) {
//   float t = NAN, p = NAN;
//   if (!readTemperaturePressure(t, p)) return false;
//   tempC = t;
//   return true;
// }

// bool BMP585_7Semi::readPressurePa(float & pressPa) {
//   float t = NAN, p = NAN;
//   if (!readTemperaturePressure(t, p)) return false;
//   pressPa = p;
//   return true;
// }

// bool BMP585_7Semi::readTemperaturePressure(float &tempC, float &pressPa) {
//   /**
//    * Burst read temperature + pressure for a consistent snapshot
//    * - TEMP:  0x1D..0x1F (3 bytes)
//    * - PRESS: 0x20..0x22 (3 bytes)
//    */
//   uint8_t buf[6] = {0};
//   if (!readNReg(REG_TEMP_XLSB, buf, sizeof(buf))) return false;

//   uint32_t t24u = (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
//   uint32_t p24u = (uint32_t(buf[5]) << 16) | (uint32_t(buf[4]) << 8) | uint32_t(buf[3]);

//   tempC   = tempFromRaw(signExtend24(t24u));
//   pressPa = pressFromRaw(signExtend24(p24u));
//   return true;
// }

// bool BMP585_7Semi::readAltitudeM(float seaLevel_hPa, float &altitude_m) {
//   /**
//    * Altitude estimate using standard barometric approximation
//    * - Uses pressure in hPa for computation
//    * - seaLevel_hPa must be > 0
//    */
//   if(seaLevel_hPa <= 0.0f) return false;
//   float p_pa = 0;
//   if (!readPressurePa(p_pa)) return false;
//   if (!isfinite(p_pa) || seaLevel_hPa <= 0.0f) return false;

//   float p_hpa = p_pa / 100.0f;
//   altitude_m = 44330.0f * (1.0f - powf(p_hpa / seaLevel_hPa, 0.19029495f));
//   return true;
// }


// bool BMP585_7Semi::writeReg(uint8_t reg, uint8_t value) {
//   if (bus == Interface::I2C) {
//     /**
//      * I2C write
//      * - Writes register address, then 1 data byte
//      */
//     if (!i2c) return false;

//     i2c->beginTransmission(address);
//     i2c->write(reg & 0x7Fu);
//     i2c->write(value);
//     return (i2c->endTransmission() == 0);
//   }

//   /**
//    * SPI write
//    * - First byte: W + A6..A0 (W=0)
//    * - Then 1 data byte
//    */
//   if (!spi || cs_pin < 0) return false;

//   digitalWrite(cs_pin, LOW);
//   spi->beginTransaction(spiSettings);

//   spi->transfer(reg & 0x7Fu);
//   spi->transfer(value);

//   spi->endTransaction();
//   digitalWrite(cs_pin, HIGH);
//   return true;
// }

// bool BMP585_7Semi::readReg(uint8_t reg, uint8_t &value) {
//   return readNReg(reg, &value, 1);
// }

// bool BMP585_7Semi::readNReg(uint8_t reg, uint8_t *buf, size_t len) {
//   if (!buf || len == 0) return false;

//   if (bus == Interface::I2C) {
//     /**
//      * I2C burst read
//      * - Write register address (no stop)
//      * - Request N bytes
//      */
//     if (!i2c) return false;

//     i2c->beginTransmission(address);
//     i2c->write(reg & 0x7Fu);
//     if (i2c->endTransmission(false) != 0) return false;

//     size_t got = i2c->requestFrom((int)address, (int)len);
//     if (got != len) return false;

//     for (size_t i = 0; i < len; i++) buf[i] = i2c->read();
//     return true;
//   }

//   /**
//    * SPI burst read
//    * - First byte: R + A6..A0 (R=1)
//    * - Then read N bytes by sending dummy bytes
//    */
//   if (!spi || cs_pin < 0) return false;

//   digitalWrite(cs_pin, LOW);
//   spi->beginTransaction(spiSettings);

//   spi->transfer((reg & 0x7Fu) | 0x80u);
//   for (size_t i = 0; i < len; i++) buf[i] = spi->transfer(0x00);

//   spi->endTransaction();
//   digitalWrite(cs_pin, HIGH);
//   return true;
// }

// bool BMP585_7Semi::setBits(uint8_t reg, uint8_t mask, uint8_t valueShifted) {
//   /**
//    * Read-modify-write helper
//    * - Clears bits in mask, then ORs in masked valueShifted
//    */
//   uint8_t v = 0;
//   if (!readReg(reg, v)) return false;
//   v = (v & ~mask) | (valueShifted & mask);
//   return writeReg(reg, v);
// }

// /* =========================
//  * Raw conversions
//  * ========================= */

// int32_t BMP585_7Semi::signExtend24(uint32_t x) {
//   /**
//    * Sign extend a 24-bit two's complement value into int32_t
//    */
//   x &= 0xFFFFFFu;
//   if (x & 0x800000u) return (int32_t)(x | 0xFF000000u);
//   return (int32_t)x;
// }

// float BMP585_7Semi::tempFromRaw(int32_t raw24) {
//   /**
//    * Temp_Data arithmetic representation: signed (24,16) [degC]
//    * - value = raw / 2^16
//    */
//   return float(raw24) / 65536.0f;
// }

// float BMP585_7Semi::pressFromRaw(int32_t raw24) {
//   /**
//    * Press_Data arithmetic representation: signed (24,6) [Pa]
//    * - value = raw / 2^6
//    */
//   return float(raw24) / 64.0f;
// }

// /* =========================
//  * NVM helpers
//  * ========================= */

// BMP585_7Semi::NVMStatus BMP585_7Semi::readNVMStatus() {
//   /**
//    * NVM status from STATUS (0x28)
//    * - rdy: STATUS.status_nvm_rdy
//    * - err: STATUS.status_nvm_err
//    * - cmd_err: STATUS.status_nvm_cmd_err
//    */
//   NVMStatus st;
//   uint8_t v = 0;
//   if (!readReg(REG_STATUS, v)) return st;

//   st.rdy     = (v & (1u << 1)) != 0;
//   st.err     = (v & (1u << 2)) != 0;
//   st.cmd_err = (v & (1u << 3)) != 0;
//   return st;
// }

// bool BMP585_7Semi::waitNVMReady(uint32_t timeout_us) {
//   /** Wait until STATUS.status_nvm_rdy is 1
//    *
//    * - Timeout is host-side safety
//    * - NVM read is ~200us typical, NVM program is ~10ms typical
//    */
//   const uint32_t start = micros();
//   while ((uint32_t)(micros() - start) < timeout_us) {
//     auto st = readNVMStatus();
//     if (st.rdy) return true;
//     delayMicroseconds(50);
//   }
//   return false;
// }

// bool BMP585_7Semi::writeCMDSequence(uint8_t b0, uint8_t b1) {
//   /** CMD sequence must be a dedicated write transaction
//    *
//    * - Do not combine CMD writes with other register writes in a burst
//    */
//   if (!writeReg(REG_CMD, b0)) return false;
//   if (!writeReg(REG_CMD, b1)) return false;
//   return true;
// }

// bool BMP585_7Semi::readNVMUserRow(uint8_t rowAddr, uint16_t &out) {
//   out = 0;

//   /** User range is rows 0x20..0x22 (2 bytes each) */
//   if (rowAddr < 0x20 || rowAddr > 0x22) return false;

//   /** Switch to STANDBY and disable deep standby */
//   if (!setPowerMode(PowerMode::Standby)) return false;

//   uint8_t odr = 0;
//   if (!readReg(REG_ODR_CONFIG, odr)) return false;
//   odr |= (1u << 7);
//   if (!writeReg(REG_ODR_CONFIG, odr)) return false;

//   /** Wait for NVM ready */
//   if (!waitNVMReady(20000)) return false;

//   /** Write NVM_ADDR with prog_en=0 and row address */
//   uint8_t nvm_addr = (rowAddr & 0x3Fu);
//   if (!writeReg(REG_NVM_ADDR, nvm_addr)) return false;

//   /** CMD = USR_READ sequence (0x5D, 0xA5) */
//   if (!writeCMDSequence(0x5D, 0xA5)) return false;

//   /** Wait until ready again */
//   if (!waitNVMReady(20000)) return false;

//   /** Check status error flags */
//   auto st = readNVMStatus();
//   if (st.err || st.cmd_err) return false;

//   /** Read data regs */
//   uint8_t l = 0, h = 0;
//   if (!readReg(REG_NVM_DATA_LSB, l)) return false;
//   if (!readReg(REG_NVM_DATA_MSB, h)) return false;

//   out = (uint16_t(h) << 8) | uint16_t(l);
//   return true;
// }

// bool BMP585_7Semi::writeNVMUserRow(uint8_t rowAddr, uint16_t value) {
//   /** User range is rows 0x20..0x22 (2 bytes each) */
//   if (rowAddr < 0x20 || rowAddr > 0x22) return false;

//   /** Switch to STANDBY and disable deep standby */
//   if (!setPowerMode(PowerMode::Standby)) return false;

//   uint8_t odr = 0;
//   if (!readReg(REG_ODR_CONFIG, odr)) return false;
//   odr |= (1u << 7);
//   if (!writeReg(REG_ODR_CONFIG, odr)) return false;

//   /** Wait for NVM ready */
//   if (!waitNVMReady(20000)) return false;

//   /** Set NVM_ADDR with prog_en=1 and row address */
//   uint8_t nvm_addr = (rowAddr & 0x3Fu) | (1u << 6);
//   if (!writeReg(REG_NVM_ADDR, nvm_addr)) return false;

//   /** Write data regs */
//   uint8_t l = uint8_t(value & 0xFFu);
//   uint8_t h = uint8_t((value >> 8) & 0xFFu);
//   if (!writeReg(REG_NVM_DATA_LSB, l)) return false;
//   if (!writeReg(REG_NVM_DATA_MSB, h)) return false;

//   /** CMD = USR_PROG sequence (0x5D, 0xA0) */
//   if (!writeCMDSequence(0x5D, 0xA0)) return false;

//   /** Wait until ready (~10ms typical) */
//   if (!waitNVMReady(50000)) return false;

//   /** Check status error flags */
//   auto st = readNVMStatus();
//   if (st.err || st.cmd_err) return false;

//   /** Reset prog_en to 0 */
//   uint8_t nvm_addr_clear = (rowAddr & 0x3Fu);
//   if (!writeReg(REG_NVM_ADDR, nvm_addr_clear)) return false;

//   return true;
// }


// bool BMP585_7Semi::configureInterruptPin(bool latched, bool activeHigh, bool openDrain, bool enable) {
//   /**
//    * INT_CONFIG (0x14)
//    * - bit0 int_mode: 0 pulsed, 1 latched
//    * - bit1 int_pol:  0 active low, 1 active high
//    * - bit2 int_od:   0 push-pull, 1 open-drain
//    * - bit3 int_en:   0 disabled, 1 enabled
//    */
//   uint8_t v = 0;
//   if (latched)    v |= (1u << 0);
//   if (activeHigh) v |= (1u << 1);
//   if (openDrain)  v |= (1u << 2);
//   if (enable)     v |= (1u << 3);
//   return writeReg(REG_INT_CONFIG, v);
// }

// bool BMP585_7Semi::setInterruptSources(bool drdy, bool fifo_full, bool fifo_ths, bool oor_p) {
//   /**
//    * INT_SOURCE (0x15)
//    * - bit0 drdy_data_reg_en
//    * - bit1 fifo_full_en
//    * - bit2 fifo_ths_en
//    * - bit3 oor_p_en
//    */
//   uint8_t v = 0;
//   if (drdy)      v |= (1u << 0);
//   if (fifo_full) v |= (1u << 1);
//   if (fifo_ths)  v |= (1u << 2);
//   if (oor_p)     v |= (1u << 3);
//   return writeReg(REG_INT_SOURCE, v);
// }

// BMP585_7Semi::InterruptStatus BMP585_7Semi::readInterruptStatus() {
//   InterruptStatus st;
//   uint8_t v = 0;
//   if (!readReg(REG_INT_STATUS, v)) return st;

//   st.drdy_data_reg = (v & (1u << 0)) != 0;
//   st.fifo_full     = (v & (1u << 1)) != 0;
//   st.fifo_ths      = (v & (1u << 2)) != 0;
//   st.oor_p         = (v & (1u << 3)) != 0;
//   st.por           = (v & (1u << 4)) != 0;
//   return st;
// }


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
