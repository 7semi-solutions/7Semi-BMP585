#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

/* =========================
 * BMP585 register map
 * ========================= */

#define REG_CHIP_ID        0x01
#define REG_REV_ID         0x02

#define REG_CHIP_STATUS    0x11

#define REG_DRIVE_CONFIG   0x13
#define REG_INT_CONFIG     0x14
#define REG_INT_SOURCE     0x15
#define REG_FIFO_CONFIG    0x16
#define REG_FIFO_COUNT     0x17
#define REG_FIFO_SEL       0x18

#define REG_TEMP_XLSB      0x1D /* TEMP_LSB = 0x1E, TEMP_MSB = 0x1F */
#define REG_PRESS_XLSB     0x20 /* PRESS_LSB = 0x21, PRESS_MSB = 0x22 */

#define REG_INT_STATUS     0x27
#define REG_STATUS         0x28
#define REG_FIFO_DATA      0x29

#define REG_NVM_ADDR       0x2B
#define REG_NVM_DATA_LSB   0x2C
#define REG_NVM_DATA_MSB   0x2D

#define REG_DSP_CONFIG     0x30
#define REG_DSP_IIR        0x31

#define REG_OOR_THR_LSB    0x32
#define REG_OOR_THR_MSB    0x33
#define REG_OOR_RANGE      0x34
#define REG_OOR_CONFIG     0x35

#define REG_OSR_CONFIG     0x36
#define REG_ODR_CONFIG     0x37
#define REG_OSR_EFF        0x38

#define REG_CMD            0x7E

#define CHIP_ID_EXPECTED   0x51

enum class Interface : uint8_t {
    I2C = 0,
    SPI = 1
  };

  enum class PowerMode : uint8_t {
    Standby = 0x0,   /* no measurement ongoing */
    Normal  = 0x1,   /* measurement in configured ODR grid */
    Forced  = 0x2,   /* forced one-time measurement */
    NonStop = 0x3    /* repetitive measurements without further duty-cycling */
  };

  /** 
   * Oversampling encoding for OSR_CONFIG.osr_t and OSR_CONFIG.osr_p
   * - 0..7 => 1x..128x
   */
  enum class OSR : uint8_t {
    OSR_1   = 0x0,
    OSR_2   = 0x1,
    OSR_4   = 0x2,
    OSR_8   = 0x3,
    OSR_16  = 0x4,
    OSR_32  = 0x5,
    OSR_64  = 0x6,
    OSR_128 = 0x7
  };

  /** 
   * ODR encoding for ODR_CONFIG.odr (bit offset 2)
   * - Values 0x00..0x1F map to 240 Hz down to 0.125 Hz
   */
    enum class ODR : uint8_t {
    ODR_240_Hz     = 0x00,
    ODR_218_537_Hz = 0x01,
    ODR_199_111_Hz = 0x02,
    ODR_179_200_Hz = 0x03,
    ODR_160_Hz     = 0x04,
    ODR_149_333_Hz = 0x05,
    ODR_140_Hz     = 0x06,
    ODR_129_855_Hz = 0x07,
    ODR_120_Hz     = 0x08,
    ODR_110_164_Hz = 0x09,
    ODR_100_299_Hz = 0x0A,
    ODR_89_600_Hz   = 0x0B,
    ODR_80_Hz      = 0x0C,
    ODR_70_Hz      = 0x0D,
    ODR_60_Hz      = 0x0E,
    ODR_50_056_Hz  = 0x0F,
    ODR_45_025_Hz  = 0x10,
    ODR_40_Hz      = 0x11,
    ODR_35_Hz      = 0x12,
    ODR_30_Hz      = 0x13,
    ODR_25_005_Hz  = 0x14,
    ODR_20_Hz      = 0x15,
    ODR_15_Hz      = 0x16,
    ODR_10_Hz      = 0x17,
    ODR_5_Hz       = 0x18,
    ODR_4_Hz       = 0x19,
    ODR_3_Hz       = 0x1A,
    ODR_2_Hz       = 0x1B,
    ODR_1_Hz       = 0x1C,
    ODR_0_5_Hz     = 0x1D,
    ODR_0_25_Hz    = 0x1E,
    ODR_0_125_Hz   = 0x1F
  };

  enum class IIRCoeff : uint8_t {
    OFF  = 0x0,
    IIRCoeff_1   = 0x1,
    IIRCoeff_3   = 0x2,
    IIRCoeff_7   = 0x3,
    IIRCoeff_15  = 0x4,
    IIRCoeff_31  = 0x5,
    IIRCoeff_63  = 0x6,
    IIRCoeff_127 = 0x7
  };

  enum class FIFOFrame : uint8_t {
    Disabled  = 0x0,
    TempOnly  = 0x1,
    PressOnly = 0x2,
    PressTemp = 0x3
  };

  enum class FIFOMode : uint8_t {
    StreamToFIFO = 0x0,
    StopOnFull   = 0x1
  };

  struct InterruptStatus {
    bool drdy_data_reg = false;
    bool fifo_full     = false;
    bool fifo_ths      = false;
    bool oor_p         = false;
    bool por           = false;
  };

  struct FIFORead {
    bool valid = false;
    bool empty = false;
    FIFOFrame frame = FIFOFrame::Disabled;
    float temperature_c = NAN;
    float pressure_pa   = NAN;
  };

  /** 
   * NVM status from STATUS (0x28)
   * - rdy: STATUS.status_nvm_rdy
   * - err: STATUS.status_nvm_err
   * - cmd_err: STATUS.status_nvm_cmd_err
   */
  struct NVMStatus {
    bool rdy = false;
    bool err = false;
    bool cmd_err = false;
  };
