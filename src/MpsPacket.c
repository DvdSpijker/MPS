
#include "MpsPacket.h"
#include "MpsPacketInternal.h"
#include "MpsMem.h"
#include "MpsUtil.h"
#include "MpsLog.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ASSERT_PACKET_HANDLE(packet, res) if(packet == NULL) return res;
#define ASSERT_ARG_NEQ(arg, val, res) if(arg == val) return res;
#define ASSERT_ARG_EQ(arg, val, res) if(arg != val) return res;

static void DumpHex(uint8_t *data, uint32_t size);
static void MpsPacketDestroy(MpsPacketHandle_t packet);
struct MpsPacketField *MpsPacketFieldCreate(uint8_t *data, uint8_t size);
static void MpsPacketFieldDelete(struct MpsPacketField *field);
void MpsPacketFieldListInit(struct MpsPacketFieldList *list);
void MpsPacketFieldListDestroy(struct MpsPacketFieldList *list);
void MpsPacketFieldListAddHead(struct MpsPacketFieldList *list, struct MpsPacketField *field);
void MpsPacketFieldListAddTail(struct MpsPacketFieldList *list, struct MpsPacketField *field);
struct MpsPacketField *MpsPacketFieldListRemoveHead(struct MpsPacketFieldList *list);
struct MpsPacketField *MpsPacketFieldListRemoveTail(struct MpsPacketFieldList *list);
MpsResult_t MpsPacketFieldListCopy(struct MpsPacketFieldList *list_to_copy, struct MpsPacketFieldList *list_copy);
struct MpsPacketField *MpsPacketFieldCopy(struct MpsPacketField *field_to_copy);

