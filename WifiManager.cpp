/******************************************************************************
 * WiFi Module
 *
 * PURPOSE
 * Connette il dispositivo a una rete Wi-Fi esistente in modalità STA.
 *
 ******************************************************************************/

#include "WifiManager.h"

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>

namespace
{
// Credenziali provvisorie: in una revisione futura saranno caricate da
// configurazione persistente (Preferences/NVS) senza modificare l'interfaccia
// pubblica del modulo.
constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";

constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;
constexpr unsigned long WIFI_CONNECT_POLL_MS = 250;
constexpr char MDNS_HOSTNAME[] = "vmc";

struct WifiManagerCredentials
{
    const char* ssid;
    const char* password;
};

bool wifiConnected = false;

WifiManagerCredentials loadCredentials()
{
    // Punto unico di sostituzione per una futura lettura da Preferences/NVS.
    return { WIFI_SSID, WIFI_PASSWORD };
}

bool waitForConnection()
{
    const unsigned long startMs = millis();

    while ((WiFi.status() != WL_CONNECTED) &&
           ((millis() - startMs) < WIFI_CONNECT_TIMEOUT_MS))
    {
        delay(WIFI_CONNECT_POLL_MS);
        Serial.print('.');
    }

    Serial.println();

    return (WiFi.status() == WL_CONNECTED);
}
} // namespace

bool WifiManager_begin()
{
    wifiConnected = false;

    Serial.println();
    Serial.println(F("===== WiFi Begin ====="));
    const WifiManagerCredentials credentials = loadCredentials();

    Serial.print(F("Connecting to SSID: "));
    Serial.println(credentials.ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(credentials.ssid, credentials.password);

    if (!waitForConnection())
    {
        WiFi.disconnect(true);
        Serial.println(F("WiFi: connection timeout"));
        Serial.println(F("======================"));
        return false;
    }

    wifiConnected = true;

    Serial.print(F("WiFi IP: "));
    Serial.println(WiFi.localIP());

    if (MDNS.begin(MDNS_HOSTNAME))
    {
        Serial.print(F("mDNS: http://"));
        Serial.print(MDNS_HOSTNAME);
        Serial.println(F(".local/"));
    }
    else
    {
        Serial.println(F("mDNS: ERROR"));
    }

    Serial.println(F("======================"));

    return true;
}

bool WifiManager_isConnected()
{
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

IPAddress WifiManager_getLocalIp()
{
    return WiFi.localIP();
}
