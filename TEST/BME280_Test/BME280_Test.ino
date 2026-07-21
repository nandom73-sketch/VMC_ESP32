#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);

  Serial.println();
  Serial.println("================================");
  Serial.println("BME280 CHIP ID TEST");
  Serial.println("================================");

  Wire.beginTransmission(0x76);

  if (Wire.endTransmission() != 0)
  {
    Serial.println("Device 0x76 not found!");
    return;
  }

  Wire.beginTransmission(0x76);
  Wire.write(0xD0);          // Registro CHIP ID
  Wire.endTransmission(false);

  Wire.requestFrom(0x76, 1);

  if (Wire.available())
  {
    byte id = Wire.read();

    Serial.print("Chip ID = 0x");

    if (id < 16)
      Serial.print("0");

    Serial.println(id, HEX);

    switch (id)
    {
      case 0x60:
        Serial.println("==> BME280");
        break;

      case 0x58:
        Serial.println("==> BMP280");
        break;

      default:
        Serial.println("==> Unknown device");
        break;
    }
  }
  else
  {
    Serial.println("Unable to read CHIP ID");
  }
}

void loop()
{
}