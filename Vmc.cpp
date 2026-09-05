/******************************************************************************
 * VMC Logic Module
 ******************************************************************************/

#include "Vmc.h"

#include <Arduino.h>
#include <limits.h>

#include "Config.h"
#include "Fans.h"

//==============================================================================
// Variabili private
//==============================================================================

static VmcData vmc =
{
    VmcMode::OFF,
    0,
    0
};

static uint32_t lastCountdownMillis = 0;

static void Vmc_stop()
{
    Fans_stopAll();
    vmc.mode = VmcMode::OFF;
    vmc.speed = 0;
    vmc.boostRemainingSeconds = 0;
}

//==============================================================================
// Inizializzazione
//==============================================================================

bool Vmc_begin()
{
    vmc.mode = VmcMode::OFF;
    vmc.speed = 0;
    vmc.boostRemainingSeconds = 0;
    lastCountdownMillis = millis();

    return true;
}

//==============================================================================
// Aggiornamento countdown Boost
//==============================================================================

void Vmc_update()
{
    if (vmc.mode != VmcMode::BOOST)
        return;

    const uint32_t now = millis();
    const uint32_t elapsedSeconds = (now - lastCountdownMillis) / 1000UL;

    if (elapsedSeconds == 0)
        return;

    lastCountdownMillis += elapsedSeconds * 1000UL;

    if (elapsedSeconds >= vmc.boostRemainingSeconds)
    {
        Vmc_stop();
        return;
    }

    vmc.boostRemainingSeconds -= elapsedSeconds;
}

//==============================================================================
// Modalità manuale
//==============================================================================

void Vmc_setManualSpeed(uint8_t speed)
{
    if (vmc.mode == VmcMode::BOOST)
        return;

    if (speed == 0)
    {
        Vmc_stop();
        return;
    }

    if (speed > 7)
        return;

    Fans_setSpeed(speed);
    vmc.mode = VmcMode::MANUALE;
    vmc.speed = speed;
    vmc.boostRemainingSeconds = 0;
}

//==============================================================================
// Modalità Boost
//==============================================================================

void Vmc_startBoost()
{
    if (vmc.mode == VmcMode::BOOST)
        Vmc_update();

    const bool startNewCountdown = vmc.mode != VmcMode::BOOST;

    const uint32_t boostSeconds = static_cast<uint32_t>(BOOST_TIME_MINUTES) * 60UL;

    if (UINT32_MAX - vmc.boostRemainingSeconds < boostSeconds)
        vmc.boostRemainingSeconds = UINT32_MAX;
    else
        vmc.boostRemainingSeconds += boostSeconds;

    Fans_setSpeed(7);
    vmc.mode = VmcMode::BOOST;
    vmc.speed = 7;

    if (startNewCountdown)
        lastCountdownMillis = millis();
}

void Vmc_cancelBoost()
{
    if (vmc.mode == VmcMode::BOOST)
        Vmc_stop();
}

//==============================================================================
// Dati pubblici
//==============================================================================

const VmcData& Vmc_getData()
{
    return vmc;
}
