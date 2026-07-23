/******************************************************************************
 * Fans Module
 ******************************************************************************/

#ifndef FANS_H
#define FANS_H

#include <stdint.h>

struct FanData
{
    uint8_t inPercent;
    uint8_t outPercent;
};

bool Fans_begin();

void Fans_setIn(uint8_t percent);
void Fans_setOut(uint8_t percent);

uint8_t Fans_getIn();
uint8_t Fans_getOut();
const FanData& Fans_getData();

void Fans_stopAll();

#endif