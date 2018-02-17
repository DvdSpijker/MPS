
#include "MpsBuffer.h"
#include "MpsMalloc.h"
#include "MpsUtil.h"

#include <stdlib.h>
#include <stdio.h>

MpsBufferHandle_t MpsBufferCreate(uint8_t *buffer, uint16_t size, void *layer_specific)
{
    if(size == 0) {
        return NULL;
    }

    struct MpsBuffer * mps_buf = (struct MpsBuffer *)MpsMalloc(sizeof(struct MpsBuffer));

    if(mps_buf != NULL) {
        if(buffer == NULL) {
            mps_buf->is_external = 0;
            mps_buf->buffer = (uint8_t *)MpsMalloc(sizeof(uint8_t) * size);
        } else {
            mps_buf->is_external = 1;
            mps_buf->buffer = buffer;
        }
        if(mps_buf->buffer != NULL) {
            mps_buf->capacity = size;
            mps_buf->layer_specific = layer_specific;
            mps_buf->next_in_queue = NULL;
            mps_buf->id = MpsUtilRunTimeHashGenerate();
            mps_buf->size = 0;
            mps_buf->offset = 0;
        } else {
            MpsFree((void *)mps_buf);
            mps_buf = NULL;
        }
    }

    return mps_buf;
}


void MpsBufferDelete(MpsBufferHandle_t buffer)
{
    if(buffer == NULL) {
        return; /* Return error here. */
    }

    if(buffer->is_external == 0) {
        MpsFree(buffer->buffer);
    }
    MpsFree(buffer);

    return; /* Return ok here. */
}

MpsBufferHandle_t MpsBufferResize(MpsBufferHandle_t buffer, uint16_t new_size)
{	
	MpsBufferHandle_t new_buf = NULL;
	if(!buffer->is_external) { /* Cannot resize if buffer is external. */
		/* Create a new buffer, copy data and stats, then delete old buffer. */
		new_buf = MpsBufferCreate(NULL, new_size, buffer->layer_specific);
		if(new_buf != NULL) {
			for(uint16_t i = buffer->offset; i < buffer->size, i++) {
				new_buf->buffer[i] = buffer->buffer[i];
			}
			new_buf->offset = buffer->offset;
			new_buf->size = buffer->size;	
			MpsBufferDelete(buffer);
		}
	}
	return new_buf;
}

uint16_t MpsBufferSizeGet(MpsBufferHandle_t buffer)
{
    return (buffer->size);
}


MpsResult_t  MpsBufferAddHeader(MpsBufferHandle_t buffer, uint8_t *data, uint16_t size)
{

    MpsResult_t result = MPS_RESULT_OK;

    if( (size > (buffer->capacity - buffer->size)) || (size > (buffer->offset + 1)) ) {
        result = MPS_RESULT_NO_SPACE;
    }

    if(result == MPS_RESULT_OK) {

        buffer->offset -= size;
        buffer->size += size;

        uint16_t i;
        for(i = 0; i < size; i++) {
            buffer->buffer[i+buffer->offset] = data[i];
        }
    }

    return result;
}

MpsResult_t MpsBufferAddTrailer(MpsBufferHandle_t buffer, uint8_t *data, uint16_t size)
{
    MpsResult_t result = MPS_RESULT_OK;

    if( (size > (buffer->capacity - buffer->size)) || (size > buffer->capacity - (buffer->offset + buffer->size))) {
        result = MPS_RESULT_NO_SPACE;
    }

    if(result == MPS_RESULT_OK) {
        uint16_t offset = buffer->size + buffer->offset;
        buffer->size += size;

        uint16_t i;
        for(i = 0; i < size; i++) {
            buffer->buffer[i+offset] = data[i];
        }
    }

    return result;
}


