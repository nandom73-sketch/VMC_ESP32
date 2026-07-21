/******************************************************************************
 * Display Module
 *
 * STATUS  : DEVELOPMENT
 * VERSION : 0.1.4
 *
 * PURPOSE
 * Gestione display ILI9341.
 *
 ******************************************************************************/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#include "Climate.h"
#include "Fans.h"

//=============================================================================
// Inizializzazione
//=============================================================================

bool Display_begin();

//=============================================================================
// Schermate
//=============================================================================

void Display_clear();

void Display_showSplash();

void Display_showHome(const SensorData& climate,
                      const FanData& fans);

#endif