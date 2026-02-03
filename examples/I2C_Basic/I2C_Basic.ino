/*  I2C_Basic.ino  —  7Semi BMP585 example over I²C
 *
 *  What this does
 *  - Starts BMP585 over I²C (default 0x46, 400 kHz)
 *  - In loop(), reads temperature + pressure in one burst
 *  - Prints values every 250 ms
 *
 *  Wiring (typical 3V3 MCU)
 *    VDD      -> 3V3
 *    GND      -> GND
 *    SDA(SDI) -> MCU SDA
 *    SCL(SCK) -> MCU SCL
 *    SDO -> LOW  (addr 0x46)  or HIGH (addr 0x47)
 */
#include <7Semi_BMP585.h>


// ---------- I2C settings & device ----------
BMP585_7Semi bmp;

/** Default BMP585 I2C address is 0x46 (0x47 if SDO is HIGH) */
static const uint8_t BMP_ADDR = 0x47;

/** Optional (ESP32 only): set pins, else keep -1 */
int SDA_PIN = -1;
int SCL_PIN = -1;
// int SDA_PIN = 21;
// int SCL_PIN = 22;

static uint32_t t = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait for USB (if applicable) */
  }
  Serial.println(F("\n[7Semi BMP585 — I2C_Basic]"));

  if (!bmp.beginI2C(Wire, BMP_ADDR, 400000, SDA_PIN, SCL_PIN)) {
    Serial.println(" BMP585 initialization failed (I2C)");
    while (1) delay(1000);
  }

  /** Reset + configure */
  bmp.reset();

  /** 
   * OSR: temperature + pressure oversampling
   * - pressEnable = true enables pressure measurement
   */
  bmp.setOSR(OSR::OSR_8, OSR::OSR_8, true);

  /** 
   * ODR + power mode
   * - Normal mode keeps periodic measurements running
   * - deepStandbyDisable=false is fine for normal reads
   */
  bmp.setODR(ODR::ODR_10_Hz, PowerMode::Normal);

  Serial.println("Configured: OSR x8/x8, ODR 10Hz, Normal mode");
  delay(100);
}

void loop() {
  float tempC = NAN;
  float pressPa = NAN;
  float altitude;
  if (millis() - t > 250) {
    t = millis();

    if (bmp.readTemperaturePressure(tempC, pressPa)) {
      Serial.print("T = ");
      Serial.print(tempC, 2);
      Serial.print(" C,  P = ");
      Serial.print(pressPa, 2);
      Serial.print(" Pa");
      if (bmp.readAltitudeM(1013.25f, altitude)) {
        Serial.print("  Alt = ");
        Serial.print(altitude, 2);
        Serial.println(" m");
      }
    } else {
      Serial.println("Read failed");
    }

    Serial.print('\n');
  }
}
