#ifndef MPS_H_
#define MPS_H_

#include <stdint.h>

#include "MpsConfig.h"
#include "MpsMalloc.h"
#include "MpsHandles.h"
#include "MpsResult.h"
#include "MpsUtil.h"
#include "MpsPacket.h"
#include "MpsQueue.h"
#include "MpsLayer.h"

#define MPS_ERROR_ID              0x1000

#define MPS_ERROR_NO_BUFFER_SPACE (MPS_ERROR_ID | 0x0001)   /* No buffer space left. */
#define MPS_ERROR_NO_MEMORY       (MPS_ERROR_ID | 0x0002)   /* No memory left. */
#define MPS_ERROR_INVALID_COMP    (MPS_ERROR_ID | 0x0003)   /* Invalid packet composition. */
#define MPS_ERROR_TRANSFER        (MPS_ERROR_ID | 0x0004)   /* Error during layer-to-layer packet transfer. */

struct Mps {
    MpsQueueHandle_t *queues;	/* Series of queues. Dynamically allocated at initialization. */
    uint8_t size;				/* Actual stack size; 0 < size <= MPS_CONFIG_STACK_MAX_SIZE. */
    MpsLayerHandle_t layers[MPS_CONFIG_STACK_MAX_SIZE]; /* Stack layers. */
};

/* Initializes the stack with given size and creates the queues.
 * All protocol layers should be placed in the layers array before
 * calling MpsInit.
 * MpsInit calls the Init functions of all layers if defined.
 * Note: The MPS struct should be declared globally or a similar
 * preserved location. */
MpsResult_t MpsInit(MpsHandle_t stack, uint8_t size);

/* Deletes all packets, then all queues and de-initializes
 * the stack layers. */
MpsResult_t MpsDeinit(MpsHandle_t stack);

MpsResult_t MpsStart(MpsHandle_t stack);

MpsResult_t MpsStop(MpsHandle_t stack);

/* Return the current amount of memory in use, in bytes. */
uint32_t MpsMemoryUsageGet(void);

/* Returns the maximum amount of memory used, in bytes, since the last reset. */
uint32_t MpsMemoryUsageHighMarkGet(void);

/* Returns the current top layer. */
MpsLayerHandle_t MpsTopGet(MpsHandle_t stack);

/* Returns the current bottom layer. */
MpsLayerHandle_t MpsBottomGet(MpsHandle_t stack);

/* Calls all run-loop functions registered by layers. */
MpsResult_t MpsRunLoop(void);



/* Callbacks */
void MpsOnError(uint8_t error);


#endif
