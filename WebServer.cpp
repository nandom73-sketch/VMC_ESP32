/******************************************************************************
 * WebServer Module
 ******************************************************************************/

#include "WebServer.h"
#include "Climate.h"
#include "Fans.h"

#include <Arduino.h>
#include_next <WebServer.h>
#include <stdio.h>


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
} // namespace

bool WebServer_begin()
{
    server.on("/", handleRoot);
    server.begin();

    return true;
}

void WebServer_update()
{
    server.handleClient();
}
