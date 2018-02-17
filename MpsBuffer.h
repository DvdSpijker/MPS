#ifndef MPS_BUFFER_H_
#define MPS_BUFFER_H_

#include "MpsResult.h"
#include "MpsHandles.h"
#include <stdint.h>


struct MpsBuffer {
    uint32_t id;

    uint8_t is_external;
    uint8_t *buffer;
    uint16_t capacity;
    uint16_t size;
    uint16_t offset;
    void    *layer_specific; /* Optional pointer for layer specific data-structures. */

    struct MpsBuffer *next_in_queue;
};

/* Creates a MspBuffer of given size and sets the layer specific implementation. */
MpsBufferHandle_t MpsBufferCreate(uint8_t *buffer, uint16_t size, void *layer_specific);

void MpsBufferDelete(MpsBufferHandle_t buffer);

/* Resizes the buffer while retaining the data. */
MpsBufferHandle_t MpsBufferResize(MpsBufferHandle_t buffer, uint16_t new_size);

/* TODO: Returns a copy of the buffer. */
MpsBufferHandle_t MpsBufferCopy(MpsBufferHandle_t buffer);

/* Returns the size of all regions combined. */
uint16_t MpsBufferSizeGet(MpsBufferHandle_t buffer);

/* Adds data to the header region after merging a possible previous header with the current body.
 * After the operation the header size will be the added header size.
 * Header and body offsets are adjusted. */
MpsResult_t MpsBufferAddHeader(MpsBufferHandle_t buffer, uint8_t *data, uint16_t size);


void MpsBufferAddBody(MpsBufferHandle_t buffer, uint8_t *data, uint16_t size);

/* Adds data to the trailer region after merging a possible previous trailer with the current body.
 * After the operation the trailer size will be the added trailer size.
 * The trailer offset is adjusted. */
MpsResult_t MpsBufferAddTrailer(MpsBufferHandle_t buffer, uint8_t *data, uint16_t size);

/* Adds data to the specified region. Copying from 'data' starts at the 'data_offset'
 * and is copied to the body at the 'body_offset'. */
void MpsBufferAddBodyWithOffset(MpsBufferHandle_t buffer, uint8_t *data, uint16_t data_offset, uint16_t size, uint16_t body_offset);

/* Extracts a header of defined size from the current body. After the
 * operation the header region size will be the extracted header size.
 * Header and body offsets are adjusted. */
MpsResult_t MpsBufferExtractHeader(MpsBufferHandle_t buffer, uint8_t *data, uint16_t size);

/* Extracts a body of defined size from the current body. After the
 * operation the body region size will decreased by the extracted body size.
 * The body offset is adjusted.
 * Note: If 0 is passed as the size, the entire body will be extracted. */
MpsResult_t MpsBufferExtractBody(MpsBufferHandle_t buffer, uint8_t *data);

/* Extracts a trailer of defined size from the current body. After the
 * operation the trailer region size will be the extracted trailer size.
 * The trailer offset is adjusted. */
MpsResult_t MpsBufferExtractTrailer(MpsBufferHandle_t buffer, uint8_t *data, uint16_t size);


uint8_t *MpsBufferWriteDirect(MpsBufferHandle_t buffer, uint16_t offset, uint16_t size);

uint8_t *MpsBufferDataPointerGet(MpsBufferHandle_t buffer);

/* Zeroes the buffer and resets all regions. */
MpsResult_t MpsBufferFlush(MpsBufferHandle_t buffer);

void MpsBufferDump(MpsBufferHandle_t buffer);
#endif