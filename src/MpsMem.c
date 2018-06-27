#include "MpsConfig.h"
#include "MpsMem.h"

#include <stdlib.h>

#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
#include <PriorRTOS.h>
Id_t MpsPool = OS_ID_INVALID;
#else
uint32_t MemSize = 0;
uint32_t MemLeft = 0;
#endif
uint32_t MemUsageHighMark = 0;

void MpsMemInit(uint32_t mem_size)
{
#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
    MpsPool = MemPoolCreate(mem_size);
#else
    MemSize = MemLeft = mem_size;
#endif
}

void *MpsMemAlloc(uint32_t size)
{
    void *ptr = NULL;
#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
    ptr = MemAlloc(MpsPool, size);
    if(MemPoolUsedSpaceGet(MpsPool) > MemUsageHighMark) {
        MemUsageHighMark = MemPoolUsedSpaceGet(MpsPool);
    }
    return ptr;
#else
    //if(MpsMemLeft < size) {
    //return NULL;
    //}
    //MpsMemLeft -= size;
    //if((MpsMemSize - MpsMemLeft) > MemUsageHighMark) {
    //MemUsageHighMark = (MpsMemSize - MpsMemLeft);
    //}
    return malloc(size);
#endif


}


void MpsMemFree(void *ptr)
{
#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
    MemFree((void **)&ptr);
#else

    free(ptr);
#endif
}


void MpsMemDeinit(void)
{
#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
    MemPoolDelete(MpsPool);
#endif
}

uint32_t MpsMemUsageGet(void)
{
#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
    return MemPoolUsedSpaceGet(MpsPool);

#else
    return (MemSize - MemLeft);
#endif

}

uint32_t MpsMemUsageHighGet(void)
{
    return MemUsageHighMark;
}