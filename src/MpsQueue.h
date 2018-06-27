#ifndef MPS_QUEUE_H_
#define MPS_QUEUE_H_

#ifdef __cplusplus 
extern "C" {
#endif

#include "MpsHandles.h"
#include "MpsResult.h"

#include <stdint.h>

struct MpsQueue {
    MpsPacketHandle_t head;
    MpsPacketHandle_t tail;
    uint16_t size;
};

/* Creates a queue. */
MpsQueueHandle_t MpsQueueCreate(void);

/* Deletes the queue. */
void MpsQueueDelete(MpsQueueHandle_t queue);

/* Returns the size of the queue. */
uint16_t MpsQueueSizeGet(MpsQueueHandle_t queue);

/* Adds a MpsPacket to the tail of the queue. */
MpsResult_t MpsQueuePush(MpsQueueHandle_t queue, MpsPacketHandle_t packet);

/* Removes a MpsPacket from the head the queue. */
MpsPacketHandle_t MpsQueuePop(MpsQueueHandle_t queue);

/* TODO: Remove specific packet from the queue. */
MpsResult_t MpsQueueRemove(MpsQueueHandle_t queue, MpsPacketHandle_t packet);

#ifdef __cplusplus 
}
#endif


#endif