/*
 * MpsPacketInternal.h
 *
 *  Created on: Apr 13, 2019
 *      Author: dorus
 */

#ifndef MPS_PACKET_INTERNAL_H_
#define MPS_PACKET_INTERNAL_H_

#define PACKET_FIELD_DATA_OFFSET sizeof(uint8_t)
#define PACKET_PAYLOAD_SIZE_OFFSET sizeof(uint8_t)
#define PACKET_PAYLOAD_DATA_OFFSET sizeof(MpsPacketSize_t) + PACKET_PAYLOAD_SIZE_OFFSET
#define PACKET_PAYLOAD_MARKER_OFFSET 0
#define PACKET_PAYLOAD_MARKER (uint8_t)0xFF

#endif /* MPS_PACKET_INTERNAL_H_ */
