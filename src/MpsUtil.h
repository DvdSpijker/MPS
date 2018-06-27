#ifndef MPS_UTIL_H_
#define MPS_UTIL_H_

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>


/* Simple hashing function based on djb2. */
uint32_t MpsUtilHash(uint8_t *data, uint32_t size);

uint32_t MpsUtilRunTimeHoursGet(void);

uint32_t MpsUtilRunTimeMillisGet(void);

/* Generates hash-value using a hashing function
 * on the current run-time. */
uint32_t MpsUtilRunTimeHashGenerate(void);


void MpsPacketDump(uint8_t *packet, uint32_t size);

#ifdef __cplusplus 
}
#endif


#endif /* MPS_UTIL_H_ */