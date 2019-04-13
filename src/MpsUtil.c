/*
* MpsUtil.c
*
* Created: 2-10-2017 18:16:09
*  Author: Dorus
*/

#include "MpsUtil.h"
#include "MpsConfig.h"
#include "MpsPort.h"

#include <stdio.h>

uint32_t MpsUtilHash(uint8_t *data, uint32_t size)
{
    uint32_t hash = 5381;
    
    for (uint32_t i = 0; i < size; i++) {
        hash = ((hash << 5) + hash) + data[i]; /* hash * 33 + c */
    }
    
    return hash;
}

uint32_t MpsUtilRunTimeHoursGet(void)
{
	uint32_t hours = MpsPortTimeHours();

    return hours;
}
uint32_t MpsUtilRunTimeMillisGet(void)
{
	uint32_t millis = MpsPortTimeMillis();
    return millis;    
}

uint32_t MpsUtilRunTimeHashGenerate(void)
{
    uint32_t hash;
    uint32_t run_time[2];
    run_time[0] = MpsUtilRunTimeHoursGet();
    run_time[1] = MpsUtilRunTimeMillisGet();
    
    hash = MpsUtilHash((uint8_t *)run_time, sizeof(run_time));
    
    return hash;
}
