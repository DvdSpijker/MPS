#ifndef MPS_BUFFER_H_
#define MPS_BUFFER_H_

#include "MpsResult.h"
#include "MpsHandles.h"
#include <stdint.h>

typedef MpsBufSize_t uint32_t;

struct MpsBuffer {
    uint32_t id;
	
    uint8_t *buffer;		/* Pointer to the buffer. */
    uint8_t is_external;	/* Buffer is declared externally i.e. not allocated by MPS. */
    uint8_t type;		/* Event or Data type. See MPS_BUFFER_TYPE_* below. */
    MpsBufSize_t capacity;	/* Buffer capacity in bytes. */
    MpsBufSize_t size;		/* Current buffer size in bytes. */
    uint16_t offset;		/* Current offset with respect to the buffer index 0. */
    void    *layer_specific; /* Optional pointer for layer specific data-structures. */

    struct MpsBuffer *next_in_queue;	/* Used by MpsQueue for linking. */
};

/* Buffer-type definitions. */
/* A buffer containing an event is emitted by the layer itself. Expected
 * behavior of layers above or below the emitting layer is to pass the buffer along
 * to the top or bottom layer. */
#define MPS_BUFFER_TYPE_EVENT	0x01 

/* A buffer containing a packet can be modified by layers as it propagates
 * through the stack. Layers can add/remove sections of the buffer e.g. a header. */
#define MPS_BUFFER_TYPE_PACKET	0x02

/* Creates a MspBuffer of given size and sets the layer specific implementation. */
MpsBufferHandle_t MpsBufferCreate(uint8_t *buffer, MpsBufSize_t size, uint8_t type, void *layer_specific);

void MpsBufferDelete(MpsBufferHandle_t buffer);

/* Resizes the buffer while retaining the data. */
MpsBufferHandle_t MpsBufferResize(MpsBufferHandle_t buffer, MpsBufSize_t new_size);

/* TODO: Returns a copy of the buffer. */
MpsBufferHandle_t MpsBufferCopy(MpsBufferHandle_t buffer);

/* Returns the size of all regions combined. */
MpsBufSize_t MpsBufferSizeGet(MpsBufferHandle_t buffer);

/* Adds data to the header region after merging a possible previous header with the current body.
 * After the operation the header size will be the added header size.
 * Header and body offsets are adjusted. */
MpsResult_t MpsBufferAddHeader(MpsBufferHandle_t buffer, uint8_t *data, MpsBufSize_t size);


void MpsBufferAddBody(MpsBufferHandle_t buffer, uint8_t *data, MpsBufSize_t size);

/* Adds data to the trailer region after merging a possible previous trailer with the current body.
 * After the operation the trailer size will be the added trailer size.
 * The trailer offset is adjusted. */
MpsResult_t MpsBufferAddTrailer(MpsBufferHandle_t buffer, uint8_t *data, MpsBufSize_t size);

/* Adds data to the specified region. Copying from 'data' starts at the 'data_offset'
 * and is copied to the body at the 'body_offset'. */
void MpsBufferAddBodyWithOffset(MpsBufferHandle_t buffer, uint8_t *data, MpsBufSize_t data_offset, MpsBufSize_t size, MpsBufSize_t body_offset);

/* Extracts a header of defined size from the current body. After the
 * operation the header region size will be the extracted header size.
 * Header and body offsets are adjusted. */
MpsResult_t MpsBufferExtractHeader(MpsBufferHandle_t buffer, uint8_t *data, MpsBufSize_t size);

/* Extracts a body of defined size from the current body. After the
 * operation the body region size will decreased by the extracted body size.
 * The body offset is adjusted.
 * Note: If 0 is passed as the size, the entire body will be extracted. */
MpsResult_t MpsBufferExtractBody(MpsBufferHandle_t buffer, uint8_t *data);

/* Extracts a trailer of defined size from the current body. After the
 * operation the trailer region size will be the extracted trailer size.
 * The trailer offset is adjusted. */
MpsResult_t MpsBufferExtractTrailer(MpsBufferHandle_t buffer, uint8_t *data, MpsBufSize_t size);

uint8_t MpsBufferTypeGet(MpsBufferHandle_t buffer);

/* Write data directly into the buffer */
uint8_t *MpsBufferWriteDirect(MpsBufferHandle_t buffer, uint16_t offset, MpsBufSize_t size);

/* Returns a pointer to the data in the buffer with the current offset taken into account. */
uint8_t *MpsBufferDataPointerGet(MpsBufferHandle_t buffer);

/* Zeros the buffer and resets all regions. */
MpsResult_t MpsBufferFlush(MpsBufferHandle_t buffer);

void MpsBufferDump(MpsBufferHandle_t buffer);
#endif
