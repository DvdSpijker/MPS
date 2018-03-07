
#include "MpsPacket.h"
#include "MpsMalloc.h"
#include "MpsUtil.h"

#include <stdlib.h>
#include <stdio.h>

#define PACKET_FIELD_DATA_OFFSET sizeof(uint8_t)
#define PACKET_PAYLOAD_SIZE_OFFSET sizeof(uint8_t)
#define PACKET_PAYLOAD_DATA_OFFSET sizeof(MpsPacketSize_t) + PACKET_PAYLOAD_SIZE_OFFSET
#define PACKET_PAYLOAD_MARKER 0xFF

struct MpsPacketField *MpsPacketFieldCreate(uint8_t *data, uint8_t size);
void MpsPacketFieldListInit(struct MpsPacketFieldList *list);
void MpsPacketFieldLisDestroy(struct MpsPacketFieldList *list);
void MpsPacketFieldListAddHead(struct MpsPacketFieldList *list, struct MpsPacketField *field);
void MpsPacketFieldListAddTail(struct MpsPacketFieldList *list, struct MpsPacketField *field);
struct MpsPacketField *MpsPacketFieldListRemoveHead(struct MpsPacketFieldList *list);
struct MpsPacketField *MpsPacketFieldListRemoveTail(struct MpsPacketFieldList *list);

MpsPacketHandle_t MpsPacketCreate(uint8_t *buffer, MpsPacketSize_t size, uint8_t type, void *layer_specific)
{
    if(size == 0) {
        return NULL;
    }

    struct MpsPacket * packet = (struct MpsPacket *)MpsMalloc(sizeof(struct MpsPacket));

    if(packet != NULL) {
		MpsPacketFieldListInit(packet->headers_list);
		MpsPacketFieldListInit(packet->trailers_list);
		packet->payload = NULL;
		packet->payload_size = 0;
		packet->packet_size = 0;
		packet->dest = packet->src = 0;
        packet->layer_specific = layer_specific;
        packet->next_in_queue = NULL;
        packet->id = MpsUtilRunTimeHashGenerate();
		packet->type = type; 
    }

    return packet;
}


void MpsPacketDelete(MpsPacketHandle_t packet)
{
    if(packet == NULL) {
        return; /* Return error here. */
    }
	MpsPacketFieldListDestroy(packet->headers_list);
	MpsPacketFieldListDestroy(packet->trailers_list);
	if(packet->payload != NULL) {
		MpsFree(packet->payload);
	}
    MpsFree(packet);

    return; /* Return ok here. */
}

MpsPacketHandle_t MpsPacketResize(MpsPacketHandle_t packet, MpsPacketSize_t new_size)
{	
}

MpsPacketSize_t MpsPacketSizeGet(MpsPacketHandle_t packet)
{
    return (packet->packet_size);
}

void MpsPacketFormat(MpsPacketHandle_t packet, uint8_t *buf)
{
	MpsPacketSize_t offset = 0;
	struct MpsPacketField *field = NULL;
	
	do {
		field = MpsPacketFieldListRemoveHead(packet->headers_list);
		memcpy(&buf[offset], field->data, field->size);
		offset += field->size;
		if(field != NULL) {
			MpsPacketFieldDelete(field);
		}
	} while(field != NULL);
		
	memcpy(&buf[offset], packet->payload, packet->payload_size);
	offset += packet->payload_size;
	
	do {
		field = MpsPacketFieldListRemoveHead(packet->trailers_list);
		memcpy(&buf[offset], field->data, field->size);
		offset += field->size;
		if(field != NULL) {
			MpsPacketFieldDelete(field);
		}
	} while(field != NULL);
}

MpsResult_t MpsPacketParse(uint8_t *buf, MpsPacketSize_t size, MpsPacketHandle_t packet)
{
	MpsResult_t result = MPS_RESULT_OK;
	
	struct MpsPacketField *field = NULL;
	MpsPacketSize_t offset = 0;
	uint8_t field_size = 0;
	while(offset < size && buf[offset] != PACKET_PAYLOAD_MARKER) {
		field_size = buf[offset] - 1;
		field = MpsPacketFieldCreate(&buf[++offset], field_size);
		offset += field_size;
		if(field == NULL) {
			 result = MPS_RESULT_NO_MEM
			 break;
		} 
		MpsPacketFieldListAddTail(packet->headers_list, field);
	} 
	
	if(result == MPS_RESULT_OK && offset < size) {
		memcpy(&packet->payload_size, &buf[++offset], sizeof(MpsPacketSize_t));
		packet->payload_size -= sizeof(MpsPacketSize_t) - sizeof(uint8_t);
		offset += sizeof(MpsPacketSize_t);
		memcpy(&packet->payload[offset], buf, packet->payload_size);
		offset += packet->payload_size
	}

	while(offset < size) {
		field_size = buf[offset] - 1;
		field = MpsPacketFieldCreate(&buf[++offset], field_size);
		offset += field_size;
		if(field == NULL) {
			result = MPS_RESULT_NO_MEM
			break;
		}
		MpsPacketFieldListAddHead(packet->trailers_list, field);
	}	
	
	return result;
}

MpsResult_t MpsPacketPayloadSet(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size)
{
	MpsResult_t result = MPS_RESULT_NO_MEM;
	uint8_t *payload_buf = MpsMalloc(size+sizeof(MpsPacketSize_t));
	if(payload_buf != NULL) {
		packet->payload = payload_buf;
		packet->payload[0] = PACKET_PAYLOAD_MARKER;
		memcpy(&packet->payload[PACKET_PAYLOAD_SIZE_OFFSET], &size, sizeof(MpsPacketSize_t));
		memcpy(&packet->payload[PACKET_PAYLOAD_DATA_OFFSET], data, size);
		packet->payload_size = size + sizeof(MpsPacketSize_t) + sizeof(uint8_t);
		packet->packet_size += packet->payload_size;
	}
}

