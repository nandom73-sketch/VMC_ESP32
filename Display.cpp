/******************************************************************************
 * Display Module
 *
 * STATUS  : DEVELOPMENT
 * VERSION : 0.1.6 - Smooth Font temperature rendering
 *
 ******************************************************************************/

#include "Display.h"
#include "Config.h"

#include <SPI.h>
#include <TFT_eSPI.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "Roboto55.h"
#include "Roboto40.h"
#include "VMC_UI_v1.h"

//=============================================================================

static TFT_eSPI display;

//=============================================================================

static constexpr uint8_t DISPLAY_BACKLIGHT_PIN = 16;
static constexpr uint8_t DISPLAY_ROTATION = 3;
static constexpr uint8_t HOME_INVALID_PERCENT = 255;

// Temperature value panels in vmcBackground.  The clear rectangles stay within
// the black panels so only the changed value is refreshed.
static constexpr int16_t HOME_INT_TEMP_CLEAR_X = 15;
static constexpr int16_t HOME_EXT_TEMP_CLEAR_X = 176;
static constexpr int16_t HOME_TEMP_CLEAR_Y = 96;
static constexpr int16_t HOME_TEMP_CLEAR_W = 106;
static constexpr int16_t HOME_TEMP_CLEAR_H = 52;
static constexpr int16_t HOME_TEMP_TEXT_Y = 98;

static constexpr int16_t HOME_INT_TEMP_CENTER_X =
    HOME_INT_TEMP_CLEAR_X + (HOME_TEMP_CLEAR_W / 2);
static constexpr int16_t HOME_EXT_TEMP_CENTER_X =
    HOME_EXT_TEMP_CLEAR_X + (HOME_TEMP_CLEAR_W / 2);

static constexpr int16_t HOME_INT_HUM_X = 73;
static constexpr int16_t HOME_EXT_HUM_X = 233;
static constexpr int16_t HOME_HUM_CLEAR_Y = 160;
static constexpr int16_t HOME_HUM_CLEAR_W = 55;
static constexpr int16_t HOME_HUM_CLEAR_H = 40;
static constexpr int16_t HOME_HUM_BASELINE = 192;
static constexpr int16_t HOME_INT_HUM_CLEAR_X = 57;
static constexpr int16_t HOME_EXT_HUM_CLEAR_X = 218;

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

static void Display_drawHomeLayout()
{
    display.pushImage(0,
                      0,
                      VMC_BG_WIDTH,
                      VMC_BG_HEIGHT,
                      vmcBackground);

    homeCache.layoutDrawn = true;
}

static void Display_drawTemperature(float temperature,
                                    int16_t clearX,
                                    int16_t centerX)
{
    char temperatureText[12];
    snprintf(temperatureText, sizeof(temperatureText), "%.1f", temperature);

    char* decimalPart = strchr(temperatureText, '.');
    char decimalDigit = '0';
    if (decimalPart != nullptr)
    {
        decimalDigit = decimalPart[1];
        *decimalPart = '\0';
    }

    // snprintf supplies the decimal point for finite values.  Keeping the
    // fallback digit also preserves the required one-decimal presentation.
    char decimalText[3] = { '.', decimalDigit, '\0' };

    // Clear before changing fonts.  Smooth fonts blend their antialiased edge
    // pixels with the supplied background colour, which must remain black.
    display.fillRect(clearX,
                     HOME_TEMP_CLEAR_Y,
                     HOME_TEMP_CLEAR_W,
                     HOME_TEMP_CLEAR_H,
                     TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(TL_DATUM);

    display.loadFont(Roboto55);
    const int16_t integerWidth = display.textWidth(temperatureText);
    display.unloadFont();

    display.loadFont(Roboto40);
    const int16_t decimalWidth = display.textWidth(decimalText);
    const int16_t textX = centerX - ((integerWidth + decimalWidth) / 2);
    display.drawString(decimalText, textX + integerWidth, HOME_TEMP_TEXT_Y);
    display.unloadFont();

    display.loadFont(Roboto55);
    display.drawString(temperatureText, textX, HOME_TEMP_TEXT_Y);
    display.unloadFont();
}

static void Display_drawHumidity(float humidity,
                                 int16_t clearX,
                                 int16_t textX)
{
    display.fillRect(clearX,
                     HOME_HUM_CLEAR_Y,
                     HOME_HUM_CLEAR_W,
                     HOME_HUM_CLEAR_H,
                     TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setFreeFont(&FreeSansBold18pt7b);
    display.setCursor(textX, HOME_HUM_BASELINE);
    display.print(static_cast<int>(humidity));
    display.setFreeFont(nullptr);
}

//=============================================================================

bool Display_begin()
{
    Display_invalidateHomeCache();

    pinMode(DISPLAY_BACKLIGHT_PIN, OUTPUT);
    analogWrite(DISPLAY_BACKLIGHT_PIN, 255);

    pinMode(PIN_TFT_RST, OUTPUT);

    digitalWrite(PIN_TFT_RST, HIGH);
    delay(5);
    digitalWrite(PIN_TFT_RST, LOW);
    delay(15);
    digitalWrite(PIN_TFT_RST, HIGH);
    delay(15);

    SPI.begin(PIN_TFT_SCK, PIN_TFT_MISO, PIN_TFT_MOSI, PIN_TFT_CS);

    display.init();
    display.setRotation(DISPLAY_ROTATION);
    display.fillScreen(TFT_BLACK);

    return true;
}

//=============================================================================

void Display_clear()
{
    display.fillScreen(TFT_BLACK);
    Display_invalidateHomeCache();
}

//=============================================================================

void Display_showSplash()
{
    display.fillScreen(TFT_BLACK);
    Display_invalidateHomeCache();

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextFont(1);
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
    (void)fans;

    if (!homeCache.layoutDrawn)
    {
        Display_drawHomeLayout();
    }

    if (Display_floatChanged(homeCache.intTemp, climate.intTemp))
    {
        Display_drawTemperature(climate.intTemp,
                                HOME_INT_TEMP_CLEAR_X,
                                HOME_INT_TEMP_CENTER_X);
        homeCache.intTemp = climate.intTemp;
    }

    if (Display_floatChanged(homeCache.intHum, climate.intHum))
    {
        Display_drawHumidity(climate.intHum,
                             HOME_INT_HUM_CLEAR_X,
                             HOME_INT_HUM_X);
        homeCache.intHum = climate.intHum;
    }

    if (Display_floatChanged(homeCache.extTemp, climate.extTemp))
    {
        Display_drawTemperature(climate.extTemp,
                                HOME_EXT_TEMP_CLEAR_X,
                                HOME_EXT_TEMP_CENTER_X);
        homeCache.extTemp = climate.extTemp;
    }

    if (Display_floatChanged(homeCache.extHum, climate.extHum))
    {
        Display_drawHumidity(climate.extHum,
                             HOME_EXT_HUM_CLEAR_X,
                             HOME_EXT_HUM_X);
        homeCache.extHum = climate.extHum;
    }
}
