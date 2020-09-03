/*
 * iot_packet.h
 *
 *  Created on: Aug 5, 2020
 *      Author: Austin McElroy
 */

#ifndef INCLUDE_IoT_PACKET_H_
#define INCLUDE_IoT_PACKET_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "iot_checksum.h"
#ifdef __TI_ARM__
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define PACKET_SIZE 128
#define DATA_SIZE 101
#define HEADER_SIZE 16

enum IoT_PACKET_PAYLOAD {
    IoT_NONE,
    IoT_UINT8,
    IoT_STRING,
};

enum IoT_PACKET_ADD_BYTE_CODES {
    IoT_PACKET_BUFFER_OVERFLOW,
    IoT_PACKET_CHECKSUM_ERROR,
    IoT_PACKET_COMPLETE,
    IoT_PACKET_PARSING,
    IoT_PACKET_SYNC_ERROR_DETECTED,
};

/**
    * The packet structure for the MDI device.
    * payload: indicates the type of data coming across the bus.
    * data_size: Size of the data array. In deterministic mode, this
    * should be set to PACKET_SIZE, otherwise it can be set to however
    * much memory was malloced.
    * checksum: CRC16 checksum. This should be computed by the sender
    * and validated by the receiver.
    * current_byte_data: Convenience counter for keeping track of how
    * much data has been put into the data[].
    * current_byte: Convenience counter for keeping track of how many
    * bytes have been written to an IoT_I2C_PACKET. This also keeps track
    * of bytes send across the I2C bus like payload, data_size, and checksum.
    *
    *
    * @sa IoT_I2C_PAYLOAD, PACKET_SIZE, IoT_I2C_Generate_Checksum, IoT_I2C_Validate_Packet, crc16.h
    */
struct IoT_PACKET {
    enum IoT_PACKET_PAYLOAD payload;
    char header[16];
    uint16_t checksum;
    uint32_t current_packet;
    uint32_t total_packets;
    uint8_t data[DATA_SIZE];
    uint32_t current_byte_data;
    uint32_t current_byte;
};

/**
 * Generates a CRC16 checksum. If store is NOT 0, the CRC is stored in the packet
 * struct.
 *
 * @param packet
 * @param store
 * @return Crc16 generated
 *
 * @sa crc16.h
 */
uint16_t IoT_Packet_Generate_Checksum(struct IoT_PACKET *packet, uint8_t store);

/**
 * Validates the packet by:
 * 1. Run IoT_I2C_Generate_Checksum without storing the data
 * 2. Compare the results from 1. with the stored packet->checksum value
 *
 * @param packet
 * @return 1 if the results match, 0 otherwise
 */
uint8_t IoT_Packet_Validate(struct IoT_PACKET *packet);

/**
 * Computes the size of transmitted information for an IoT_I2C_PACKET. This may seem
 * redundant, but in DETERMINISTIC mode, the size of the packet is fixed, in an OS
 * embedded environment, this is not always the case. For example, sizeof(IoT_I2C_PACKET)
 * would return a size with a fixed byte array of PACKET_SIZE in an embedded, non-OS environment,
 * but would return a much, much smaller value if some sort of OS with memory management is used.
 *
 * @return
 */
uint32_t IoT_Packet_Sizeof(struct IoT_PACKET *packet);


/**
 * Adds a byte to the packet. This also takes into account the additional
 * information, such as checksum, datasize, and payload. This is meant to be
 * used with bytes that are coming off the I2C bus. It returns one of the
 * values from struct IoT_PACKET_ADD_BYTE_CODES, with IoT_PACKET_PARSING and
 * IoT_PACKET_ADD_BYTE_CODES hopefully being the response codes.
 *
 * @param packet
 * @param byte
 * @return struct _PACKET_ADD_BYTE_CODES
 * @sa IoT_PACKET_ADD_BYTE_CODES
 */
int IoT_Packet_Add_Byte(struct IoT_PACKET *packet, uint8_t byte);

/**
 * Returns the current byte of the packet. IoT_PACKET has an internal counter that is updated each time
 * this is called. Overflow checking occurs based on the amount of data in the data_size array vs. the
 * internal counter, so overflows shouldn't occur. Call
 *
 * @param packet
 * @return
 */
uint8_t IoT_Packet_Get_Byte(struct IoT_PACKET *packet);

void IoT_Packet_Reset(struct IoT_PACKET *packet);

void IoT_Packet_Complete_Callback(struct IoT_PACKET *packet) __attribute__((weak));

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_IoT_PACKET_H_ */
