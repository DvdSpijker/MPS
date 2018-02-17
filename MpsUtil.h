/*
 * MpsUtil.h
 *
 * Created: 2-10-2017 18:16:00
 *  Author: Dorus
 */ 


#ifndef MPS_UTIL_H_
#define MPS_UTIL_H_


#include <stdint.h>


/* Simple hashing function based on djb2. */
uint32_t MpsUtilHash(uint8_t *data, uint32_t size);

uint32_t MpsUtilRunTimeHoursGet(void);

uint32_t MpsUtilRunTimeMicrosGet(void);

/* Generates hash-value using a hashing function
 * on the current run-time. */
uint32_t MpsUtilRunTimeHashGenerate(void);


void MpsPacketDump(uint8_t *packet, uint32_t size);

#endif /* MPS_UTIL_H_ */