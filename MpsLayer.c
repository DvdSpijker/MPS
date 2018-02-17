/*
* MpsLayer.c
*
* Created: 2-10-2017 18:04:14
*  Author: Dorus
*/

#include "MpsLayer.h"

#include <stdlib.h>

MpsResult_t MpsLayerInit(MpsLayerHandle_t layer)
{
    if(layer == NULL) {
        return MPS_RESULT_INVALID_ARG;
    }
    layer->state = LAYER_STATE_INITIALIZED;
    layer->layer_above = NULL;
    layer->layer_below = NULL;
    return MPS_RESULT_OK;
}

MpsResult_t MpsLayerDeinit(MpsLayerHandle_t layer)
{
    if(layer == NULL) {
        return MPS_RESULT_INVALID_ARG;
    }
    layer->state = LAYER_STATE_UNINITIALIZED;
    return MPS_RESULT_OK;
}

void MpsLayerReportError(MpsLayerHandle_t src_layer, MpsLayerHandle_t dest_layer, uint16_t error)
{
    if(src_layer != NULL && dest_layer != NULL) {
        if(dest_layer->error != NULL && dest_layer->state < LAYER_STATE_INVALID) {
            dest_layer->error(src_layer, error);
        }
    }
}