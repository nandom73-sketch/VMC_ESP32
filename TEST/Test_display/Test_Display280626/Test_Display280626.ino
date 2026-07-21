/******************************************************************************
 * Test_Display_280626
 *
 * STATUS : WORKING
 *
 * PURPOSE
 * Verifica del display ILI9341:
 *  - Hardware SPI
 *  - Reset hardware
 *  - Test colori RGB
 *
 * RESULT
 * ✓ Display funzionante
 *
 ******************************************************************************/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// ---------------------------
// Pin ESP32
// ---------------------------

constexpr uint8_t TFT_CS   = 5;
constexpr uint8_t TFT_DC   = 4;
constexpr uint8_t TFT_RST  = 2;

constexpr uint8_t TFT_MOSI = 23;
constexpr uint8_t TFT_SCK  = 18;

// ---------------------------

Adafruit_ILI9341 display(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=================================");
    Serial.println("ILI9341 DISPLAY TEST");
    Serial.println("=================================");

    // Reset hardware come da produttore

    pinMode(TFT_RST, OUTPUT);

    digitalWrite(TFT_RST, HIGH);
    delay(5);

    digitalWrite(TFT_RST, LOW);
    delay(15);

    digitalWrite(TFT_RST, HIGH);
    delay(15);

    // SPI ESP32

    SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);

    Serial.println("Init display...");

    display.begin();

    display.setRotation(1);

    Serial.println("Display initialized.");
}

void loop()
{
    Serial.println("RED");
    display.fillScreen(ILI9341_RED);
    delay(2000);

    Serial.println("GREEN");
    display.fillScreen(ILI9341_GREEN);
    delay(2000);

    Serial.println("BLUE");
    display.fillScreen(ILI9341_BLUE);
    delay(2000);

    Serial.println("BLACK");
    display.fillScreen(ILI9341_BLACK);
    delay(2000);
}