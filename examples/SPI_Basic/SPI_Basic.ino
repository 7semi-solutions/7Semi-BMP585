/*  SPI_Basic.ino  —  7Semi BMP585 example over SPI
 *
 *  What this does
 *  - Starts BMP585 over SPI (CS pin + SPISettings)
 *  - In loop(), reads temperature + pressure in one burst
 *  - Prints values every 250 ms
 *
 *  Wiring (typical 3V3 MCU)
 *    VDD       -> 3V3
 *    GND       -> GND
 *    SCK       -> MCU SCK
 *    MOSI(SDI) -> MCU MOSI
 *    MISO(SDO) -> MCU MISO
 *    CS        -> MCU GPIO (define below)
 */

#include <7Semi_BMP585.h>
// ---------- SPI settings & device ----------
static const uint8_t BMP_CS = 10;  // Change to your board's CS pin
static SPISettings bmpSPI(1000000, MSBFIRST, SPI_MODE0);

BMP585_7Semi bmp;

static uint32_t t = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println(F("\n[7Semi BMP585 — SPI_Basic]"));

  if (!bmp.beginSPI(SPI, BMP_CS, bmpSPI)) {
    Serial.println(" BMP585 initialization failed (SPI)");
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
        Serial.print("  Altitude = ");
        Serial.print(altitude, 2);
        Serial.println(" m");
      }
    } else {
      Serial.println("Read failed");
    }

    Serial.print('\n');
  }
}
