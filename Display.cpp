/******************************************************************************
 * Display Module
 *
 * STATUS  : DEVELOPMENT
 * VERSION : 0.1.4
 *
 ******************************************************************************/

#include "Display.h"
#include "Config.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

//=============================================================================

static Adafruit_ILI9341 display(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

//=============================================================================

bool Display_begin()
{
    pinMode(PIN_TFT_RST, OUTPUT);

    digitalWrite(PIN_TFT_RST, HIGH);
    delay(5);

    digitalWrite(PIN_TFT_RST, LOW);
    delay(15);

    digitalWrite(PIN_TFT_RST, HIGH);
    delay(15);

    SPI.begin(PIN_TFT_SCK, PIN_TFT_MISO, PIN_TFT_MOSI, PIN_TFT_CS);

    display.begin();

    display.setRotation(1);

    display.fillScreen(ILI9341_BLACK);

    return true;
}

//=============================================================================

void Display_clear()
{
    display.fillScreen(ILI9341_BLACK);
}

//=============================================================================

void Display_showSplash()
{
    display.fillScreen(ILI9341_BLACK);

    display.setTextColor(ILI9341_WHITE);
    display.setTextSize(3);

    display.setCursor(40, 40);
    display.println("VMC");

    display.setTextSize(2);

    display.setCursor(40, 90);
    display.println("Firmware");

    display.setCursor(40, 120);
    display.print("v");
    display.println(FW_VERSION);

    delay(2000);
}

//=============================================================================

void Display_showHome(const SensorData& climate,
                      const FanData& fans)
{
    display.fillScreen(ILI9341_BLACK);

    display.setTextColor(ILI9341_WHITE);
    display.setTextSize(2);

    display.setCursor(10,10);
    display.println("VMC HOME");

    display.setCursor(10,45);
    display.print("INT ");
    display.print(climate.intTemp,1);
    display.println(" C");

    display.setCursor(10,65);
    display.print("HUM ");
    display.print(climate.intHum,1);
    display.println(" %");

    display.setCursor(10,105);
    display.print("EXT ");
    display.print(climate.extTemp,1);
    display.println(" C");

    display.setCursor(10,125);
    display.print("HUM ");
    display.print(climate.extHum,1);
    display.println(" %");

    display.setCursor(10,165);
    display.print("PRES ");
    display.print(climate.pressure,1);
    display.println(" hPa");

    display.setCursor(10,200);
    display.print("IN  ");
    display.print(fans.inPercent);
    display.println(" %");

    display.setCursor(10,220);
    display.print("OUT ");
    display.print(fans.outPercent);
    display.println(" %");
}