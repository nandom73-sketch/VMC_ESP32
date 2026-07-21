/******************************************************************************
 * Climate Module
 *
 * STATUS:
 * ✓ STABLE
 *
 * NOTE:
 * Per il BME280 usare SEMPRE:
 *
 *     bme.begin(0x76, &Wire);
 *
 ******************************************************************************/

#include "Climate.h"
#include "Config.h"

#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_BME280.h>

static Adafruit_SHT31 sht31;
static Adafruit_BME280 bme;

static bool sht31OK = false;
static bool bmeOK = false;

bool Climate_begin()
{
    Serial.println();
    Serial.println("===== Climate Begin =====");

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    Serial.println("Wire.begin() OK");

    // ---------------------------
    // Test BME280
    // ---------------------------

    Serial.println("Trying BME280 @0x76");

    bmeOK = bme.begin(0x76, &Wire);

    Serial.print("Result: ");
    Serial.println(bmeOK ? "OK" : "FAIL");

    if (!bmeOK)
    {
        Serial.println("Trying BME280 @0x77");

        bmeOK = bme.begin(0x77, &Wire);

        Serial.print("Result: ");
        Serial.println(bmeOK ? "OK" : "FAIL");
    }

    // ---------------------------
    // Test SHT31
    // ---------------------------

    Serial.println("Trying SHT31");

    sht31OK = sht31.begin(0x44);

    Serial.print("Result: ");
    Serial.println(sht31OK ? "OK" : "FAIL");

    Serial.println("=========================");

    return (sht31OK && bmeOK);
}

bool Climate_read(SensorData &data)
{
    if (sht31OK)
    {
        data.intTemp = sht31.readTemperature();
        data.intHum  = sht31.readHumidity();
    }

    if (bmeOK)
    {
        data.extTemp  = bme.readTemperature();
        data.extHum   = bme.readHumidity();
        data.pressure = bme.readPressure() / 100.0f;
    }

    return true;
}

bool Climate_isSHT31_OK()
{
    return sht31OK;
}

bool Climate_isBME280_OK()
{
    return bmeOK;
}