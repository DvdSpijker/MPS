
#include "MpsPacket.h"
#include "MpsMalloc.h"
#include "MpsUtil.h"

#include <stdlib.h>
#include <stdio.h>

MpsPacketHandle_t MpsPacketCreate(uint8_t *buffer, MpsPacketSize_t size, uint8_t type, void *layer_specific)
{
    if(size == 0) {
        return NULL;
    }

    struct MpsPacket * packet = (struct MpsPacket *)MpsMalloc(sizeof(struct MpsPacket));

    if(packet != NULL) {
        if(buffer == NULL) {
            packet->is_external = 0;
            packet->buffer = (uint8_t *)MpsMalloc(sizeof(uint8_t) * size);
        } else {
            packet->is_external = 1;
            packet->buffer = buffer;
        }
        if(packet->buffer != NULL) {
            packet->capacity = size;
            packet->layer_specific = layer_specific;
            packet->next_in_queue = NULL;
            packet->id = MpsUtilRunTimeHashGenerate();
            packet->size = 0;
            packet->offset = 0;
			packet->type = type;
        } else {
            MpsFree((void *)packet);
            packet = NULL;
        }
    }

    return packet;
}


void MpsPacketDelete(MpsPacketHandle_t packet)
{
    if(packet == NULL) {
        return; /* Return error here. */
    }

    if(packet->is_external == 0) {
        MpsFree(packet->buffer);
    }
    MpsFree(packet);

    return; /* Return ok here. */
}

MpsPacketHandle_t MpsPacketResize(MpsPacketHandle_t packet, MpsPacketSize_t new_size)
{	
	MpsPacketHandle_t new_packet = NULL;
	if(!packet->is_external) { /* Cannot resize if packet is external. */
		/* Create a new packet, copy data and stats, then delete old packet. */
		new_packet = MpsPacketCreate(NULL, new_size, packet->layer_specific);
		if(new_packet != NULL) {
			for(uint16_t i = packet->offset; i < packet->size, i++) {
				new_packet->buffer[i] = packet->buffer[i];
			}
			new_packet->offset = packet->offset;
			new_packet->size = packet->size;	
			MpsPacketDelete(packet);
		}
	}
	return new_packet;
}

MpsPacketSize_t MpsPacketSizeGet(MpsPacketHandle_t packet)
{
    return (packet->size);
}


MpsResult_t  MpsPacketHeaderAdd(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size)
{

    MpsResult_t result = MPS_RESULT_OK;

    if( (size > (packet->capacity - packet->size)) || (size > (packet->offset + 1)) ) {
        result = MPS_RESULT_NO_SPACE;
    }

    if(result == MPS_RESULT_OK) {

        packet->offset -= size;
        packet->size += size;

        MpsPacketSize_t i;
        for(i = 0; i < size; i++) {
            packet->buffer[i+packet->offset] = data[i];
        }
    }

    return result;
}

MpsResult_t MpsPacketTrailerAdd(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size)
{
    MpsResult_t result = MPS_RESULT_OK;

    if( (size > (packet->capacity - packet->size)) || (size > packet->capacity - (packet->offset + packet->size))) {
        result = MPS_RESULT_NO_SPACE;
    }

    if(result == MPS_RESULT_OK) {
        MpsPacketSize_t offset = packet->size + packet->offset;
        packet->size += size;

        MpsPacketSize_t i;
        for(i = 0; i < size; i++) {
            packet->buffer[i+offset] = data[i];
        }
    }

    return result;
}


void MpsPacketAddBodyWithOffset(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t data_offset, MpsPacketSize_t size, MpsPacketSize_t body_offset)
{
    MpsPacketSize_t i;
    for(i = 0; i < size; i++) {
        packet->buffer[i+body_offset] = data[i + data_offset];
    }

    packet->size = size;
    packet->offset = body_offset;

    //if( (packet->size > 0) ) {
    //packet->size += size - (body_offset + packet->size);
    //} else {
    //packet->size = size;
    //}
//
    //if( (packet->offset > body_offset) || (packet->offset == 0) ) {
    //packet->offset = body_offset;
    //}

}




MpsResult_t MpsPacketHeaderRemove(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size)
{

    MpsResult_t result = MPS_RESULT_OK;

    /* Check input. */
    if(packet == NULL || data == NULL || size == 0) {
        result = MPS_RESULT_INVALID_ARG;
    }

    /* Check size boundaries. */
    if(size > packet->size) {
        result = MPS_RESULT_NO_SPACE;
    }


    if(result == MPS_RESULT_OK) {
        MpsPacketSize_t i;
        for (i = 0; (i < size); i++) {
            data[i] = packet->buffer[packet->offset + i];
        }
        packet->offset += size;
        packet->size -= size;

        result = MPS_RESULT_OK;
    }

    return result;
}

MpsResult_t MpsPacketTrailerRemove(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size)
{
    MpsResult_t result = MPS_RESULT_OK;

    if(packet == NULL || data == NULL || size == 0) {
        result = MPS_RESULT_INVALID_ARG;
    }

    if(size > packet->size) {
        result = MPS_RESULT_NO_SPACE;
    }
    if(result == MPS_RESULT_OK) {

        MpsPacketSize_t offset = packet->offset + (packet->size - size);
        packet->size -= size;
        MpsPacketSize_t i;
        for (i = 0; (i < size); i++) {
            data[i] = packet->buffer[offset + i];
        }
        result = MPS_RESULT_OK;
    }

    return result;

}

MpsResult_t MpsPacketBodyRemove(MpsPacketHandle_t packet, uint8_t *data)
{
    MpsResult_t result = MPS_RESULT_OK;

    if(packet == NULL || data == NULL) {
        result = MPS_RESULT_INVALID_ARG;
    }

    /* Extract remaining body. */
    if(result == MPS_RESULT_OK) {

        MpsPacketSize_t i;
        for (i = 0; (i < packet->size); i++) {
            data[i] = packet->buffer[packet->offset + i];
        }

        packet->offset = 0;
        packet->size = 0;

        result = MPS_RESULT_OK;
    }

    return result;
}

uint8_t MpsPacketTypeGet(MpsPacketHandle_t packet)
{
	uint8_t type = 0x00;
	if(packet != NULL) {
		type = packet->type;
	}
	
	return type;
}


uint8_t *MpsPacketPointerGet(MpsPacketHandle_t packet)
{
    uint8_t *data_ptr = NULL;
    if(packet != NULL) {
        data_ptr = &packet->buffer[packet->offset];
    }
    return data_ptr;
}

uint8_t *MpsPacketWriteDirect(MpsPacketHandle_t packet, MpsPacketSize_t offset, MpsPacketSize_t size)
{
    uint8_t *data_ptr = NULL;
    if(packet->size == 0) {
        packet->offset = offset;
        packet->size = size;
        data_ptr = &packet->buffer[packet->offset];
    }
    return data_ptr;
}

/***** Internal functions *****/

MpsResult_t MpsPacketFlush(MpsPacketHandle_t packet)
{
    if(packet == NULL) {
        return MPS_RESULT_INVALID_ARG;
    }

    packet->offset = 0;
    packet->size = 0;
    MpsPacketSize_t i = 0;
    for (; i < packet->capacity; i++) {
        packet->buffer[i] = 0;
    }

    return MPS_RESULT_OK;
}

void MpsPacketDump(MpsPacketHandle_t packet)
{
    packet->buffer[packet->offset + packet->size] = '\0';
    printf("\npacket:%s", &packet->buffer[packet->offset]);
}
