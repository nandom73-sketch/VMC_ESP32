/******************************************************************************
 * Display Module
 *
 * STATUS  : DEVELOPMENT
 * VERSION : 0.1.5 - Ruotato display
 *
 ******************************************************************************/

#include "Display.h"
#include "Config.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <math.h>
#include "VMC_UI_v1.h"

//=============================================================================

static Adafruit_ILI9341 display(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

//=============================================================================

static constexpr int16_t HOME_LABEL_X = 10;
static constexpr int16_t HOME_VALUE_X = 70;
static constexpr int16_t HOME_LINE_H = 20;
static constexpr uint8_t HOME_INVALID_PERCENT = 255;

struct HomeDisplayCache
{
    float intTemp;
    float intHum;
    float extTemp;
    float extHum;
    float pressure;
    uint8_t inPercent;
    uint8_t outPercent;
    bool layoutDrawn;
};

static HomeDisplayCache homeCache;

static void Display_invalidateHomeCache()
{
    homeCache.intTemp = NAN;
    homeCache.intHum = NAN;
    homeCache.extTemp = NAN;
    homeCache.extHum = NAN;
    homeCache.pressure = NAN;
    homeCache.inPercent = HOME_INVALID_PERCENT;
    homeCache.outPercent = HOME_INVALID_PERCENT;
    homeCache.layoutDrawn = false;
}

static bool Display_floatChanged(float cached, float value)
{
    return isnan(cached) || cached != value;
}

static void Display_clearHomeValue(int16_t y)
{
    display.fillRect(HOME_VALUE_X,
                     y,
                     display.width() - HOME_VALUE_X,
                     HOME_LINE_H,
                     ILI9341_BLACK);
}

static void Display_drawHomeFloat(int16_t y,
                                  const char* suffix,
                                  float value)
{
    Display_clearHomeValue(y);
    display.setCursor(HOME_VALUE_X, y);
    display.print(value, 1);
    display.println(suffix);
}

static void Display_drawHomePercent(int16_t y,
                                    uint8_t value)
{
    Display_clearHomeValue(y);
    display.setCursor(HOME_VALUE_X, y);
    display.print(value);
    display.println(" %");
}

static void Display_drawHomeLayout()
{
    display.drawRGBBitmap(
        0,
        0,
        vmcBackground,
        VMC_BG_WIDTH,
        VMC_BG_HEIGHT);

    homeCache.layoutDrawn = true;
}

//=============================================================================

bool Display_begin()
{
    Display_invalidateHomeCache();

    pinMode(PIN_TFT_RST, OUTPUT);

    digitalWrite(PIN_TFT_RST, HIGH);
    delay(5);

    digitalWrite(PIN_TFT_RST, LOW);
    delay(15);

    digitalWrite(PIN_TFT_RST, HIGH);
    delay(15);

    SPI.begin(PIN_TFT_SCK, PIN_TFT_MISO, PIN_TFT_MOSI, PIN_TFT_CS);

    display.begin();

    display.setRotation(3);

    display.fillScreen(ILI9341_BLACK);

    return true;
}

//=============================================================================

void Display_clear()
{
    display.fillScreen(ILI9341_BLACK);
    Display_invalidateHomeCache();
}

//=============================================================================

void Display_showSplash()
{
    display.fillScreen(ILI9341_BLACK);
    Display_invalidateHomeCache();

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
    if (!homeCache.layoutDrawn)
    {
        Display_drawHomeLayout();
    }

    display.setTextColor(ILI9341_WHITE);
    display.setTextSize(2);

    if (Display_floatChanged(homeCache.intTemp, climate.intTemp))
{
    display.fillRect(15, 99, 106, 44, ILI9341_BLACK);

    display.setTextColor(ILI9341_WHITE);
    display.setTextSize(5);
    display.setCursor(20,108);

    display.print(climate.intTemp,1);
    //display.print("°");

    homeCache.intTemp = climate.intTemp;
}

    if (Display_floatChanged(homeCache.intHum, climate.intHum))
    {
        display.fillRect(57,170,55,28,ILI9341_BLACK);

display.setTextColor(ILI9341_WHITE);
display.setTextSize(4);
display.setCursor(73,174);

display.print((int)climate.intHum);
//display.print("%");
        homeCache.intHum = climate.intHum;
    }

    if (Display_floatChanged(homeCache.extTemp, climate.extTemp))
{
    display.fillRect(176, 99, 107, 44, ILI9341_BLACK);

    display.setTextColor(ILI9341_WHITE);
    display.setTextSize(5);
    display.setCursor(181, 108);

    display.print(climate.extTemp, 1);

    homeCache.extTemp = climate.extTemp;
}

    if (Display_floatChanged(homeCache.extHum, climate.extHum))
    {
        display.fillRect(218,170,55,28,ILI9341_BLACK);

display.setTextColor(ILI9341_WHITE);
display.setTextSize(4);
display.setCursor(233,174);

display.print((int)climate.extHum);
//display.print("%");
        homeCache.extHum = climate.extHum;
    }
/*
   if (Display_floatChanged(homeCache.pressure, climate.pressure))
    {
        Display_drawHomeFloat(165, " hPa", climate.pressure);
        homeCache.pressure = climate.pressure;
    }

    if (homeCache.inPercent != fans.inPercent)
    {
        Display_drawHomePercent(200, fans.inPercent);
        homeCache.inPercent = fans.inPercent;
    }

    if (homeCache.outPercent != fans.outPercent)
    {
        Display_drawHomePercent(220, fans.outPercent);
        homeCache.outPercent = fans.outPercent;
    }
*/
}
