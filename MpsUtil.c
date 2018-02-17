/*
* MpsUtil.c
*
* Created: 2-10-2017 18:16:09
*  Author: Dorus
*/

#include "MpsUtil.h"
#include "MpsConfig.h"

#ifdef MPS_CONFIG_USE_PRIOR_RTOS
#include <PriorRTOS.h>
#endif


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
    uint32_t rt_hours = 0;
    #ifdef MPS_CONFIG_USE_PRIOR_RTOS
    rt_hours = OsRuntimeHoursGet();
    #endif
    return rt_hours;
}
uint32_t MpsUtilRunTimeMicrosGet(void)
{
    uint32_t rt_micros = 0;
    #ifdef MPS_CONFIG_USE_PRIOR_RTOS
    rt_micros = OsRunTimeMicrosGet();
    #endif
    return rt_micros;    
}

uint32_t MpsUtilRunTimeHashGenerate(void)
{
    uint32_t hash;
    uint32_t run_time[2];
    run_time[0] = MpsUtilRunTimeHoursGet();
    run_time[1] = MpsUtilRunTimeMicrosGet();
    
    hash = MpsUtilHash((uint8_t *)run_time, sizeof(run_time));
    
    return hash;
}

void MpsPacketDump(uint8_t *packet, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        if(packet[i] == 0) {
            packet[i] += 48;
        }
    }

    packet[size] = '\0';
    printf("%s", packet);
    
    for (uint32_t i = 0; i < size; i++) {
        if(packet[i] == 48) {
            packet[i] = 0;
        }
    }    
}