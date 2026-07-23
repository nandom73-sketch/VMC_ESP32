/******************************************************************************
 * WiFi Module
 *
 * PURPOSE
 * Gestione connettività Wi-Fi STA e pubblicazione mDNS.
 *
 ******************************************************************************/

#ifndef VMC_WIFI_MANAGER_H
#define VMC_WIFI_MANAGER_H

#include <IPAddress.h>

bool WifiManager_begin();
bool WifiManager_isConnected();
IPAddress WifiManager_getLocalIp();

#endif
