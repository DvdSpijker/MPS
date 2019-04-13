/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/
 
/* Test framework */
#include "unity.h"
 
/* UUT */
#include "MpsPacket.h"
#include "MpsPacketInternal.h"
#include "MpsMem.h"
#include "MpsUtil.h"
#include "MpsPort.h"

/* Dependencies */


/* Other */

 
/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/
 
/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/
 
/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/
 
 
/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
 
 
/*******************************************************************************
 *    SETUP, TEARDOWN
 ******************************************************************************/
 
void setUp(void)
{
}
 
void tearDown(void)
{
}
 
/*******************************************************************************
 *    TESTS
 ******************************************************************************/

void test_MpsPacketCreate_successful(void)
{
	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(packet, NULL, "Packet is NULL.");
}

void test_MpsPacketPayloadAdd_invalid_packet(void)
{
	MpsResult_t res = MPS_RESULT_ERROR;
	uint8_t payload[3] = {0,1,2};
	res = MpsPacketPayloadAdd(NULL, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_INVALID_ARG, "Result incorrect on invalid packet.");
}

void test_MpsPacketPayloadAdd_invalid_payload(void)
{
	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(packet, NULL, "Packet is NULL.");

	MpsResult_t res = MPS_RESULT_ERROR;
	uint8_t payload[3] = {0,1,2};
	res = MpsPacketPayloadAdd(packet, NULL, sizeof(payload));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_INVALID_ARG, "Result incorrect on invalid data.");
}

void test_MpsPacketPayloadAdd_invalid_size(void)
{
	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(packet, NULL, "Packet is NULL.");

	MpsResult_t res = MPS_RESULT_ERROR;
	uint8_t payload[3] = {0,1,2};
	res = MpsPacketPayloadAdd(packet, payload, 0);
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_INVALID_ARG, "Result incorrect on invalid size.");
}

