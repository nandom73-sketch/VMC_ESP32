/******************************************************************************
 * HttpServer Module
 ******************************************************************************/

#include "HttpServer.h"
#include "Climate.h"
#include "Fans.h"
#include "Vmc.h"

#include <Arduino.h>
#include <WebServer.h>
#include <stdio.h>
#include <stdlib.h>


namespace
{
constexpr uint16_t WEB_SERVER_PORT = 80;
constexpr size_t HTML_BUFFER_SIZE = 768;

::WebServer server(WEB_SERVER_PORT);
char htmlBuffer[HTML_BUFFER_SIZE];

void handleRoot()
{
    const SensorData& climate = Climate_getData();
    const FanData& fans = Fans_getData();

    snprintf(htmlBuffer,
             sizeof(htmlBuffer),
             "<!doctype html>"
             "<html>"
             "<head>"
             "<meta charset=\"utf-8\">"
             "<title>VMC Status</title>"
             "</head>"
             "<body>"
             "<p>Indoor temperature: %.1f C</p>"
             "<p>Indoor humidity: %.1f %%</p>"
             "<p>Outdoor temperature: %.1f C</p>"
             "<p>Outdoor humidity: %.1f %%</p>"
             "<p>Fan IN: %u %%</p>"
             "<p>Fan OUT: %u %%</p>"
             "</body>"
             "</html>",
             climate.intTemp,
             climate.intHum,
             climate.extTemp,
             climate.extHum,
             fans.inPercent,
             fans.outPercent);

    server.send(200, "text/html", htmlBuffer);
}

const char* vmcModeToString(VmcMode mode)
{
    switch (mode)
    {
        case VmcMode::OFF:
            return "OFF";
        case VmcMode::MANUALE:
            return "MANUALE";
        case VmcMode::BOOST:
            return "BOOST";
    }

    return "OFF";
}

void sendVmcStatus()
{
    const VmcData& vmc = Vmc_getData();

    snprintf(htmlBuffer,
             sizeof(htmlBuffer),
             "{\"mode\":\"%s\",\"speed\":%u,\"boostRemainingSeconds\":%lu}",
             vmcModeToString(vmc.mode),
             vmc.speed,
             static_cast<unsigned long>(vmc.boostRemainingSeconds));

    server.send(200, "application/json", htmlBuffer);
}

void sendBadRequest(const char* message)
{
    snprintf(htmlBuffer,
             sizeof(htmlBuffer),
             "{\"error\":\"%s\"}",
             message);
    server.send(400, "application/json", htmlBuffer);
}

void handleSetSpeed()
{
    if (!server.hasArg("speed"))
    {
        sendBadRequest("Missing speed parameter");
        return;
    }

    const String speedArgument = server.arg("speed");
    char* end = nullptr;
    const long speed = strtol(speedArgument.c_str(), &end, 10);

    if (speedArgument.length() == 0 || *end != '\0' || speed < 0 || speed > 7)
    {
        sendBadRequest("Speed must be an integer between 0 and 7");
        return;
    }

    Vmc_setManualSpeed(static_cast<uint8_t>(speed));
    sendVmcStatus();
}

void handleStartBoost()
{
    Vmc_startBoost();
    sendVmcStatus();
}

void handleCancelBoost()
{
    Vmc_cancelBoost();
    sendVmcStatus();
}
} // namespace

bool WebServer_begin()
{
    server.on("/", handleRoot);
    server.on("/api/status", HTTP_GET, sendVmcStatus);
    server.on("/api/speed", HTTP_POST, handleSetSpeed);
    server.on("/api/boost", HTTP_POST, handleStartBoost);
    server.on("/api/boost/cancel", HTTP_POST, handleCancelBoost);
    server.begin();

    return true;
}

void WebServer_update()
{
    server.handleClient();
}
