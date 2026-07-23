/******************************************************************************
 * Climate Module
 ******************************************************************************/

#ifndef CLIMATE_H
#define CLIMATE_H

struct SensorData
{
    float intTemp;
    float intHum;

    float extTemp;
    float extHum;

    float pressure;
};

bool Climate_begin();
bool Climate_read(SensorData &data);
const SensorData& Climate_getData();

bool Climate_isSHT31_OK();
bool Climate_isBME280_OK();

#endif