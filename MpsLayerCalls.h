#ifndef MPS_LAYER_CALLS_H_
#define MPS_LAYER_CALLS_H_

#include "MpsResult.h"

#include <stdint.h>

typedef MpsResult_t  (*MpsLayerCallGeneric_t)(void);

typedef uint32_t (*MpsLayerCallTransferRawData_t)(uint8_t *data, uint32_t size);

typedef MpsResult_t (*MpsLayerCallRtxRequest_t)(MpsBufferHandle_t buffer);

typedef MpsResult_t (*MpsLayerCallLbRequest_t)(MpsBufferHandle_t buffer, MpsLayerHandle_t layer);

typedef void (*MpsLayerCallError_t)(MpsLayerHandle_t source_layer, MpsBufferHandle_t buffer, uint16_t error);

#endif