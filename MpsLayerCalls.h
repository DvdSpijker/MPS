#ifndef MPS_LAYER_CALLS_H_
#define MPS_LAYER_CALLS_H_

#include "MpsResult.h"

#include <stdint.h>

typedef MpsResult_t  (*MpsLayerCallGeneric_t)(void);

typedef uint32_t (*MpsLayerCallTransferRawData_t)(uint8_t *data, uint32_t size);

typedef MpsResult_t (*MpsLayerCallRtxRequest_t)(MpsPacketHandle_t packet);

typedef MpsResult_t (*MpsLayerCallLbRequest_t)(MpsPacketHandle_t packet, MpsLayerHandle_t layer);

typedef void (*MpsLayerCallError_t)(MpsLayerHandle_t source_layer, MpsPacketHandle_t packet, uint16_t error);

#endif