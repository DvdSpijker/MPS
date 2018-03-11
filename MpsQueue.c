
#include "MpsQueue.h"
#include "MpsPacket.h"
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

/* Adds a MpsPacket to the tail of the queue. */
MpsResult_t MpsQueuePush(MpsQueueHandle_t queue, MpsPacketHandle_t packet)
{
    MpsResult_t result = MPS_RESULT_OK;
    
    if(queue->size == 0XFFFF) {
        result = MPS_RESULT_NO_SPACE; /* Return error. */
    }
    
    if(result == MPS_RESULT_OK) {
        if(queue->size == 0) {
            queue->head = queue->tail = packet;
		} else {
            queue->tail->next_in_queue = packet;
            queue->tail = packet;
        }
        packet->next_in_queue = NULL;
        queue->size++;
    }
    
    return result;
}

/* Removes a MpsPacket from the head the queue. */
MpsPacketHandle_t MpsQueuePop(MpsQueueHandle_t queue)
{
    MpsPacketHandle_t packet = NULL;
    
    if(queue->size == 0) {
        return NULL;
	} else if(queue->size == 1) {
        packet = queue->head;
        queue->head = queue->tail = NULL;
	} else {
        packet = queue->head;
        queue->head = packet->next_in_queue;
        packet->next_in_queue = NULL;
    }

    queue->size--;

    return packet;
}

MpsResult_t MpsQueueRemove(MpsQueueHandle_t queue, MpsPacketHandle_t packet)
{	
	MpsResult_t result = MPS_RESULT_ERROR;
	uint16_t i = 0;
	MpsPacketHandle_t it_packet = queue->head;
	MpsPacketHandle_t it_packet_prev = NULL;
	while(it_packet != packet && it_packet != NULL && i < queue->size) {
		it_packet_prev = it_packet;
		it_packet = it_packet->next_in_queue;
	}
	
	if(it_packet == packet) {
		it_packet_prev->next_in_queue = it_packet->next_in_queue;
		it_packet->next_in_queue = NULL;
		MpsResult_t result = MPS_RESULT_OK;
	}
	return result;
}