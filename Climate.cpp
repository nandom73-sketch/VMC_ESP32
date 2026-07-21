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

namespace
{
constexpr float PA_TO_HPA = 100.0f;

Adafruit_SHT31 sht31;
Adafruit_BME280 bme;

bool sht31OK = false;
bool bmeOK = false;

void printResult(const bool ok)
{
    Serial.print(F("Result: "));
    Serial.println(ok ? F("OK") : F("FAIL"));
}

bool beginBME280()
{
    Serial.print(F("Trying BME280 @0x"));
    Serial.println(0x76, HEX);

    const bool ok = bme.begin(0x76, &Wire);
    printResult(ok);

    return ok;
}

bool beginSHT31()
{
    Serial.println(F("Trying SHT31"));

    const bool ok = sht31.begin(SHT31_ADDRESS);
    printResult(ok);

    return ok;
}

void readSHT31(SensorData &data)
{
    data.intTemp = sht31.readTemperature();
    data.intHum = sht31.readHumidity();
}

void readBME280(SensorData &data)
{
    data.extTemp = bme.readTemperature();
    data.extHum = bme.readHumidity();
    data.pressure = bme.readPressure() / PA_TO_HPA;
}
} // namespace

bool Climate_begin()
{
    Serial.println();
    Serial.println(F("===== Climate Begin ====="));

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    Serial.println(F("Wire.begin() OK"));

    // ---------------------------
    // Test BME280
    // ---------------------------

    bmeOK = beginBME280();

    // ---------------------------
    // Test SHT31
    // ---------------------------

    sht31OK = beginSHT31();

    Serial.println(F("========================="));

    return (sht31OK && bmeOK);
}

bool Climate_read(SensorData &data)
{
    if (sht31OK)
    {
        readSHT31(data);
    }

    if (bmeOK)
    {
        readBME280(data);
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
