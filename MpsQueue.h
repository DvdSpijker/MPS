#ifndef MPS_QUEUE_H_
#define MPS_QUEUE_H_

#include "MpsHandles.h"
#include "MpsResult.h"

#include <stdint.h>

struct MpsQueue {
    MpsBufferHandle_t head;
    MpsBufferHandle_t tail;
    uint16_t size;
};

/* Creates a queue. */
MpsQueueHandle_t MpsQueueCreate(void);

/* Deletes the queue. */
void MpsQueueDelete(MpsQueueHandle_t queue);

/* Returns the size of the queue. */
uint16_t MpsQueueSizeGet(MpsQueueHandle_t queue);

/* Adds a MpsBuffer to the tail of the queue. */
MpsResult_t MpsQueuePush(MpsQueueHandle_t queue, MpsBufferHandle_t buffer);

/* Removes a MpsBuffer from the head the queue. */
MpsBufferHandle_t MpsQueuePop(MpsQueueHandle_t queue);

/* TODO: Remove specific buffer from the queue. */
MpsResult_t MpsQueueRemove(MpsQueueHandle_t queue, MpsBufferHandle_t buffer);

#endif