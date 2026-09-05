/******************************************************************************
 * Fans Module
 ******************************************************************************/

#include "Fans.h"
#include "Config.h"

//==============================================================================
// Variabili private
//==============================================================================

static FanData fans =
{
    0,  // inPercent
    0,  // outPercent
    0   // speed
};

static void Fans_updateSpeed()
{
    fans.speed = 0;

    if (fans.inPercent != fans.outPercent)
        return;

    for (uint8_t index = 0; index < 7; ++index)
    {
        if (fans.inPercent == FAN_SPEED_PERCENT[index])
        {
            fans.speed = index + 1;
            return;
        }
    }
}

//==============================================================================
// Inizializzazione
//==============================================================================

bool Fans_begin()
{
    fans.inPercent = 0;
    fans.outPercent = 0;
    fans.speed = 0;

    return true;
}

//==============================================================================
// Imposta velocità ventola ingresso
//==============================================================================

void Fans_setIn(uint8_t percent)
{
    if (percent > 100)
        percent = 100;

    fans.inPercent = percent;
}

//==============================================================================
// Imposta velocità ventola uscita
//==============================================================================

void Fans_setOut(uint8_t percent)
{
    if (percent > 100)
        percent = 100;

    fans.outPercent = percent;
}

//==============================================================================
// Imposta lo stesso livello di ventilazione su entrambe le uscite
//==============================================================================

void Fans_setSpeed(uint8_t speed)
{
    if (speed == 0)
    {
        Fans_stopAll();
        return;
    }

    if (speed > 7)
        return;

    const uint8_t percent = FAN_SPEED_PERCENT[speed - 1];

    fans.inPercent = percent;
    fans.outPercent = percent;
    fans.speed = speed;

}

//==============================================================================
// Legge velocità ventola ingresso
//==============================================================================

uint8_t Fans_getIn()
{
    return fans.inPercent;
}

//==============================================================================
// Legge velocità ventola uscita
//==============================================================================

uint8_t Fans_getOut()
{
    return fans.outPercent;
}

//==============================================================================
// Legge stato ventole
//==============================================================================

const FanData& Fans_getData()
{
    return fans;
}

//==============================================================================
// Arresta entrambe le ventole
//==============================================================================

void Fans_stopAll()
{
    fans.inPercent = 0;
    fans.outPercent = 0;
    fans.speed = 0;

}
