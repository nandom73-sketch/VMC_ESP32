#include <Wire.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21,22);

  Serial.println("Test BME280");

  if (bme.begin(0x76))
  {
    Serial.println("BME280 OK");

    Serial.print("Temp: ");
    Serial.println(bme.readTemperature());

    Serial.print("Hum: ");
    Serial.println(bme.readHumidity());

    Serial.print("Press: ");
    Serial.println(bme.readPressure()/100.0);
  }
  else
  {
    Serial.println("BME280 FAILED");
  }
}

void loop()
{
}