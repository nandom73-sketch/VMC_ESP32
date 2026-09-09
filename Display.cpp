/******************************************************************************
 * Display Module
 *
 * STATUS  : DEVELOPMENT
 * VERSION : 0.1.7 - Smooth Font rendering optimized by font size
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

// Temperature value panels in vmcBackground.
// The clear rectangles stay within the black panels so only the changed
// value is refreshed.

// Temperature
static constexpr int16_t HOME_INT_TEMP_CLEAR_X = 15;
static constexpr int16_t HOME_EXT_TEMP_CLEAR_X = 176;
static constexpr int16_t HOME_TEMP_CLEAR_Y = 96;
static constexpr int16_t HOME_TEMP_CLEAR_W = 106;
static constexpr int16_t HOME_TEMP_CLEAR_H = 52;
static constexpr int16_t HOME_TEMP_TEXT_Y = 98;

// Temperature centers.
// Shifted 10 px to the right to better match the graphic layout.
static constexpr int16_t HOME_INT_TEMP_CENTER_X =
    HOME_INT_TEMP_CLEAR_X + (HOME_TEMP_CLEAR_W / 2) + 10;

static constexpr int16_t HOME_EXT_TEMP_CENTER_X =
    HOME_EXT_TEMP_CLEAR_X + (HOME_TEMP_CLEAR_W / 2) + 10;

// Humidity
static constexpr int16_t HOME_HUM_CLEAR_Y = 160;
static constexpr int16_t HOME_HUM_CLEAR_W = 55;
static constexpr int16_t HOME_HUM_CLEAR_H = 40;
static constexpr int16_t HOME_HUM_TEXT_Y = HOME_HUM_CLEAR_Y;

static constexpr int16_t HOME_INT_HUM_CLEAR_X = 57;
static constexpr int16_t HOME_EXT_HUM_CLEAR_X = 218;

static constexpr int16_t HOME_INT_HUM_CENTER_X =
    HOME_INT_HUM_CLEAR_X + (HOME_HUM_CLEAR_W / 2);

static constexpr int16_t HOME_EXT_HUM_CENTER_X =
    HOME_EXT_HUM_CLEAR_X + (HOME_HUM_CLEAR_W / 2);

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
    // TFT_eSPI needs swapping enabled when transferring this image buffer.
    display.setSwapBytes(true);

    display.pushImage(0,
                      0,
                      VMC_BG_WIDTH,
                      VMC_BG_HEIGHT,
                      vmcBackground);

    homeCache.layoutDrawn = true;
}

//=============================================================================
// TEMPERATURE RENDER DATA
//=============================================================================

struct TemperatureRenderData
{
    char integerText[12];
    char decimalText[3];

    int16_t integerWidth;
    int16_t decimalWidth;
    int16_t unitWidth;

    int16_t textX;
};

static void Display_prepareTemperature(float temperature,
                                       int16_t centerX,
                                       TemperatureRenderData& data)
{
    snprintf(data.integerText,
             sizeof(data.integerText),
             "%.1f",
             temperature);

    char* decimalPart = strchr(data.integerText, '.');
    char decimalDigit = '0';

    if (decimalPart != nullptr)
    {
        decimalDigit = decimalPart[1];
        *decimalPart = '\0';
    }

    data.decimalText[0] = '.';
    data.decimalText[1] = decimalDigit;
    data.decimalText[2] = '\0';

    // The widths are filled by Display_prepareHomeTextMetrics().
    data.integerWidth = 0;
    data.decimalWidth = 0;
    data.unitWidth = 0;
    data.textX = centerX;
}

//=============================================================================

static void Display_prepareHumidity(float humidity,
                                    int16_t centerX,
                                    char* valueText,
                                    size_t valueTextSize,
                                    int16_t& valueWidth,
                                    int16_t& unitWidth,
                                    int16_t& textX)
{
    snprintf(valueText,
             valueTextSize,
             "%d",
             static_cast<int>(humidity));

    // Roboto50
    display.loadFont(Roboto50);
    valueWidth = display.textWidth(valueText);
    display.unloadFont();

    // Roboto40
    display.loadFont(Roboto40);
    unitWidth = display.textWidth("%");
    display.unloadFont();

    const int16_t spacing = 4;
    const int16_t totalWidth = valueWidth + spacing + unitWidth;

    textX = centerX - (totalWidth / 2);
}

//=============================================================================

static void Display_drawTemperatureWithLoadedFonts(
    const TemperatureRenderData& data,
    int16_t textY)
{
    const char* unitText = "°C";

    // Integer - Roboto65
    display.loadFont(Roboto65);
    display.drawString(data.integerText,
                       data.textX,
                       textY);
    display.unloadFont();

    // Decimal and unit - Roboto40
    display.loadFont(Roboto40);

    display.drawString(data.decimalText,
                       data.textX + data.integerWidth,
                       textY);

    display.drawString(unitText,
                       data.textX + data.integerWidth + data.decimalWidth,
                       textY);

    display.unloadFont();
}

//=============================================================================

static void Display_drawHumidityWithLoadedFont(
    const char* valueText,
    int16_t valueX,
    int16_t unitX,
    int16_t textY)
{
    // Value - Roboto50
    display.loadFont(Roboto50);
    display.drawString(valueText,
                       valueX,
                       textY);
    display.unloadFont();

    // Unit - Roboto40
    display.loadFont(Roboto40);
    display.drawString("%",
                       unitX,
                       textY);
    display.unloadFont();
}

//=============================================================================
// HOME DISPLAY
//=============================================================================

void Display_showHome(const SensorData& climate,
                      const FanData& fans)
{
    (void)fans;

    if (!homeCache.layoutDrawn)
    {
        Display_drawHomeLayout();
    }

    const bool intTempChanged =
        Display_floatChanged(homeCache.intTemp, climate.intTemp);

    const bool extTempChanged =
        Display_floatChanged(homeCache.extTemp, climate.extTemp);

    const bool intHumChanged =
        Display_floatChanged(homeCache.intHum, climate.intHum);

    const bool extHumChanged =
        Display_floatChanged(homeCache.extHum, climate.extHum);

    // Nothing changed.
    if (!intTempChanged &&
        !extTempChanged &&
        !intHumChanged &&
        !extHumChanged)
    {
        return;
    }

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(TL_DATUM);

    //=========================================================================
    // PREPARE TEMPERATURE TEXT
    //=========================================================================

    TemperatureRenderData intTempData;
    TemperatureRenderData extTempData;

    if (intTempChanged)
    {
        Display_prepareTemperature(climate.intTemp,
                                   HOME_INT_TEMP_CENTER_X,
                                   intTempData);
    }

    if (extTempChanged)
    {
        Display_prepareTemperature(climate.extTemp,
                                   HOME_EXT_TEMP_CENTER_X,
                                   extTempData);
    }

    //=========================================================================
    // MEASURE INTEGER TEMPERATURES WITH ROBOTO65
    //=========================================================================

    if (intTempChanged || extTempChanged)
    {
        display.loadFont(Roboto65);

        if (intTempChanged)
        {
            intTempData.integerWidth =
                display.textWidth(intTempData.integerText);
        }

        if (extTempChanged)
        {
            extTempData.integerWidth =
                display.textWidth(extTempData.integerText);
        }

        display.unloadFont();
    }

    //=========================================================================
    // MEASURE DECIMALS, UNITS AND HUMIDITY UNITS WITH ROBOTO40
    //=========================================================================

    if (intTempChanged || extTempChanged)
    {
        display.loadFont(Roboto40);

        const int16_t unitWidth =
            display.textWidth("°C");

        if (intTempChanged)
        {
            intTempData.decimalWidth =
                display.textWidth(intTempData.decimalText);

            intTempData.unitWidth = unitWidth;
        }

        if (extTempChanged)
        {
            extTempData.decimalWidth =
                display.textWidth(extTempData.decimalText);

            extTempData.unitWidth = unitWidth;
        }

        display.unloadFont();
    }

    //=========================================================================
    // CALCULATE TEMPERATURE POSITIONS
    //=========================================================================

    if (intTempChanged)
    {
        const int16_t totalWidth =
            intTempData.integerWidth +
            intTempData.decimalWidth +
            intTempData.unitWidth;

        intTempData.textX =
            HOME_INT_TEMP_CENTER_X - (totalWidth / 2);
    }

    if (extTempChanged)
    {
        const int16_t totalWidth =
            extTempData.integerWidth +
            extTempData.decimalWidth +
            extTempData.unitWidth;

        extTempData.textX =
            HOME_EXT_TEMP_CENTER_X - (totalWidth / 2);
    }

    //=========================================================================
    // PREPARE HUMIDITY
    //=========================================================================

    char intHumText[4];
    char extHumText[4];

    int16_t intHumValueWidth = 0;
    int16_t intHumUnitWidth = 0;
    int16_t intHumTextX = 0;

    int16_t extHumValueWidth = 0;
    int16_t extHumUnitWidth = 0;
    int16_t extHumTextX = 0;

    if (intHumChanged)
    {
        snprintf(intHumText,
                 sizeof(intHumText),
                 "%d",
                 static_cast<int>(climate.intHum));

        display.loadFont(Roboto50);
        intHumValueWidth = display.textWidth(intHumText);
        display.unloadFont();

        display.loadFont(Roboto40);
        intHumUnitWidth = display.textWidth("%");
        display.unloadFont();

        const int16_t totalWidth =
            intHumValueWidth + 4 + intHumUnitWidth;

        intHumTextX =
            HOME_INT_HUM_CENTER_X - (totalWidth / 2);
    }

    if (extHumChanged)
    {
        snprintf(extHumText,
                 sizeof(extHumText),
                 "%d",
                 static_cast<int>(climate.extHum));

        display.loadFont(Roboto50);
        extHumValueWidth = display.textWidth(extHumText);
        display.unloadFont();

        display.loadFont(Roboto40);
        extHumUnitWidth = display.textWidth("%");
        display.unloadFont();

        const int16_t totalWidth =
            extHumValueWidth + 4 + extHumUnitWidth;

        extHumTextX =
            HOME_EXT_HUM_CENTER_X - (totalWidth / 2);
    }

    //=========================================================================
    // CLEAR ONLY CHANGED VALUE AREAS
    //=========================================================================

    if (intTempChanged)
    {
        display.fillRect(HOME_INT_TEMP_CLEAR_X,
                         HOME_TEMP_CLEAR_Y,
                         HOME_TEMP_CLEAR_W,
                         HOME_TEMP_CLEAR_H,
                         TFT_BLACK);
    }

    if (extTempChanged)
    {
        display.fillRect(HOME_EXT_TEMP_CLEAR_X,
                         HOME_TEMP_CLEAR_Y,
                         HOME_TEMP_CLEAR_W,
                         HOME_TEMP_CLEAR_H,
                         TFT_BLACK);
    }

    if (intHumChanged)
    {
        display.fillRect(HOME_INT_HUM_CLEAR_X,
                         HOME_HUM_CLEAR_Y,
                         HOME_HUM_CLEAR_W,
                         HOME_HUM_CLEAR_H,
                         TFT_BLACK);
    }

    if (extHumChanged)
    {
        display.fillRect(HOME_EXT_HUM_CLEAR_X,
                         HOME_HUM_CLEAR_Y,
                         HOME_HUM_CLEAR_W,
                         HOME_HUM_CLEAR_H,
                         TFT_BLACK);
    }

    //=========================================================================
    // DRAW TEMPERATURES
    //=========================================================================

    if (intTempChanged || extTempChanged)
    {
        if (intTempChanged)
        {
            // Integer part - Roboto65
            display.loadFont(Roboto65);
            display.drawString(intTempData.integerText,
                               intTempData.textX,
                               HOME_TEMP_TEXT_Y);
            display.unloadFont();

            // Decimal + °C - Roboto40
            display.loadFont(Roboto40);

            display.drawString(intTempData.decimalText,
                               intTempData.textX +
                                   intTempData.integerWidth,
                               HOME_TEMP_TEXT_Y);

            display.drawString("°C",
                               intTempData.textX +
                                   intTempData.integerWidth +
                                   intTempData.decimalWidth,
                               HOME_TEMP_TEXT_Y);

            display.unloadFont();
        }

        if (extTempChanged)
        {
            // Integer part - Roboto65
            display.loadFont(Roboto65);
            display.drawString(extTempData.integerText,
                               extTempData.textX,
                               HOME_TEMP_TEXT_Y);
            display.unloadFont();

            // Decimal + °C - Roboto40
            display.loadFont(Roboto40);

            display.drawString(extTempData.decimalText,
                               extTempData.textX +
                                   extTempData.integerWidth,
                               HOME_TEMP_TEXT_Y);

            display.drawString("°C",
                               extTempData.textX +
                                   extTempData.integerWidth +
                                   extTempData.decimalWidth,
                               HOME_TEMP_TEXT_Y);

            display.unloadFont();
        }
    }

    //=========================================================================
    // DRAW HUMIDITY
    //=========================================================================

    if (intHumChanged || extHumChanged)
    {
        if (intHumChanged)
        {
            // Value - Roboto50
            display.loadFont(Roboto50);
            display.drawString(intHumText,
                               intHumTextX,
                               HOME_HUM_TEXT_Y);
            display.unloadFont();

            // % - Roboto40
            display.loadFont(Roboto40);
            display.drawString("%",
                               intHumTextX +
                                   intHumValueWidth + 4,
                               HOME_HUM_TEXT_Y);
            display.unloadFont();
        }

        if (extHumChanged)
        {
            // Value - Roboto50
            display.loadFont(Roboto50);
            display.drawString(extHumText,
                               extHumTextX,
                               HOME_HUM_TEXT_Y);
            display.unloadFont();

            // % - Roboto40
            display.loadFont(Roboto40);
            display.drawString("%",
                               extHumTextX +
                                   extHumValueWidth + 4,
                               HOME_HUM_TEXT_Y);
            display.unloadFont();
        }
    }

    //=========================================================================
    // UPDATE CACHE
    //=========================================================================

    if (intTempChanged)
    {
        homeCache.intTemp = climate.intTemp;
    }

    if (extTempChanged)
    {
        homeCache.extTemp = climate.extTemp;
    }

    if (intHumChanged)
    {
        homeCache.intHum = climate.intHum;
    }

    if (extHumChanged)
    {
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