MpsPacketHandle_t MpsPacketCreate(uint8_t type, void *layer_specific)
{

    struct MpsPacket * packet = (struct MpsPacket *)MpsMemAlloc(sizeof(struct MpsPacket));

    if(packet != NULL) {
		MpsPacketFieldListInit(&packet->headers_list);
		MpsPacketFieldListInit(&packet->trailers_list);
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
	MpsPacketFieldListDestroy(&packet->headers_list);
	MpsPacketFieldListDestroy(&packet->trailers_list);
	if(packet->payload != NULL) {
		MpsMemFree((void **)&packet->payload);
	}
	
    MpsPacketDestroy(packet);

    return; /* Return ok here. */
}

MpsPacketHandle_t MpsPacketCopy(MpsPacketHandle_t packet)
{
	if(packet == NULL) {
		return NULL;
	}

	MpsResult_t result = MPS_RESULT_ERROR;
	MpsPacketHandle_t packet_cpy = MpsMemAlloc(sizeof(struct MpsPacket));
	
	if(packet_cpy != NULL) {
		result = MpsPacketFieldListCopy(&packet->headers_list, &packet_cpy->headers_list);
		if(result != MPS_RESULT_OK) {
			MpsPacketDestroy(packet_cpy);
			goto exit;
		}	
		result = MpsPacketFieldListCopy(&packet->trailers_list, &packet_cpy->trailers_list);
		if(result != MPS_RESULT_OK) {
			MpsPacketFieldListDestroy(&packet_cpy->headers_list);
			MpsPacketDestroy(packet_cpy);
			goto exit;
		}
		result = MpsPacketPayloadAdd(packet_cpy, packet->payload, packet->payload_size);
		if(result != MPS_RESULT_OK) {
			MpsPacketFieldListDestroy(&packet_cpy->headers_list);
			MpsPacketFieldListDestroy(&packet_cpy->trailers_list);
			MpsPacketDestroy(packet_cpy);
			goto exit;
		}		
		packet_cpy->payload_size = packet->payload_size;
		packet_cpy->dest = packet->dest;
		packet_cpy->src = packet->src;
		packet_cpy->id = packet->id;
		packet_cpy->layer_specific = NULL;
		packet_cpy->next_in_queue = NULL;
		packet_cpy->type = packet->type;
	}

exit:	
	return packet_cpy;
}

MpsPacketSize_t MpsPacketSizeGet(MpsPacketHandle_t packet)
{
	if(packet == NULL) {
		return 0;
	}

    return (packet->packet_size);
}

MpsResult_t MpsPacketSerialize(MpsPacketHandle_t packet, uint8_t *buf, MpsPacketSize_t *size)
{
	ASSERT_PACKET_HANDLE(packet, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(buf, NULL, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(size, NULL, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(*size, 0, MPS_RESULT_INVALID_ARG);

	MpsPacketSize_t offset = 0;
	struct MpsPacketField *field = NULL;
	uint8_t tmp_byte = 0;
	uint8_t list_size = 0;

	MPS_LOG("Serialize packet");
	/* Loop through the header(s). 
	 * First remove the header from its list, 
	 * copy its data,
	 * add its back to the tail. This way
	 * the packet structure remains intact
	 * to support multiple calls of Format on the
	 * same packet. */
	list_size = packet->headers_list.size;
	MPS_LOG("Headers: %u", list_size);
	while(list_size > 0){
		MPS_LOG("Removing header");
		field = MpsPacketFieldListRemoveHead(&packet->headers_list);
		if(field != NULL) {
			MPS_LOG("Size: %u", field->size);
			MPS_LOG("Data:");
			DumpHex(field->data, field->size);

			memcpy(&buf[offset], field->data, field->size);
			offset += field->size;

			MPS_LOG("Buffer (%u):", offset);
			DumpHex(buf, offset);

			MpsPacketFieldListAddTail(&packet->headers_list, field);
		}
		list_size--;
	}
	
	MPS_LOG("Payload size: %u", packet->payload_size);

	tmp_byte = PACKET_PAYLOAD_MARKER;
	memcpy(&buf[offset], &tmp_byte, sizeof(PACKET_PAYLOAD_MARKER));
	offset+=sizeof(PACKET_PAYLOAD_MARKER);
	memcpy(&buf[offset], &packet->payload_size, sizeof(MpsPacketSize_t));
	offset+=sizeof(MpsPacketSize_t);

	MPS_LOG("Buffer (%u):", offset);
	DumpHex(buf, offset);

	/* If payload size > 0 copy the payload data to the buffer. */
	if(packet->payload_size > 0) {
		MPS_LOG("Data:");
		DumpHex(packet->payload, packet->payload_size);

		memcpy(&buf[offset], packet->payload, packet->payload_size);
		offset += packet->payload_size;

		MPS_LOG("Buffer (%u):", offset);
		DumpHex(buf, offset);

	} else { /* Only add: |payload maker|0 (size)|. */
		MPS_LOG("No payload to copy");
	}

	/* Loop through the trailer(s), copy their data. */
	list_size = packet->trailers_list.size;
	MPS_LOG("Trailers: %u", list_size);
	while(list_size > 0){
		MPS_LOG("Removing trailer");
		field = MpsPacketFieldListRemoveHead(&packet->trailers_list);
		if(field != NULL) {
			MPS_LOG("Size: %u", field->size);
			MPS_LOG("Data:");
			DumpHex(field->data, field->size);

			memcpy(&buf[offset], field->data, field->size);
			offset += field->size;

			MPS_LOG("Buffer (%u):", offset);
			DumpHex(buf, offset);

			MpsPacketFieldListAddTail(&packet->trailers_list, field);
		}
		list_size--;
	};

	*size = offset;

	MPS_LOG("Serialized packet size: %u", offset);
	MPS_LOG("Data:");
	DumpHex(buf, offset);
}

MpsResult_t MpsPacketDeserialize(uint8_t *buf, MpsPacketSize_t size, MpsPacketHandle_t packet)
{
	ASSERT_PACKET_HANDLE(packet, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(buf, NULL, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(size, 0, MPS_RESULT_INVALID_ARG);

	MPS_LOG("Deserialize packet");

	MpsResult_t result = MPS_RESULT_OK;
	
	/* Loop through the list of headers and copy the data (containing |size n|byte 0|...|byte n-1|). */
	struct MpsPacketField *field = NULL;
	MpsPacketSize_t offset = 0;
	uint8_t field_size = 0;
	while(offset < size && buf[offset] != PACKET_PAYLOAD_MARKER) {
		field_size = buf[offset];
		MPS_LOG("Header size: %u", field_size);
		field = MpsPacketFieldCreate(&buf[++offset], field_size);
		offset += field_size; /* Increment offset for the next field or the payload. */
		MPS_LOG("Offset: %u", offset);
		if(field == NULL) {
			result = MPS_RESULT_NO_MEM;
			 break;
		} 
		/* Add created field to the tail of the list of headers. */
		MpsPacketFieldListAddTail(&packet->headers_list, field);
	}
	
	MPS_LOG("Headers: %u", packet->headers_list.size);

	/* Parse the payload. */
	if(result == MPS_RESULT_OK && offset < size) {
		memcpy(&packet->payload_size, &buf[++offset], sizeof(MpsPacketSize_t));
		MPS_LOG("Payload size: %u", packet->payload_size);
		offset += sizeof(MpsPacketSize_t);

		if(packet->payload_size > 0) {
			packet->payload =  (uint8_t *)MpsMemAlloc(packet->payload_size);
			if(packet->payload != NULL) {
				memcpy(packet->payload, &buf[offset], packet->payload_size);
				MPS_LOG("Payload:");
				DumpHex(packet->payload, packet->payload_size);
				offset += packet->payload_size;
				MPS_LOG("Offset: %u", offset);
			}
		}
	}
	
	/* Loop through the list of trailers and copy the data (containing |size n|byte 0|...|byte n-1|). */
	if(result == MPS_RESULT_OK) {
		while(offset < size) {
			field_size = buf[offset];
			MPS_LOG("Trailer size: %u", field_size);
			field = MpsPacketFieldCreate(&buf[++offset], field_size);
			offset += field_size;
			MPS_LOG("Offset: %u", offset);
			if(field == NULL) {
				result = MPS_RESULT_NO_MEM;
				break;
			}
			MpsPacketFieldListAddHead(&packet->trailers_list, field);
		}
	}
	
	MPS_LOG("Trailers: %u", packet->trailers_list.size);

	return result;
}

MpsResult_t MpsPacketPayloadAdd(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size)
{
	ASSERT_PACKET_HANDLE(packet, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(data, NULL, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(size, 0, MPS_RESULT_INVALID_ARG);

	MpsResult_t result = MPS_RESULT_INVALID_ARG;
	
	if(packet != NULL && data != NULL && size != 0) {
		result = MPS_RESULT_INVALID_STATE;

		if(packet->payload == NULL) {
			result = MPS_RESULT_NO_MEM;

			uint8_t *payload_buf = MpsMemAlloc(size);
			if(payload_buf != NULL) {
				result = MPS_RESULT_OK;

				packet->payload = payload_buf;
				memcpy(packet->payload, data, size);
				packet->payload_size = size;
				packet->packet_size += size;
			}
		}
	}
	
	return result;
}

MpsPacketSize_t MpsPacketPayloadRemove(MpsPacketHandle_t packet, uint8_t *data)
{
	if(packet == NULL || data == NULL) {
		return 0;
	}

	MpsPacketSize_t size = 0;
	
	if(packet->payload != NULL) {
		size = packet->payload_size;
		memcpy(data, packet->payload, size);
		MpsMemFree(packet->payload);
		packet->packet_size -= packet->payload_size;
		packet->payload_size = 0;
	}	
	
	return size;
}

MpsResult_t MpsPacketHeaderAdd(MpsPacketHandle_t packet, uint8_t *data, uint8_t size)
{
	ASSERT_PACKET_HANDLE(packet, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(data, NULL, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(size, 0, MPS_RESULT_INVALID_ARG);

    MpsResult_t result = MPS_RESULT_NO_MEM;
	struct MpsPacketField *field = MpsPacketFieldCreate(data, size);
	
	if(field != NULL) {
		MpsPacketFieldListAddHead(&packet->headers_list, field);
		packet->packet_size += field->size;
		result = MPS_RESULT_OK;
	}
	
    return result;
}

MpsResult_t MpsPacketTrailerAdd(MpsPacketHandle_t packet, uint8_t *data, uint8_t size)
{
	ASSERT_PACKET_HANDLE(packet, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(data, NULL, MPS_RESULT_INVALID_ARG);
	ASSERT_ARG_NEQ(size, 0, MPS_RESULT_INVALID_ARG);

    MpsResult_t result = MPS_RESULT_NO_MEM;
    struct MpsPacketField *field = MpsPacketFieldCreate(data, size);
	
    if(field != NULL) {
	    MpsPacketFieldListAddTail(&packet->trailers_list, field);
	    packet->packet_size += field->size;
	    result = MPS_RESULT_OK;
    }
    
    return result;
}

uint8_t MpsPacketHeaderRemove(MpsPacketHandle_t packet, uint8_t *data)
{
	ASSERT_PACKET_HANDLE(packet, 0);
	ASSERT_ARG_NEQ(data, NULL, 0);

	MpsPacketSize_t size = 0;
	struct MpsPacketField *field = MpsPacketFieldListRemoveHead(&packet->headers_list);
	
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
	ASSERT_PACKET_HANDLE(packet, 0);
	ASSERT_ARG_NEQ(data, NULL, 0)

	uint8_t size = 0;
	struct MpsPacketField *field = MpsPacketFieldListRemoveTail(&packet->trailers_list);
	
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
	ASSERT_PACKET_HANDLE(packet, 0);

	uint8_t type = 0x00;
	if(packet != NULL) {
		type = packet->type;
	}
	
	return type;
}

void MpsPacketDump(MpsPacketHandle_t packet)
{
	if(packet == NULL) {
		return;
	}

	printf("[MPS] Packet payload: ");
	DumpHex(packet->payload, packet->payload_size);
}


/***** Internal functions *****/

static void DumpHex(uint8_t *data, uint32_t size)
{
	const uint32_t bytes_per_line = 10;

	for(uint32_t i = 0; i < size; i+=bytes_per_line) {
		for(uint32_t j = 0; j < bytes_per_line && (i + j) < size; j++) {
			printf("0x%02x ", data[i+j]);
		}
		printf("\r\n");
	}
}

static void MpsPacketDestroy(MpsPacketHandle_t packet)
{
	MpsMemFree((void **)&packet);
}

struct MpsPacketField *MpsPacketFieldCreate(uint8_t *data, uint8_t size)
{
	if(size > (0xFF - 2)) {
		return NULL;
	}
	struct MpsPacketField *field = MpsMemAlloc(sizeof(struct MpsPacketField));
	if(field != NULL) {
		field->data = MpsMemAlloc(size+1);
		if(field->data != NULL) {
			field->data[0] = size;
			memcpy(&field->data[PACKET_FIELD_DATA_OFFSET], data, size);
			field->size = size+1;
			field->next = NULL;			
		} else {
			MpsMemFree((void *)field);
		}
	}
	return field;
}

static void MpsPacketFieldDelete(struct MpsPacketField *field)
{
	if(field->data != NULL) {
		MpsMemFree((void *)field->data);
	}
	MpsMemFree((void *)field);
}

struct MpsPacketField *MpsPacketFieldCopy(struct MpsPacketField *field_to_copy)
{
	return MpsPacketFieldCreate(&field_to_copy->data[PACKET_FIELD_DATA_OFFSET], field_to_copy->size);
}

void MpsPacketFieldListInit(struct MpsPacketFieldList *list)
{
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
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

MpsResult_t MpsPacketFieldListCopy(struct MpsPacketFieldList *list_to_copy, struct MpsPacketFieldList *list_copy)
{
	MpsResult_t result = MPS_RESULT_OK;
	struct MpsPacketField *field_copy = NULL;
	struct MpsPacketField *field_to_copy = list_to_copy->head;
	while(field_to_copy != NULL && field_copy != NULL) {
		field_copy = MpsPacketFieldCopy(field_to_copy);
		if(field_copy != NULL) {
			MpsPacketFieldListAddTail(list_copy, field_copy);
		}  else {
			result = MPS_RESULT_NO_MEM;
		}
	}
	list_copy->size = list_to_copy->size;

	return result;	
}

void MpsPacketFieldListAddHead(struct MpsPacketFieldList *list, struct MpsPacketField *field)
{
	if(list->head == NULL) {
		list->head = list->tail = field;
	} else {
		field->next = list->head;
		list->head = field;
	}
	list->size++;
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
	list->size++;
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
	list->size--;

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
	list->size--;

	return field;
}

