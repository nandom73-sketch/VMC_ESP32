/******************************************************************************
 * WiFi Module
 *
 * PURPOSE
 * Gestione connettività Wi-Fi STA e pubblicazione mDNS.
 *
 ******************************************************************************/

#ifndef VMC_WIFI_H
#define VMC_WIFI_H

#include <IPAddress.h>

bool Wifi_begin();
bool Wifi_isConnected();
IPAddress Wifi_getLocalIp();

#endif
