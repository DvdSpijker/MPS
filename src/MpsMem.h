#ifndef MPS_MEM_H_
#define MPS_MEM_H_

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>

void MpsMemInit(uint32_t mem_size);

void MpsMemDeinit(void);

void *MpsMemAlloc(uint32_t size);

void MpsMemFree(void *ptr);

/* Not available when not running Prior RTOS. */
uint32_t MpsMemUsageGet(void);

uint32_t MpsMemUsageHighGet(void);

#ifdef __cplusplus 
}
#endif


#endif