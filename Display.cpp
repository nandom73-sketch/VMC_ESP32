/******************************************************************************
 * Display Module
 *
 * STATUS  : DEVELOPMENT
 * VERSION : 0.1.9 - Full Home redraw diagnostic for Smooth Font artefacts
 *
 ******************************************************************************/

#include "Display.h"
#include "Config.h"

#include <SPI.h>
#include <TFT_eSPI.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "Roboto40.h"
#include "Roboto50.h"
#include "Roboto65.h"
#include "VMC_UI_v2.h"

//=============================================================================

static TFT_eSPI display;

//=============================================================================

static constexpr uint8_t DISPLAY_BACKLIGHT_PIN = 16;
static constexpr uint8_t DISPLAY_ROTATION = 3;
static constexpr uint8_t HOME_INVALID_PERCENT = 255;
static constexpr int16_t HOME_HUMIDITY_SPACING = 4;

// Temperature centers are shifted 10 px right in the approved layout.
static constexpr int16_t HOME_INT_TEMP_CENTER_X = 78;
static constexpr int16_t HOME_EXT_TEMP_CENTER_X = 239;
static constexpr int16_t HOME_TEMP_TEXT_Y = 98;

// Humidity value positions in vmcBackground.
static constexpr int16_t HOME_INT_HUM_CENTER_X = 84;
static constexpr int16_t HOME_EXT_HUM_CENTER_X = 245;
static constexpr int16_t HOME_HUM_TEXT_Y = 160;

//=============================================================================

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

//=============================================================================

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

//=============================================================================

static bool Display_floatChanged(float cached, float value)
{
    return isnan(cached) || cached != value;
}

//=============================================================================

static void Display_drawHomeLayout()
{
    // vmcBackground stores RGB565 pixels in big-endian byte order.
    display.setSwapBytes(true);
    display.pushImage(0,
                      0,
                      VMC_BG_WIDTH,
                      VMC_BG_HEIGHT,
                      vmcBackground);

    homeCache.layoutDrawn = true;
}

//=============================================================================

static void Display_prepareTemperature(float temperature,
                                       char* integerText,
                                       size_t integerTextSize,
                                       char* decimalText,
                                       size_t decimalTextSize)
{
    snprintf(integerText, integerTextSize, "%.1f", temperature);

    char* decimalPart = strchr(integerText, '.');
    char decimalDigit = '0';

    if (decimalPart != nullptr)
    {
        decimalDigit = decimalPart[1];
        *decimalPart = '\0';
    }

    snprintf(decimalText, decimalTextSize, ".%c", decimalDigit);
}

//=============================================================================

static void Display_drawTemperature(float temperature, int16_t centerX)
{
    char integerText[12];
    char decimalText[3];

    Display_prepareTemperature(temperature,
                               integerText,
                               sizeof(integerText),
                               decimalText,
                               sizeof(decimalText));

    display.loadFont(Roboto65);
    const int16_t integerWidth = display.textWidth(integerText);
    display.unloadFont();

    display.loadFont(Roboto40);
    const int16_t decimalWidth = display.textWidth(decimalText);
    const int16_t unitWidth = display.textWidth("°C");
    display.unloadFont();

    const int16_t textX =
        centerX - ((integerWidth + decimalWidth + unitWidth) / 2);

    display.loadFont(Roboto65);
    display.drawString(integerText, textX, HOME_TEMP_TEXT_Y);
    display.unloadFont();

    display.loadFont(Roboto40);
    display.drawString(decimalText,
                       textX + integerWidth,
                       HOME_TEMP_TEXT_Y);
    display.drawString("°C",
                       textX + integerWidth + decimalWidth,
                       HOME_TEMP_TEXT_Y);
    display.unloadFont();
}

//=============================================================================

static void Display_drawHumidity(float humidity, int16_t centerX)
{
    char humidityText[4];
    snprintf(humidityText,
             sizeof(humidityText),
             "%d",
             static_cast<int>(humidity));

    display.loadFont(Roboto50);
    const int16_t valueWidth = display.textWidth(humidityText);
    display.unloadFont();

    display.loadFont(Roboto40);
    const int16_t unitWidth = display.textWidth("%");
    display.unloadFont();

    // Center the complete "XX %" group, including the approved spacing.
    const int16_t textX = centerX -
        ((valueWidth + HOME_HUMIDITY_SPACING + unitWidth) / 2);

    display.loadFont(Roboto50);
    display.drawString(humidityText, textX, HOME_HUM_TEXT_Y);
    display.unloadFont();

    display.loadFont(Roboto40);
    display.drawString("%",
                       textX + valueWidth + HOME_HUMIDITY_SPACING,
                       HOME_HUM_TEXT_Y);
    display.unloadFont();
}

//=============================================================================
// HOME DISPLAY
//=============================================================================

void Display_showHome(const SensorData& climate,
                      const FanData& fans)
{
    (void)fans;

    const bool dynamicValueChanged =
        Display_floatChanged(homeCache.intTemp, climate.intTemp) ||
        Display_floatChanged(homeCache.extTemp, climate.extTemp) ||
        Display_floatChanged(homeCache.intHum, climate.intHum) ||
        Display_floatChanged(homeCache.extHum, climate.extHum);

    if (!homeCache.layoutDrawn || dynamicValueChanged)
    {
        // Diagnostic path: replace the complete Home background before drawing
        // every dynamic value, avoiding all partial-area TFT refreshes.
        Display_drawHomeLayout();

        Display_drawTemperature(climate.intTemp, HOME_INT_TEMP_CENTER_X);
        Display_drawTemperature(climate.extTemp, HOME_EXT_TEMP_CENTER_X);
        Display_drawHumidity(climate.intHum, HOME_INT_HUM_CENTER_X);
        Display_drawHumidity(climate.extHum, HOME_EXT_HUM_CENTER_X);

        homeCache.intTemp = climate.intTemp;
        homeCache.extTemp = climate.extTemp;
        homeCache.intHum = climate.intHum;
        homeCache.extHum = climate.extHum;
    }
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

    SPI.begin(PIN_TFT_SCK,
              PIN_TFT_MISO,
              PIN_TFT_MOSI,
              PIN_TFT_CS);

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
