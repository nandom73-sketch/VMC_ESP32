/******************************************************************************
 * VMC Logic Module
 ******************************************************************************/

#ifndef VMC_H
#define VMC_H

#include <stdint.h>

enum class VmcMode
{
    OFF,
    MANUALE,
    BOOST
};

struct VmcData
{
    VmcMode mode;
    uint8_t speed;
    uint32_t boostRemainingSeconds;
};

bool Vmc_begin();
void Vmc_update();

void Vmc_setManualSpeed(uint8_t speed);

void Vmc_startBoost();
void Vmc_cancelBoost();

const VmcData& Vmc_getData();

#endif