void MpsBufferAddBodyWithOffset(MpsBufferHandle_t buffer, uint8_t *data, uint16_t data_offset, uint16_t size, uint16_t body_offset)
{
    uint16_t i;
    for(i = 0; i < size; i++) {
        buffer->buffer[i+body_offset] = data[i + data_offset];
    }

    buffer->size = size;
    buffer->offset = body_offset;

    //if( (buffer->size > 0) ) {
    //buffer->size += size - (body_offset + buffer->size);
    //} else {
    //buffer->size = size;
    //}
//
    //if( (buffer->offset > body_offset) || (buffer->offset == 0) ) {
    //buffer->offset = body_offset;
    //}

}




MpsResult_t MpsBufferExtractHeader(MpsBufferHandle_t buffer, uint8_t *data, uint16_t size)
{

    MpsResult_t result = MPS_RESULT_OK;

    /* Check input. */
    if(buffer == NULL || data == NULL || size == 0) {
        result = MPS_RESULT_INVALID_ARG;
    }

    /* Check size boundaries. */
    if(size > buffer->size) {
        result = MPS_RESULT_NO_SPACE;
    }


    if(result == MPS_RESULT_OK) {
        uint16_t i;
        for (i = 0; (i < size); i++) {
            data[i] = buffer->buffer[buffer->offset + i];
        }
        buffer->offset += size;
        buffer->size -= size;

        result = MPS_RESULT_OK;
    }

    return result;
}

MpsResult_t MpsBufferExtractTrailer(MpsBufferHandle_t buffer, uint8_t *data, uint16_t size)
{
    MpsResult_t result = MPS_RESULT_OK;

    if(buffer == NULL || data == NULL || size == 0) {
        result = MPS_RESULT_INVALID_ARG;
    }

    if(size > buffer->size) {
        result = MPS_RESULT_NO_SPACE;
    }
    if(result == MPS_RESULT_OK) {

        uint16_t offset = buffer->offset + (buffer->size - size);
        buffer->size -= size;
        uint16_t i;
        for (i = 0; (i < size); i++) {
            data[i] = buffer->buffer[offset + i];
        }
        result = MPS_RESULT_OK;
    }

    return result;

}

MpsResult_t MpsBufferExtractBody(MpsBufferHandle_t buffer, uint8_t *data)
{
    MpsResult_t result = MPS_RESULT_OK;

    if(buffer == NULL || data == NULL) {
        result = MPS_RESULT_INVALID_ARG;
    }

    /* Extract remaining body. */
    if(result == MPS_RESULT_OK) {

        uint16_t i;
        for (i = 0; (i < buffer->size); i++) {
            data[i] = buffer->buffer[buffer->offset + i];
        }

        buffer->offset = 0;
        buffer->size = 0;

        result = MPS_RESULT_OK;
    }

    return result;
}


uint8_t *MpsBufferDataPointerGet(MpsBufferHandle_t buffer)
{
    uint8_t *data_ptr = NULL;
    if(buffer != NULL) {
        data_ptr = &buffer->buffer[buffer->offset];
    }
    return data_ptr;
}

uint8_t *MpsBufferWriteDirect(MpsBufferHandle_t buffer, uint16_t offset, uint16_t size)
{
    uint8_t *data_ptr = NULL;
    if(buffer->size == 0) {
        buffer->offset = offset;
        buffer->size = size;
        data_ptr = buffer->buffer;
    }
    return data_ptr;
}

/***** Internal functions *****/

MpsResult_t MpsBufferFlush(MpsBufferHandle_t buffer)
{
    if(buffer == NULL) {
        return MPS_RESULT_INVALID_ARG;
    }

    buffer->offset = 0;
    buffer->size = 0;

    for (uint16_t i = 0; i < buffer->capacity; i++) {
        buffer->buffer[i] = 0;
    }

    return MPS_RESULT_OK;
}

void MpsBufferDump(MpsBufferHandle_t buffer)
{
    buffer->buffer[buffer->offset + buffer->size] = '\0';
    printf("\nBuffer:%s", &buffer->buffer[buffer->offset]);
}