/** 
 * 7Semi BMP585 register-level driver (no Bosch SensorAPI)
 * - Supports I2C and SPI (Arduino-core boards)
 * - Includes: OSR/ODR/IIR, FIFO, interrupts, OOR window, reset, NVM access
 * - Uses burst reads for consistent P/T snapshots
 *
 * Notes:
 * - Works on ESP32 / AVR / STM32 / RP2040 (Arduino cores)
 * - Raspberry Pi Linux requires a different backend (spidev), not Arduino SPI/Wire
 */
class BMP585_7Semi {


public:
  BMP585_7Semi();

  /** 
   * Initialize over I2C
   * - i2cAddr default is 0x46 (0x47 if SDO is HIGH)
   * - Wire is used as provided
   * - On ESP32, optional SDA/SCL can be provided
   *
   * Pin rules:
   * - Use -1 to mean "use board default pins"
   * - For non-ESP32 cores, pins are ignored
   */
  bool beginI2C(TwoWire &wire,
                uint8_t i2cAddr,
                uint32_t i2cSpeed,
                uint8_t i2cSDA = -1,
                uint8_t i2cSCL = -1);

  /** 
   * Initialize over SPI
   * - Uses standard Arduino SPIClass + a CS pin
   * - On ESP32, optional SCK/MISO/MOSI can be provided
   *
   * Notes:
   * - On many cores, SPI pins are fixed by the board / SPI instance
   * - After reset(), BMP585 may switch back to I2C/I3C, so SPI requires a dummy read
   */
  bool beginSPI(SPIClass &spiBus,
                uint8_t csPin,
                SPISettings settings,
                int sck = -1,
                int miso = -1,
                int mosi = -1);

  /** 
   * Apply safe default configuration
   * - Makes the sensor work even if the user never sets OSR/ODR/IIR
   * - beginI2C()/beginSPI() call this automatically
   */
  bool initSensor();

  /** - Soft reset (CMD=0xB6) */
  bool reset();

  /** - Read and validate chip id (ASIC ID register 0x01) */
  bool chipID(uint8_t &chip_id);

