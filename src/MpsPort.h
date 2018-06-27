#ifdef MPS_PORT_H_
#define MPS_PORT_H_

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>

void *MpsPortMalloc(uint32_t size);

void MpsPortFree(void *ptr);

uint32_t MpsPortTimeMillis(void);

uint32_t MpsPortTimeHours(void);


#ifdef __cplusplus 
}
#endif

#endif 