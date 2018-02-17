#ifndef MPS_MALLOC_H_
#define MPS_MALLOC_H_

#include <stdint.h>

void MpsMallocInit(uint32_t mem_size);

void MpsMallocDeinit(void);

void *MpsMalloc(uint32_t size);

void MpsFree(void *ptr);

/* Not available when not running Prior RTOS. */
uint32_t MpsMallocMemoryUsageGet(void);

uint32_t MpsMallocMemoryUsageHighMarkGet(void);

#endif