  /** - Read ASIC revision id (0x02) */
  bool revisionID(uint8_t &rev_id);

/** 
 * Configure oversampling (OSR_CONFIG 0x36)
 * - Controls noise vs response time for temperature and pressure
 * - Higher OSR = smoother data, slower update
 *
 * Options (OSR):
 * - OSR::OSR_1   (1x)
 * - OSR::OSR_2   (2x)
 * - OSR::OSR_4   (4x)
 * - OSR::OSR_8   (8x)   (good default)
 * - OSR::OSR_16  (16x)
 * - OSR::OSR_32  (32x)
 * - OSR::OSR_64  (64x)
 * - OSR::OSR_128 (128x)
 *
 * Parameters:
 * - osr_t: temperature oversampling
 * - osr_p: pressure oversampling
 * - pressEnable:
 *   - true  = temperature + pressure enabled
 *   - false = temperature only (pressure output may be 0 / stale)
 *
 * Return:
 * - true on success, false if register write fails
 */
bool setOSR(OSR osr_t, OSR osr_p, bool pressEnable = true);

/** 
 * Configure output data rate and power mode (ODR_CONFIG 0x37)
 * - ODR sets measurement schedule
 * - Power mode controls how measurements run
 *
 * Options (PowerMode):
 * - PowerMode::Standby  (no measurement running)
 * - PowerMode::Normal   (runs at selected ODR)
 * - PowerMode::Forced   (one-shot measurement)
 * - PowerMode::NonStop  (continuous repetitive measurements)
 *
 * Options (ODR):
 * - ODR::ODR_240_Hz
 * - ODR::ODR_160_Hz
 * - ODR::ODR_80_Hz
 * - ODR::ODR_50_056_Hz
 * - ODR::ODR_20_Hz
 * - ODR::ODR_10_Hz   (good default)
 * - ODR::ODR_5_Hz
 * - ODR::ODR_2_Hz
 * - ODR::ODR_1_Hz
 * - ODR::ODR_0_5_Hz
 * - ODR::ODR_0_125_Hz
 *
 * Parameters:
 * - odr: output data rate
 * - mode: power mode
 * - deepStandbyDisable:
 *   - false = device may enter deep standby (lower power)
 *   - true  = disables deep standby (stable continuous reads for beginners)
 *
 * Return:
 * - true on success, false if register write fails
 */
bool setODR(ODR odr, PowerMode mode, bool deepStandbyDisable = false);

/** 
 * Change only the power mode field (ODR_CONFIG.pwr_mode)
 * - Keeps ODR and deep-standby settings unchanged
 *
 * Options:
 * - PowerMode::Standby
 * - PowerMode::Normal
 * - PowerMode::Forced
 * - PowerMode::NonStop
 *
 * Return:
 * - true on success, false if read/write fails
 */
bool setPowerMode(PowerMode mode);


/** 
 * Configure IIR filtering (DSP_IIR 0x31)
 * - Reduces noise by smoothing temperature/pressure
 * - Higher coefficient = stronger filtering, slower response
 *
 * Datasheet behavior:
 * - IIR must be changed in STANDBY
 * - Library switches to STANDBY internally and restores mode
 *
 * Options (IIRCoeff):
 * - IIRCoeff::OFF          (no filtering)
 * - IIRCoeff::IIRCoeff_1
 * - IIRCoeff::IIRCoeff_3
 * - IIRCoeff::IIRCoeff_7
 * - IIRCoeff::IIRCoeff_15
 * - IIRCoeff::IIRCoeff_31
 * - IIRCoeff::IIRCoeff_63
 * - IIRCoeff::IIRCoeff_127 (maximum smoothing)
 *
 * Recommended:
 * - OFF for fastest response
 * - IIRCoeff_7 or IIRCoeff_15 for stable UI/altitude
 *
 * Return:
 * - true on success, false if register access fails
 */
bool setIIR(IIRCoeff iir_t, IIRCoeff iir_p);


  /** - Read temperature in °C */
  bool readTemperatureC(float &tempC);

  /** - Read pressure in Pa */
  bool readPressurePa(float &pressPa);

  /** 
   * Read both temperature and pressure in one burst
   * - Keeps values consistent inside one measurement cycle
   */
  bool readTemperaturePressure(float &tempC, float &pressPa);

  /** 
   * Altitude estimate (meters) using sea level pressure (hPa)
   * - Uses standard barometric formula approximation
   */
  bool readAltitudeM(float seaLevel_hPa, float &altitude_m);

/** 
 * Configure FIFO (FIFO_CONFIG 0x16, FIFO_SEL 0x18)
 * - Allows buffered samples for burst reads
 * - Useful when MCU sleeps or reads slowly
 *
 * Options (FIFOFrame):
 * - FIFOFrame::Disabled
 * - FIFOFrame::TempOnly
 * - FIFOFrame::PressOnly
 * - FIFOFrame::PressTemp
 *
 * Options (FIFOMode):
 * - FIFOMode::StreamToFIFO (keeps overwriting old samples)
 * - FIFOMode::StopOnFull   (stops when full)
 *
 * Parameters:
 * - frameSel:
 *   - TempOnly = 3 bytes per frame
 *   - PressOnly = 3 bytes per frame
 *   - PressTemp = 6 bytes per frame (recommended if you want both)
 * - decimationPow2:
 *   - 0..7 means store every 2^n sample
 *   - 0 = store every sample, 1 = every 2nd, 2 = every 4th, ...
 * - thresholdFrames:
 *   - 0 disables threshold interrupt
 *   - 1..31 sets FIFO threshold level (device-dependent limits apply)
 * - mode:
 *   - stream or stop-on-full
 *
 * Datasheet requirement:
 * - FIFO_SEL should be changed in STANDBY mode
 *
 * Return:
 * - true on success, false if register write fails
 */
bool configureFIFO(FIFOFrame frameSel, uint8_t decimationPow2, uint8_t thresholdFrames, FIFOMode mode);