void test_MpsPacketPayloadAdd_valid(void)
{
	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(packet, NULL, "Packet is NULL.");

	MpsResult_t res = MPS_RESULT_ERROR;
	uint8_t payload[3] = {0,1,2};
	res = MpsPacketPayloadAdd(packet, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid payload.");
	TEST_ASSERT_EQUAL_MESSAGE(packet->payload_size, sizeof(payload), "Payload size incorrect");
	TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(payload, packet->payload, sizeof(payload), "Payload data incorrect.");
}

void test_MpsPacketPayloadAdd_payload_already_set(void)
{
	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(packet, NULL, "Packet is NULL.");

	MpsResult_t res = MPS_RESULT_ERROR;
	uint8_t payload[3] = {0,1,2};
	res = MpsPacketPayloadAdd(packet, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid payload.");

	res = MpsPacketPayloadAdd(packet, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_INVALID_STATE, "Result incorrect on already set payload.");

}

void test_MpsPacketSerialize_complete_packet(void)
{
	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(packet, NULL, "Packet is NULL.");

	MpsResult_t res = MPS_RESULT_ERROR;
	uint8_t payload[3] = {'5','6','7'};
	res = MpsPacketPayloadAdd(packet, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid payload.");

	uint8_t header_0[2] = {'3', '4'};
	res = MpsPacketHeaderAdd(packet, header_0, sizeof(header_0));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid header.");

	uint8_t header_1[2] = {'1', '2'};
	res = MpsPacketHeaderAdd(packet, header_1, sizeof(header_1));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid header.");

	uint8_t trailer[2] = {'8', '9'};
	res = MpsPacketTrailerAdd(packet, trailer, sizeof(trailer));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid trailer.");

	uint8_t ser_buf[20] = {0};
	MpsPacketSize_t size = 20;

	MpsPacketSerialize(packet, ser_buf, &size);

	uint8_t exp_data[18] = {2, '1', '2', 2, '3', '4', PACKET_PAYLOAD_MARKER, 3, 0, 0, 0, '5', '6', '7', 2, '8', '9'};
	TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(exp_data, ser_buf, sizeof(exp_data), "Serialized packet data incorrect.");
}

void test_MpsPacketSerialize_complete_packet_twice(void)
{
	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(packet, NULL, "Packet is NULL.");

	MpsResult_t res = MPS_RESULT_ERROR;
	uint8_t payload[3] = {'5','6','7'};
	res = MpsPacketPayloadAdd(packet, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid payload.");

	uint8_t header_0[2] = {'3', '4'};
	res = MpsPacketHeaderAdd(packet, header_0, sizeof(header_0));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid header.");

	uint8_t header_1[2] = {'1', '2'};
	res = MpsPacketHeaderAdd(packet, header_1, sizeof(header_1));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid header.");

	uint8_t trailer[2] = {'8', '9'};
	res = MpsPacketTrailerAdd(packet, trailer, sizeof(trailer));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid trailer.");

	uint8_t ser_buf[20] = {0};
	MpsPacketSize_t size = 20;

	MpsPacketSerialize(packet, ser_buf, &size);

	uint8_t exp_data[18] = {2, '1', '2', 2, '3', '4', PACKET_PAYLOAD_MARKER, 3, 0, 0, 0, '5', '6', '7', 2, '8', '9'};
	TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(exp_data, ser_buf, sizeof(exp_data), "Serialized packet data incorrect.");

	uint8_t ser_buf_sec[20] = {0};
	MpsPacketSerialize(packet, ser_buf_sec, &size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(ser_buf_sec, ser_buf, sizeof(ser_buf), "First serialization contains different data from the second.");
}

void test_MpsPacketSerialize_no_payload(void)
{
	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(packet, NULL, "Packet is NULL.");

	MpsResult_t res = MPS_RESULT_ERROR;

	uint8_t header_0[2] = {'3', '4'};
	res = MpsPacketHeaderAdd(packet, header_0, sizeof(header_0));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid header.");

	uint8_t header_1[2] = {'1', '2'};
	res = MpsPacketHeaderAdd(packet, header_1, sizeof(header_1));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid header.");

	uint8_t trailer[2] = {'8', '9'};
	res = MpsPacketTrailerAdd(packet, trailer, sizeof(trailer));
	TEST_ASSERT_EQUAL_MESSAGE(res, MPS_RESULT_OK, "Result incorrect on valid trailer.");

	uint8_t ser_buf[20] = {0};
	MpsPacketSize_t size = 20;

	MpsPacketSerialize(packet, ser_buf, &size);

	uint8_t exp_data[18] = {2, '1', '2', 2, '3', '4', PACKET_PAYLOAD_MARKER, 0, 0, 0, 0, 2, '8', '9'};
	TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(exp_data, ser_buf, sizeof(exp_data), "Serialized packet data incorrect.");
}

void test_MpsPacketDeserialize_no_payload(void)
{
	uint8_t exp_data[14] = {2, '1', '2', 2, '3', '4', PACKET_PAYLOAD_MARKER, 0, 0, 0, 0, 2, '8', '9'};

	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	MpsPacketDeserialize(exp_data, sizeof(exp_data), packet);


	uint8_t ser_buf[20] = {0};
	MpsPacketSize_t size = 20;

	MpsPacketSerialize(packet, ser_buf, &size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(exp_data, ser_buf, size, "Serialized packet data incorrect.");
}

void test_MpsPacketDeserialize_with_payload(void)
{
	uint8_t exp_data[17] = {2, '1', '2', 2, '3', '4', PACKET_PAYLOAD_MARKER, 3, 0, 0, 0, '5', '6', '7', 2, '8', '9'};

	MpsPacketHandle_t packet = MpsPacketCreate(MPS_PACKET_TYPE_DATA, NULL);
	MpsPacketDeserialize(exp_data, sizeof(exp_data), packet);

	TEST_ASSERT_EQUAL(3, packet->payload_size);
	TEST_ASSERT_NOT_EQUAL(NULL, packet->payload);

	uint8_t ser_buf[30] = {0};
	MpsPacketSize_t size = 30;

	MpsPacketSerialize(packet, ser_buf, &size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(exp_data, ser_buf, size, "Serialized packet data incorrect.");
}
