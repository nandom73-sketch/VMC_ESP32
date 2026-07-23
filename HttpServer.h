/******************************************************************************
 * HttpServer Module
 *
 * PURPOSE
 * Pubblica una semplice pagina di stato VMC tramite ESP32 WebServer.
 *
 ******************************************************************************/

#ifndef VMC_HTTP_SERVER_H
#define VMC_HTTP_SERVER_H

bool WebServer_begin();
void WebServer_update();

#endif
