#include "Mps.h"
#include "MpsMalloc.h"


#include <stdlib.h>
#include <math.h>

MpsResult_t MpsInit(MpsHandle_t stack, uint8_t size)
{
    if(stack == NULL || size == 0) {
        return MPS_RESULT_INVALID_ARG;
    }

    MpsResult_t result = MPS_RESULT_NO_MEM;
    stack->size = size;
    MpsMallocInit(MPS_CONFIG_MEMORY_SIZE_BYTES);

    /* Allocate an array of queue handles and create all ((N-1) * 2) queues. */
    uint16_t n_queues = (size-1) * 2;
    stack->queues = (MpsQueueHandle_t *)MpsMalloc(sizeof(MpsQueueHandle_t) * n_queues); /* Array of Queue handles. */
    if(stack->queues != NULL) {
        for (uint16_t i = 0; i < n_queues; i++) {
            stack->queues[i] = MpsQueueCreate();

            /* Break down all allocated queues if 1 fails. */
            if(stack->queues[i] == NULL) {
                for (uint16_t j = 0; j < i; j++) {
                    MpsFree((void*)stack->queues[j]);
                }
                result = MPS_RESULT_NO_MEM; /* Return out of memory error. */
                goto exit;
            }

        }

        /* Attach queues to their respective layers. */
        if(size > 1) {
            for (uint8_t k = 0; k < size; k++) {
                MpsLayerInit(stack->layers[k]);
                
                if(k == 0) {
                    /* Top layer does not have Tx in or Rx out queues. */
                    stack->layers[k]->layer_above = NULL;
                    stack->layers[k]->layer_below = stack->layers[k+1];
                    stack->layers[k]->tx_queue = NULL;
                    stack->layers[k]->rx_queue = stack->queues[1];
                } else if (k == size-1) {
                    /* Bottom layer does not have Tx out or Rx in queues. */
                    stack->layers[k]->layer_above = stack->layers[k-1];
                    stack->layers[k]->layer_below = NULL;
                    stack->layers[k]->tx_queue = stack->queues[k];
                    stack->layers[k]->rx_queue = NULL;
                } else {
                    /* Layers in the middle get all queues. */
                    stack->layers[k]->layer_above = stack->layers[k-1];
                    stack->layers[k]->layer_below = stack->layers[k+1];
                    stack->layers[k]->tx_queue = stack->queues[k-1];
                    stack->layers[k]->rx_queue = stack->queues[k+2];
                }

                if(stack->layers[k]->init != NULL) {
                    stack->layers[k]->init();
                }
            }
        }
        result = MPS_RESULT_OK;
    }


    exit:
    return result;

}


MpsResult_t MpsDeinit(MpsHandle_t stack)
{
    if(stack == NULL) {
        return MPS_RESULT_INVALID_ARG;
    }

    MpsBufferHandle_t buf = NULL;
    uint16_t n_queues = (stack->size-1) * 2;

    /* First calls all deinit functions of all layers (if defined).
    * Then deinit all layer structs. */
    for (uint8_t k = 0; k < stack->size; k++) {

        if(stack->layers[k]->deinit != NULL) {
            stack->layers[k]->deinit();
        }

        /* Call MpsLayerDeinit here. */
        stack->layers[k]->tx_queue = NULL;
        stack->layers[k]->rx_queue = NULL;
    }

    for (uint16_t i = 0; i < n_queues; i++) {
        /* Iterate through the queue, then remove all
        * buffers and delete them. */
        for (uint16_t j = 0; j < stack->queues[i]->size; j++) {
            buf = MpsQueuePop(stack->queues[i]);
            if(buf != NULL) {
                MpsBufferDelete(buf);
                } else {
                break;
            }
        }

        MpsQueueDelete(stack->queues[i]); /* When the queue is empty delete it. */
    }

    MpsFree((void *)stack->queues); /* Free the queue array. */

    MpsMallocDeinit(); /* When all objects are freed, deinit MpsMalloc. */

    return MPS_RESULT_OK;
}

MpsResult_t MpsStart(MpsHandle_t stack)
{
    if(stack == NULL) {
        return MPS_RESULT_INVALID_ARG;
    }

    for (uint8_t i = 0; i < stack->size; i++) {
        if(stack->layers[i]->start != NULL) {
            stack->layers[i]->start();
        }
    }

    return MPS_RESULT_OK;
}

MpsResult_t MpsStop(MpsHandle_t stack)
{
    if(stack == NULL) {
        return MPS_RESULT_INVALID_ARG;
    }

    for (uint8_t i = 0; i < stack->size; i++) {
        if(stack->layers[i]->stop != NULL) {
            stack->layers[i]->stop();
        }
    }

    return MPS_RESULT_OK;
}

uint32_t MpsMemoryUsageGet(void)
{
    #ifdef MPS_CONFIG_USE_PRIOR_RTOS
    return MpsMallocMemoryUsageGet();
    #endif
    return 0;
}

uint32_t MpsMemoryUsageHighMarkGet(void)
{
    return MpsMallocMemoryUsageHighMarkGet();
}

MpsResult_t MpsRunLoop(MpsHandle_t stack)
{
	MpsResult_t result = MPS_RESULT_ERROR;
#ifdef MPS_CONFIG_RUNLOOP_ORDER_TOP_TO_BOTTOM
	/* Calls run-loop functions in top to bottom order.  */
	for (uint8_t i = 0; i < stack->size; i++){
		if(stack->layers->runloop != NULL) {
			result = stack->layers->runloop(); /* Call layer's run-loop function. */
			if(result < MPS_RESULT_OK) { /* If result is any kind of error (< 0). */
				return result;
			}	
		}
	}
#else /*MPS_CONFIG_RUNLOOP_CALL_ORDER_BOTTOM_TO_TOP*/
	/* Calls run-loop functions in bottom to top order.  */
	for (uint8_t i = (stack->size - 1); i > 0; i--){
		if(stack->layers->runloop != NULL) {
			result = stack->layers->runloop(); /* Call layer's run-loop function. */
			if(result < MPS_RESULT_OK) { /* If result is any kind of error (< 0). */
				return result;
			}
		}
	}
#endif
	return result;
}