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
	uint16_t size;
	struct MpsPacketField *next;
};

struct MpsPacket {
    uint32_t id;
	
	MpsPacketSrc_t src; /* Packet source. */
	MpsPacketDest_t dest; /* Packet destination. */
	
    uint8_t *buffer;		/* Pointer to the packet. */
    uint8_t is_external;	/* packet is declared externally i.e. not allocated by MPS. */
    uint8_t type;		/* Event or Data type. See MPS_PACKET_TYPE_* below. */
    MpsPacketSize_t capacity;	/* packet capacity in bytes. */
    MpsPacketSize_t size;		/* Current packet size in bytes. */
    uint16_t offset;		/* Current offset with respect to the packet index 0. */
    void    *layer_specific; /* Optional pointer for layer specific data-structures. */

    struct MpsPacket *next_in_queue;	/* Used by MpsQueue for linking. */
};

/* Packet-type definitions. */
/* A packet containing an event is emitted by the layer itself. Expected
 * behavior of layers above or below the emitting layer is to pass the packet along
 * to the top or bottom layer. */
#define MPS_PACKET_TYPE_EVENT	0x01 

/* A packet containing a packet can be modified by layers as it propagates
 * through the stack. Layers can add/remove sections of the packet e.g. a header. */
#define MPS_PACKET_TYPE_DATA	0x02

/* Creates a MspPacket of given size and sets the layer specific implementation. */
MpsPacketHandle_t MpsPacketCreate(uint8_t *buffer, MpsPacketSize_t size, uint8_t type, void *layer_specific);

void MpsPacketDelete(MpsPacketHandle_t packet);

/* Resizes the packet while retaining the data. */
MpsPacketHandle_t MpsPacketResize(MpsPacketHandle_t packet, MpsPacketSize_t new_size);

/* TODO: Returns a copy of the packet. */
MpsPacketHandle_t MpsPacketCopy(MpsPacketHandle_t packet);

/* Returns the size of all regions combined. */
MpsPacketSize_t MpsPacketSizeGet(MpsPacketHandle_t packet);

/* Adds data to the header region after merging a possible previous header with the current body.
 * After the operation the header size will be the added header size.
 * Header and body offsets are adjusted. */
MpsResult_t MpsPacketHeaderAdd(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size);


void MpsPacketBodyAdd(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size);

/* Adds data to the trailer region after merging a possible previous trailer with the current body.
 * After the operation the trailer size will be the added trailer size.
 * The trailer offset is adjusted. */
MpsResult_t MpsPacketTrailerAdd(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size);

/* Adds data to the specified region. Copying from 'data' starts at the 'data_offset'
 * and is copied to the body at the 'body_offset'. */
void MpsPacketAddBodyWithOffset(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t data_offset, MpsPacketSize_t size, MpsPacketSize_t body_offset);

/* Removes a header of defined size from the current body. After the
 * operation the header region size will be the removed header size.
 * Header and body offsets are adjusted. */
MpsResult_t MpsPacketHeaderRemove(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size);

/* Removes a body of defined size from the current body. After the
 * operation the body region size will decreased by the removed body size.
 * The body offset is adjusted.
 * Note: If 0 is passed as the size, the entire body will be removed. */
MpsResult_t MpsPacketBodyRemove(MpsPacketHandle_t packet, uint8_t *data);

/* Removes a trailer of defined size from the current body. After the
 * operation the trailer region size will be the removed trailer size.
 * The trailer offset is adjusted. */
MpsResult_t MpsPacketTrailerRemove(MpsPacketHandle_t packet, uint8_t *data, MpsPacketSize_t size);

uint8_t MpsPacketTypeGet(MpsPacketHandle_t packet);

/* Write data directly into the packet */
uint8_t *MpsPacketWriteDirect(MpsPacketHandle_t packet, uint16_t offset, MpsPacketSize_t size);

/* Returns a pointer to the data in the packet with the current offset taken into account. */
uint8_t *MpsPacketPointerGet(MpsPacketHandle_t packet);

/* Zeros the packet and resets all regions. */
MpsResult_t MpsPacketFlush(MpsPacketHandle_t packet);

void MpsPacketDump(MpsPacketHandle_t packet);
#endif