  /** - FIFO frame count (FIFO_COUNT.fifo_count) */
  uint8_t getFIFOCount();

  /** 
   * Read one FIFO frame
   * - PT frame: 6 bytes (T3 + P3)
   * - T/P frame: 3 bytes
   * - Empty frame returns 0x7F
   */
  FIFORead readFIFOFrame();

  /** 
   * Best-effort FIFO clear
   * - Switches FIFO frame selection to Disabled and back (causes flush)
   */
  bool clearFIFO();

/** 
 * Configure interrupt output pin (INT_CONFIG 0x14)
 *
 * Parameters:
 * - latched:
 *   - false = pulsed interrupt
 *   - true  = latched until INT_STATUS is read
 * - activeHigh:
 *   - false = active low
 *   - true  = active high
 * - openDrain:
 *   - false = push-pull
 *   - true  = open-drain (needs pull-up)
 * - enable:
 *   - false = INT pin disabled
 *   - true  = INT pin enabled
 *
 * Return:
 * - true on success, false if register write fails
 */
bool configureInterruptPin(bool latched, bool activeHigh, bool openDrain, bool enable);

/** 
 * Select interrupt sources (INT_SOURCE 0x15)
 *
 * Parameters:
 * - drdy:
 *   - true  = data-ready triggers interrupt
 * - fifo_full:
 *   - true  = FIFO full triggers interrupt
 * - fifo_ths:
 *   - true  = FIFO threshold triggers interrupt
 * - oor_p:
 *   - true  = out-of-range pressure window triggers interrupt
 *
 * Return:
 * - true on success, false if register write fails
 */
bool setInterruptSources(bool drdy, bool fifo_full, bool fifo_ths, bool oor_p);

/** 
 * Read interrupt status flags (INT_STATUS 0x27)
 * - Bits are clear-on-read
 * - Reading this register clears the event flags
 *
 * Return:
 * - InterruptStatus struct containing flags
 */
InterruptStatus readInterruptStatus();


/** 
 * Configure out-of-range pressure window (OOR)
 * - Compares measured pressure to a reference window
 * - Can trigger interrupt when pressure is outside range
 *
 * Parameters:
 * - refPressurePa:
 *   - reference pressure in Pa (17-bit internally)
 * - rangePa:
 *   - +/- window range in Pa (8-bit value: 0..255 Pa)
 *   - Example: rangePa=50 means +/-50 Pa window
 * - countLimitCode:
 *   - debounce / persistence control (device interpretation)
 *   - common pattern is:
 *     - 0x0 => 1 sample
 *     - 0x3 => 3 samples
 *     - 0x7 => 7 samples
 *     - 0xF => 15 samples
 *
 * Return:
 * - true on success, false if register write fails
 */
bool setOORWindow(uint32_t refPressurePa, uint8_t rangePa, uint8_t countLimitCode);


  /** - Read NVM status flags */
  NVMStatus readNVMStatus();

  /** - Read user NVM row (0x20..0x22) */
  bool readNVMUserRow(uint8_t rowAddr, uint16_t &out);

  /** - Program user NVM row (0x20..0x22) */
  bool writeNVMUserRow(uint8_t rowAddr, uint16_t value);

private:
  Interface bus = Interface::I2C;

  TwoWire *i2c = nullptr;
  uint8_t address = 0x46;

  SPIClass *spi = nullptr;
  SPISettings spiSettings = SPISettings(1000000, MSBFIRST, SPI_MODE0);
  int8_t cs_pin = -1;

private:

  bool writeReg(uint8_t reg, uint8_t value);
  bool readReg(uint8_t reg, uint8_t &value);
  bool readNReg(uint8_t reg, uint8_t *buf, size_t len);

  bool setBits(uint8_t reg, uint8_t mask, uint8_t valueShifted);

  bool waitNVMReady(uint32_t timeout_us);
  bool writeCMDSequence(uint8_t b0, uint8_t b1);

  static int32_t signExtend24(uint32_t x);
  static float tempFromRaw(int32_t raw24);
  static float pressFromRaw(int32_t raw24);
};
