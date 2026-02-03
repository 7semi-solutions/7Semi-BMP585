/*  Config_Example.ino  —  7Semi BMP585 custom configuration example
 *
 *  What this does
 *  - Starts BMP585 (I2C in this example)
 *  - Overrides the default configuration after begin()
 *  - Prints temperature + pressure + altitude every 250 ms
 *
 *  Wiring (typical 3V3 MCU)
 *    VDD      -> 3V3
 *    GND      -> GND
 *    SDA(SDI) -> MCU SDA
 *    SCL(SCK) -> MCU SCL
 *    SDO -> LOW  (addr 0x46)  or HIGH (addr 0x47)
 *
 *  Configuration options used here
 *  - OSR (oversampling):
 *    - OSR::OSR_1, OSR_2, OSR_4, OSR_8, OSR_16, OSR_32, OSR_64, OSR_128
 *  - IIR (filter):
 *    - IIRCoeff::OFF, IIRCoeff_1, IIRCoeff_3, IIRCoeff_7, IIRCoeff_15,
 *      IIRCoeff_31, IIRCoeff_63, IIRCoeff_127
 *  - ODR (data rate):
 *    - ODR::ODR_240_Hz ... ODR::ODR_0_125_Hz
 *    - Common: ODR_20_Hz, ODR_10_Hz, ODR_5_Hz, ODR_1_Hz
 *  - Power mode:
 *    - PowerMode::Standby, Normal, Forced, NonStop
 *
 *  Notes
 *  - beginSPI() already applies defaults (initSensor())
 *  - This example shows how to override those defaults safely
 *  - For stable altitude, use moderate OSR + some IIR filtering
 */

#include <7Semi_BMP585.h>


// ---------- I2C settings & device ----------
/** Default BMP585 I2C address is 0x46 (0x47 if SDO is HIGH) */
static const uint8_t BMP_ADDR = 0x47;

/** Optional (ESP32 only): set pins, else keep -1 */
int SDA_PIN = -1;
int SCL_PIN = -1;
// int SDA_PIN = 21;
// int SCL_PIN = 22;
BMP585_7Semi bmp;

/** Sea level reference pressure (hPa) */
static const float SEA_LEVEL_HPA = 1013.25f;

static uint32_t t = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait for USB (if applicable) */
  }
  Serial.println(F("\n[7Semi BMP585 — Config_Example]"));

 if (!bmp.beginI2C(Wire, BMP_ADDR, 400000, SDA_PIN, SCL_PIN)) {
    Serial.println(" BMP585 initialization failed (SPI)");
    while (1) delay(1000);
  }

  /** 
   * Custom configuration (override defaults)
   * - Set OSR higher for smoother readings
   * - Enable moderate IIR filtering for stable pressure/altitude
   * - Set ODR and mode for periodic updates
   */
  if (!bmp.setOSR(OSR::OSR_16, OSR::OSR_16, true)) {
    Serial.println(" setOSR failed");
  }

  if (!bmp.setIIR(IIRCoeff::IIRCoeff_15, IIRCoeff::IIRCoeff_15)) {
    Serial.println(" setIIR failed");
  }

  /** 
   * deepStandbyDisable = true
   * - Keeps output stable for continuous serial printing
   */
  if (!bmp.setODR(ODR::ODR_10_Hz, PowerMode::Normal, true)) {
    Serial.println(" setODR failed");
  }

  Serial.println(" Custom config applied");
}

void loop() {
  float tempC = NAN;
  float pressPa = NAN;
  float altM = NAN;

  if (millis() - t > 250) {
    t = millis();

    if (bmp.readTemperaturePressure(tempC, pressPa) && bmp.readAltitudeM(SEA_LEVEL_HPA, altM)) {
      Serial.print("T = ");
      Serial.print(tempC, 2);
      Serial.print(" C,  P = ");
      Serial.print(pressPa, 2);
      Serial.print(" Pa,  Alt = ");
      Serial.print(altM, 2);
      Serial.println(" m");
    } else {
      Serial.println("Read failed");
    }

    Serial.print('\n');
  }
}
