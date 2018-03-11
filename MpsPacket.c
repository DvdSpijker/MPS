
#include "MpsPacket.h"
#include "MpsMalloc.h"
#include "MpsUtil.h"

#include <stdlib.h>
#include <stdio.h>

#define PACKET_FIELD_DATA_OFFSET sizeof(uint8_t)
#define PACKET_PAYLOAD_SIZE_OFFSET sizeof(uint8_t)
#define PACKET_PAYLOAD_DATA_OFFSET sizeof(MpsPacketSize_t) + PACKET_PAYLOAD_SIZE_OFFSET
#define PACKET_PAYLOAD_MARKER (uint8_t)0xFF

struct MpsPacketField *MpsPacketFieldCreate(uint8_t *data, uint8_t size);
void MpsPacketFieldListInit(struct MpsPacketFieldList *list);
void MpsPacketFieldLisDestroy(struct MpsPacketFieldList *list);
void MpsPacketFieldListAddHead(struct MpsPacketFieldList *list, struct MpsPacketField *field);
void MpsPacketFieldListAddTail(struct MpsPacketFieldList *list, struct MpsPacketField *field);
struct MpsPacketField *MpsPacketFieldListRemoveHead(struct MpsPacketFieldList *list);
struct MpsPacketField *MpsPacketFieldListRemoveTail(struct MpsPacketFieldList *list);

MpsPacketHandle_t MpsPacketCreate(uint8_t type, void *layer_specific)
{

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

MpsPacketHandle_t MpsPacketCopy(MpsPacketHandle_t packet)
{
	MpsPacketHandle_t packet_cpy = MpsMalloc(sizeof(struct MpsPacket));
	if(packet_cpy != NULL) {
		
	}
	
	return packet_cpy;
}

MpsPacketSize_t MpsPacketSizeGet(MpsPacketHandle_t packet)
{
    return (packet->packet_size);
}

void MpsPacketFormat(MpsPacketHandle_t packet, uint8_t *buf)
{
	MpsPacketSize_t offset = 0;
	struct MpsPacketField *field = NULL;
	
	/* Loop through the header(s). 
	 * First remove the header from its list, 
	 * copy its data,
	 * add its back to the tail. This way
	 * the packet structure remains intact
	 * to support multiple calls of Format on the
	 * same packet. */
	do {
		field = MpsPacketFieldListRemoveHead(&packet->headers_list);
		if(field != NULL) {
			memcpy(&buf[offset], field->data, field->size);
			offset += field->size;
			MpsPacketFieldListAddTail(&packet->headers_list, field);			
		}
	} while(field != NULL);
	
	/* If payload size > 0 copy the payload data to the buffer. */
	if(packet->payload_size > 0) {
		memcpy(&buf[offset], packet->payload, packet->payload_size);
		offset += packet->payload_size;		
	} else { /* Only add: |payload maker|0 (size)|. */
		memcpy(&buf[offset], PACKET_PAYLOAD_MARKER, sizeof(PACKET_PAYLOAD_MARKER));	
		offset+=sizeof(PACKET_PAYLOAD_MARKER);
		memcpy(&buf[offset], packet->payload_size, sizeof(MpsPacketSize_t));
		offset+=sizeof(MpsPacketSize_t);
	}
	
	/* Loop through the trailer(s), copy their data. */
	do {
		field = MpsPacketFieldListRemoveHead(&packet->trailers_list);
			if(field != NULL) {
				memcpy(&buf[offset], field->data, field->size);
				offset += field->size;
				MpsPacketFieldListAddTail(&packet->trailers_list, field);
		}
	} while(field != NULL);
}

MpsResult_t MpsPacketParse(uint8_t *buf, MpsPacketSize_t size, MpsPacketHandle_t packet)
{
	MpsResult_t result = MPS_RESULT_OK;
	
	/* Loop through the list of headers and copy the data (containing |size n|byte 0|...|byte n-1|). */
	struct MpsPacketField *field = NULL;
	MpsPacketSize_t offset = 0;
	uint8_t field_size = 0;
	while(offset < size && buf[offset] != PACKET_PAYLOAD_MARKER) {
		field_size = buf[offset] - 1;
		field = MpsPacketFieldCreate(&buf[++offset], field_size);
		offset += field_size; /* Increment offset for the next field or the payload. */
		if(field == NULL) {
			 result = MPS_RESULT_NO_MEM
			 break;
		} 
		/* Add created field to the tail of the list of headers. */
		MpsPacketFieldListAddTail(packet->headers_list, field); 
	} 
	
	/* Parse the payload. */
	if(result == MPS_RESULT_OK && offset < size) {
		memcpy(&packet->payload_size, &buf[++offset], sizeof(MpsPacketSize_t));
		if(packet->payload_size > 0) {
			packet->payload_size -= sizeof(MpsPacketSize_t) - sizeof(uint8_t);
			offset += sizeof(MpsPacketSize_t);
			memcpy(&packet->payload[offset], buf, packet->payload_size);
			offset += packet->payload_size
		} else {
			offset++; /* Increment offset to parse the trailer(s). */
		}
	}
	
	/* Loop through the list of trailers and copy the data (containing |size n|byte 0|...|byte n-1|). */
	if(result == MPS_RESULT_OK) {
		while(offset < size) {
			field_size = buf[offset] - 1;
			if(field_size > 0) {
				field = MpsPacketFieldCreate(&buf[++offset], field_size);
				offset += field_size;
				if(field == NULL) {
					result = MPS_RESULT_NO_MEM
					break;
				}
				MpsPacketFieldListAddHead(packet->trailers_list, field);
			} else {
				break;
			}
		}
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

MpsResult_t MpsPacketHeaderAdd(MpsPacketHandle_t packet, uint8_t *data, uint8_t size)
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

MpsResult_t MpsPacketTrailerAdd(MpsPacketHandle_t packet, uint8_t *data, uint8_t size)
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

uint8_t MpsPacketHeaderRemove(MpsPacketHandle_t packet, uint8_t *data)
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

uint8_t MpsPacketTrailerRemove(MpsPacketHandle_t packet, uint8_t *data)
{
	uint8_t size = 0;
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

void MpsPacketDump(MpsPacketHandle_t packet)
{
	
}


/***** Internal functions *****/


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

