
#include "MpsQueue.h"
#include "MpsBuffer.h"
#include "MpsMalloc.h"

#include <stdlib.h>


/* Creates a queue. */
MpsQueueHandle_t MpsQueueCreate(void)
{
    MpsQueueHandle_t queue = (MpsQueueHandle_t)MpsMalloc(sizeof(struct MpsQueue));

    if(queue != NULL) {
        queue->head = queue->tail = NULL;
        queue->size = 0;
    }

    return queue;
}

void MpsQueueDelete(MpsQueueHandle_t queue)
{
    MpsFree((void *)queue);
}

uint16_t MpsQueueSizeGet(MpsQueueHandle_t queue)
{
    return queue->size;
}

/* Adds a MpsBuffer to the tail of the queue. */
MpsResult_t MpsQueuePush(MpsQueueHandle_t queue, MpsBufferHandle_t buffer)
{
    MpsResult_t result = MPS_RESULT_OK;
    
    if(queue->size == 0XFFFF) {
        result = MPS_RESULT_NO_SPACE; /* Return error. */
    }
    
    if(result == MPS_RESULT_OK) {
        
        if(queue->size == 0) {
            queue->head = queue->tail = buffer;
            } else {
            queue->tail->next_in_queue = buffer;
            queue->tail = buffer;
        }
        buffer->next_in_queue = NULL;
        queue->size++;
    }
    
    return result;
}

/* Removes a MpsBuffer from the head the queue. */
MpsBufferHandle_t MpsQueuePop(MpsQueueHandle_t queue)
{
    MpsBufferHandle_t buffer = NULL;
    
    if(queue->size == 0) {
        return NULL;
        } else if(queue->size == 1) {
        buffer = queue->head;
        queue->head = queue->tail = NULL;
        } else {
        buffer = queue->head;
        queue->head = buffer->next_in_queue;
        buffer->next_in_queue = NULL;
    }

    queue->size--;

    return buffer;
}
