#ifndef MPS_LAYER_H_
#define MPS_LAYER_H_

#include "MpsHandles.h"
#include "MpsResult.h"
#include "MpsLayerCalls.h"

enum MpsLayerState {
    LAYER_STATE_UNUSED,
    LAYER_STATE_UNINITIALIZED,
    LAYER_STATE_INITIALIZED,
    LAYER_STATE_OPERATIONAL,
    LAYER_STATE_BUSY,
    LAYER_STATE_ERROR,
    LAYER_STATE_INVALID,
};

struct MpsLayer {
    /* Read-only. */
    enum MpsLayerState state;
    MpsQueueHandle_t tx_queue;
    MpsQueueHandle_t rx_queue;

    MpsLayerHandle_t layer_above;
    MpsLayerHandle_t layer_below;

    uint16_t header_size;
    uint16_t trailer_size;

    /* Set by user before MpsInit.
     * After that read-only. */
    void                 *module;
    MpsLayerCallGeneric_t init;
    MpsLayerCallGeneric_t deinit;
    MpsLayerCallGeneric_t start;
    MpsLayerCallGeneric_t stop;
    MpsLayerCallError_t   error;

    MpsLayerCallRtxRequest_t transmit_req;
    MpsLayerCallRtxRequest_t receive_req;
    MpsLayerCallLbRequest_t loopback_req;

};

MpsResult_t MpsLayerInit(MpsLayerHandle_t layer);
MpsResult_t MpsLayerDeinit(MpsLayerHandle_t layer);
void MpsLayerReportError(MpsLayerHandle_t src_layer, MpsLayerHandle_t dest_layer, uint16_t error);


#define MPS_LAYER_DEF(src_layer)                        \
static const MpsLayerHandle_t SrcLayerDef = src_layer;  \

#define MPS_LAYER_ASSERT(cond, err)                                     \
if(!(cond)) {                                                           \
    MpsLayerReportError(SrcLayerDef, SrcLayerDef->layer_above, err);    \
    goto assert_fail;                                                   \
}                                                                       \



#define MPS_LAYER_ASSERT_FAIL_HANDLER   \
assert_fail:                            \

#endif