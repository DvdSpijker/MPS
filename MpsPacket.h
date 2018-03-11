#ifndef MPS_PACKET_H_
#define MPS_PACKET_H_

#include "MpsResult.h"
#include "MpsHandles.h"
#include <stdint.h>

typedef uint32_t MpsPacketSize_t;
typedef uint32_t MpsPacketDest_t;
typedef uint32_t MpsPacketSrc_t;

struct MpsPacketField {
	uint8_t *data;
	uint8_t size;
	struct MpsPacketField *next;
};

struct MpsPacketFieldList {
	struct MpsPacketField *head;
	struct MpsPacketField *tail;	
};

struct MpsPacket {
    uint32_t id;
	MpsPacketSrc_t src; /* Packet source. */
	MpsPacketDest_t dest; /* Packet destination. */
	MpsPacketSize_t packet_size; /* Total packet size. */
	
	struct MpsPacketFieldList headers_list;
	struct MpsPacketFieldList trailers_list;
	
	uint8_t *payload;
	MpsPacketSize_t payload_size;
	
    uint8_t type;		/* Event, Data or Cmd type. See MPS_PACKET_TYPE_* below. */
    void    *layer_specific; /* Optional pointer for layer specific data-structures. */

    struct MpsPacket *next_in_queue;	/* Used by MpsQueue for linking. */
};

/* Packet-type definitions. */
/* A packet containing an event is emitted by the layer itself. Expected
 * behavior of layers above or below the emitting layer is to pass the packet along
 * to the top or bottom layer. */
#define MPS_PACKET_TYPE_EVENT	0x01 

/* A packet containing data can be modified by layers as it propagates
 * through the stack. Layers can add/remove sections of the packet e.g. a header. */
#define MPS_PACKET_TYPE_DATA	0x02

/* A packet containing a packet can be modified by layers as it propagates
 * through the stack. Layers can add/remove sections of the packet e.g. a header. */
#define MPS_PACKET_TYPE_CMD		0x03

/* Creates a MpsPacket of given size and sets the layer specific implementation. 
 * */
MpsPacketHandle_t MpsPacketCreate(uint8_t type, void *layer_specific);

void MpsPacketDelete(MpsPacketHandle_t packet);

/* Returns a copy of the packet. The copy's layer specific pointer will be NULL. */
MpsPacketHandle_t MpsPacketCopy(MpsPacketHandle_t packet);

/* Formats the packet into the buffer. */
MpsResult_t MpsPacketFormat(MpsPacketHandle_t packet, uint8_t *buf);

/* Parses the buffer into the packet. */
MpsResult_t MpsPacketParse(uint8_t *buf, MpsPacketSize_t size, MpsPacketHandle_t packet);

/* Returns the size of all regions combined. */
MpsPacketSize_t MpsPacketSizeGet(MpsPacketHandle_t packet);

MpsResult_t MpsPacketPayloadSet(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size);

MpsPacketSize_t MpsPacketPayloadGet(MpsPacketHandle_t packet, uint8_t *data);

/* Adds data to the header region after merging a possible previous header with the current body.
 * After the operation the header size will be the added header size.
 * Header and body offsets are adjusted. */
MpsResult_t MpsPacketHeaderAdd(MpsPacketHandle_t packet, uint8_t *data, uint8_t size);

/* Adds data to the trailer region after merging a possible previous trailer with the current body.
 * After the operation the trailer size will be the added trailer size.
 * The trailer offset is adjusted. */
MpsResult_t MpsPacketTrailerAdd(MpsPacketHandle_t packet, uint8_t *data, uint8_t size);

/* Removes a header from the packet. After the
 * operation the header region size will be the removed header size.
 * Header and body offsets are adjusted. */
uint8_t MpsPacketHeaderRemove(MpsPacketHandle_t packet, uint8_t *data);

/* Removes a trailer of defined size from the current body. After the
 * operation the trailer region size will be the removed trailer size.
 * The trailer offset is adjusted. */
uint8_t MpsPacketTrailerRemove(MpsPacketHandle_t packet, uint8_t *data);

uint8_t MpsPacketTypeGet(MpsPacketHandle_t packet);


/* Zeros the packet and resets all regions. */
MpsResult_t MpsPacketFlush(MpsPacketHandle_t packet);

void MpsPacketDump(MpsPacketHandle_t packet);
#endif
