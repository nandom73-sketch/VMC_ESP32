/******************************************************************************
 * VMC Firmware
 *
 * STATUS  : DEVELOPMENT
 * VERSION : 0.1.7
 *
 ******************************************************************************/

#include "Config.h"
#include "Climate.h"
#include "Display.h"
#include "Fans.h"
#include "HttpServer.h"
#include "WifiManager.h"

//=============================================================================
// Variabili globali
//=============================================================================

SensorData climate;
FanData fans;
bool webServerStarted = false;

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=================================");
    Serial.print(" VMC Firmware v");
    Serial.println(FW_VERSION);
    Serial.println("=================================");

    //---------------------------------------------------------
    // Climate
    //---------------------------------------------------------

    if (Climate_begin())
    {
        Serial.println("Climate : OK");
    }
    else
    {
        Serial.println("Climate : ERROR");
    }

    //---------------------------------------------------------
    // Fans
    //---------------------------------------------------------

    if (Fans_begin())
    {
        Serial.println("Fans    : OK");
    }
    else
    {
        Serial.println("Fans    : ERROR");
    }

    //---------------------------------------------------------
    // Display
    //---------------------------------------------------------

    if (Display_begin())
    {
        Serial.println("Display : OK");
        Display_showSplash();
    }
    else
    {
        Serial.println("Display : ERROR");
    }


    //---------------------------------------------------------
    // WiFi + WebServer
    //---------------------------------------------------------

    if (WifiManager_begin())
    {
        Serial.println("WiFi    : OK");

        if (WebServer_begin())
        {
            webServerStarted = true;
            Serial.println("WebServer: OK");
        }
        else
        {
            Serial.println("WebServer: ERROR");
        }
    }
    else
    {
        Serial.println("WiFi    : ERROR");
        Serial.println("WebServer: SKIPPED");
    }

    Serial.println();

    fans.inPercent = 0;
    fans.outPercent = 0;
}

//=============================================================================
// Loop
//=============================================================================

void loop()
{
    //---------------------------------------------------------
    // Lettura sensori
    //---------------------------------------------------------

    Climate_read(climate);

    //---------------------------------------------------------
    // Stato ventole
    //---------------------------------------------------------

    fans.inPercent  = Fans_getIn();
    fans.outPercent = Fans_getOut();

    //---------------------------------------------------------
    // Display
    //---------------------------------------------------------

    Display_showHome(climate, fans);

    //---------------------------------------------------------
    // Debug seriale
    //---------------------------------------------------------

    Serial.print("INT : ");
    Serial.print(climate.intTemp);
    Serial.print(" °C   ");
    Serial.print(climate.intHum);
    Serial.println(" %");

    Serial.print("EXT : ");
    Serial.print(climate.extTemp);
    Serial.print(" °C   ");
    Serial.print(climate.extHum);
    Serial.println(" %");

    Serial.print("PRES: ");
    Serial.print(climate.pressure);
    Serial.println(" hPa");

    Serial.print("FAN IN : ");
    Serial.print(fans.inPercent);
    Serial.println(" %");

    Serial.print("FAN OUT: ");
    Serial.print(fans.outPercent);
    Serial.println(" %");

    Serial.println("-----------------------------");

    if (webServerStarted)
    {
        WebServer_update();
    }

    delay(2000);
}
