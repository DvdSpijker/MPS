#include "MpsConfig.h"
#include "MpsMalloc.h"

#include <stdlib.h>

#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
#include <PriorRTOS.h>
Id_t MpsPool = OS_ID_INVALID;
#else
uint32_t MpsMemSize = 0;
uint32_t MpsMemLeft = 0;
#endif
uint32_t MemUsageHighMark = 0;

void MpsMallocInit(uint32_t mem_size)
{
#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
    MpsPool = MemPoolCreate(mem_size);
#else
    MpsMemSize = MpsMemLeft = mem_size;
#endif
}

void *MpsMalloc(uint32_t size)
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


void MpsFree(void *ptr)
{
#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
    MemFree((void **)&ptr);
#else

    free(ptr);
#endif
}


void MpsMallocDeinit(void)
{
#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
    MemPoolDelete(MpsPool);
#endif
}

uint32_t MpsMallocMemoryUsageGet(void)
{
#if defined(MPS_CONFIG_USE_PRIOR_RTOS) && !defined(MPS_CONFIG_USE_STD_MALLOC)
    return MemPoolUsedSpaceGet(MpsPool);

#else
    return (MpsMemSize - MpsMemLeft);
#endif

}

uint32_t MpsMallocMemoryUsageHighMarkGet(void)
{
    return MemUsageHighMark;
}