MpsPacketSize_t MpsPacketPayloadGet(MpsPacketHandle_t packet, uint8_t *data)
{
	MpsPacketSize_t size = 0;
	
	if(packet->payload != NULL) {
		size = packet->payload_size - sizeof(MpsPacketSize_t);
		memcpy(data, &packet->payload[PACKET_PAYLOAD_DATA_OFFSET], size);
		MpsFree(packet->payload);
		packet->packet_size -= packet->payload_size;
		packet->payload_size = 0;
	}	
	
	return size;
}

MpsResult_t MpsPacketHeaderAdd(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size)
{
    MpsResult_t result = MPS_RESULT_NO_MEM;
	struct MpsPacketField *field = MpsPacketFieldCreate(data, size);
	if(field != NULL) {
		MpsPacketFieldListAddHead(packet->headers_list, field);
		packet->packet_size += field->size;
		result = MPS_RESULT_OK;
	}
	
    return result;
}

MpsResult_t MpsPacketTrailerAdd(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size)
{
    MpsResult_t result = MPS_RESULT_NO_MEM;
    
    struct MpsPacketField *field = MpsPacketFieldCreate(data, size);
    if(field != NULL) {
	    MpsPacketFieldListAddTail(packet->trailers_list, field);
	    packet->packet_size += field->size;
	    result = MPS_RESULT_OK;
    }
    
    return result;
}

MpsPacketSize_t MpsPacketHeaderRemove(MpsPacketHandle_t packet, uint8_t *data)
{
	MpsPacketSize_t size = 0;
	struct MpsPacketField *field = MpsPacketFieldListRemoveHead(packet->headers_list);
	if(field != NULL) {
		size = field->size - 1;
		memcpy(data, &field->data[PACKET_FIELD_DATA_OFFSET], size);
		packet->packet_size -= field->size;
		MpsPacketFieldDelete(field);
	}
	
	return size;
}

MpsPacketSize_t MpsPacketTrailerRemove(MpsPacketHandle_t packet, uint8_t *data)
{
	MpsPacketSize_t size = 0;
	struct MpsPacketField *field = MpsPacketFieldListRemoveTail(packet->trailers_list);
	if(field != NULL) {
		size = field->size - 1;
		memcpy(data, &field->data[PACKET_FIELD_DATA_OFFSET], size);
		packet->packet_size -= field->size;
		MpsPacketFieldDelete(field);
	}
	
	return size;
}



uint8_t MpsPacketTypeGet(MpsPacketHandle_t packet)
{
	uint8_t type = 0x00;
	if(packet != NULL) {
		type = packet->type;
	}
	
	return type;
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

struct MpsPacketField *MpsPacketFieldCreate(uint8_t *data, uint8_t size)
{
	if(size > (0xFF - 2)) {
		return NULL;
	}
	struct MpsPacketField *field = MpsMalloc(sizeof(struct MpsPacketField));
	if(field != NULL) {
		field->data = MpsMalloc(size+1);
		if(field->data != NULL) {
			field->data[0] = size+1;
			memcpy(&field->data[PACKET_FIELD_DATA_OFFSET], data, size);
			field->size = size+1;
			field->next = NULL;			
		} else {
			MpsFree((void *)field);
		}
	}
	return field;
}

void MpsPacketFieldDelete(struct MpsPacketField *field)
{
	if(field->data != NULL) {
		MpsFree((void *)field->data);
	}
	MpsFree((void *)field);
}

void MpsPacketFieldListInit(struct MpsPacketFieldList *list)
{
	list->head = NULL;
	list->tail = NULL;
}

void MpsPacketFieldListDestroy(struct MpsPacketFieldList *list)
{
	struct MpsPacketField *field = NULL;
	do {
		field = MpsPacketFieldListRemoveTail(list);
		if(field != NULL) {
			MpsPacketFieldDelete(field);
		}
	} while(field != NULL);
}

void MpsPacketFieldListAddHead(struct MpsPacketFieldList *list, struct MpsPacketField *field)
{
	if(list->head == NULL) {
		list->head = list->tail = field;
	} else {
		field->next = list->head;
		list->head = field;
	}	
}

void MpsPacketFieldListAddTail(struct MpsPacketFieldList *list, struct MpsPacketField *field)
{
	if(list->head == NULL) {
		list->head = list->tail = field;
	} else {
		list->tail->next = field;
		list->tail = field;
	}
	field->next = NULL;
}

struct MpsPacketField *MpsPacketFieldListRemoveHead(struct MpsPacketFieldList *list)
{
	struct MpsPacketField *field = NULL;
	
	if(list->head == NULL) {
		return NULL;
	} else if(list->head->next == NULL) {
		field = list->head;
		list->head = list->tail = NULL;
	} else {
		field = list->head;
		list->head = field->next;
		field->next = NULL;
	}
	return field;
}

struct MpsPacketField *MpsPacketFieldListRemoveTail(struct MpsPacketFieldList *list)
{
	struct MpsPacketField *field = NULL;
	
	if(list->head == NULL) {
		return NULL;
	} else if(list->head->next == NULL) {
		field = list->head;
		list->head = list->tail = NULL;
	} else {
		field = list->head;
		list->head = field->next;
		field->next = NULL;
	}
	return field;
}

