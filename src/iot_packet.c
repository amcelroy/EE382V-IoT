/*
 * iot_packet.c
 *
 *  Created on: Aug 5, 2020
 *      Author: mcelr
 */

#include "iot_packet.h"

uint8_t current_packet_buffer[sizeof(uint32_t)];

uint8_t total_packet_buffer[sizeof(uint32_t)];

uint8_t checksum_buffer[sizeof(uint16_t)];

uint8_t internal_zero_byte_counter = 0;

void IoT_Packet_Reset(struct IoT_PACKET *packet){
    packet->checksum = 0;
    packet->current_byte = 0;
    packet->current_byte_data = 0;
    packet->current_packet = 0;
    packet->total_packets = 0;
    memset(packet->header, 0, HEADER_SIZE);
    memset(packet->data, 0, DATA_SIZE)
    internal_zero_byte_counter = 0;
}

uint8_t IoT_Packet_Get_Byte(struct IoT_PACKET *packet){
    uint8_t retval = 0;

    switch(packet->current_byte){
    case 0:
        retval = packet->payload;
        packet->current_byte_data = 0;
        break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
        retval = packet->header[packet->current_byte - 1];
        break;

    case 17:
        memcpy(checksum_buffer, &packet->checksum, sizeof(uint16_t));
        retval = checksum_buffer[0];
        break;

    case 18:
        retval = checksum_buffer[1];
        break;

    case 19:
        memcpy(current_packet_buffer, &packet->current_packet, sizeof(uint32_t));
        retval = current_packet_buffer[0];
        break;

    case 20:
        retval = current_packet_buffer[1];
        break;

    case 21:
        retval = current_packet_buffer[2];
        break;

    case 22:
        retval = current_packet_buffer[3];
        break;
        
    case 23:
        memcpy(total_packet_buffer, &packet->total_packets, sizeof(uint32_t));
        retval = total_packet_buffer[0];
        break;

    case 24:
        retval = total_packet_buffer[1];
        break;

    case 25:
        retval = total_packet_buffer[2];
        break;

    case 26:
        retval = total_packet_buffer[3];
        break;

    default:
        if (packet->current_byte_data >= DATA_SIZE) {
            return 0;
        }
        retval = packet->data[packet->current_byte_data];
        packet->current_byte_data += 1;
        break;
    }

    packet->current_byte += 1;

    return retval;
}

int IoT_Packet_Add_Byte(struct IoT_PACKET *packet, uint8_t byte) {
    switch (packet->current_byte) {
    case 0:
        packet->payload = byte;
        packet->current_byte_data = 0;
        break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
        packet->header[packet->current_byte - 1] = byte;
        break;

    case 17:
        checksum_buffer[0] = byte;
        break;

    case 18:
        checksum_buffer[1] = byte;
        memcpy(&packet->checksum, checksum_buffer, sizeof(uint16_t));
        break;

    case 19:
        current_packet_buffer[0] = byte;
        break;

    case 20:
        current_packet_buffer[1] = byte;
        break;

    case 21:
        current_packet_buffer[2] = byte;
        break;

    case 22:
        current_packet_buffer[3] = byte;
        memcpy(&packet->current_packet, current_packet_buffer, sizeof(uint32_t));
        break;

    case 23:
        total_packet_buffer[0] = byte;
        break;

    case 24:
        total_packet_buffer[1] = byte;
        break;

    case 25:
        total_packet_buffer[2] = byte;
        break;

    case 26:
        total_packet_buffer[3] = byte;
        memcpy(&packet->total_packets, total_packet_buffer, sizeof(uint32_t));
        break;

    default:
        packet->data[packet->current_byte_data] = byte;
        packet->current_byte_data += 1;

        if (packet->current_byte_data >= packet->data_size) {
            if(IoT_Packet_Validate(packet)){
                if(IoT_Packet_Complete_Callback != 0){
                    IoT_Packet_Complete_Callback(packet);
                }
                return IoT_PACKET_COMPLETE;
            }else{
                return IoT_PACKET_CHECKSUM_ERROR;
            }
        }
        break;
    }

    packet->current_byte += 1;

    return IoT_PACKET_PARSING;
}

uint32_t IoT_Packet_Sizeof(struct IoT_PACKET *data){
    //This is an optimized version, one could do sizeof(IoT_I2C_PACKET), but this would include excess 0 in the data[]
    return PACKET_SIZE;
}

uint16_t IoT_Packet_Generate_Checksum(struct IoT_PACKET *packet, bool store){
    uint16_t tmp_crc = IoT_Checksum_CRC16((char*)packet->data, packet->data_size);
    if(store){
        packet->checksum = tmp_crc;
    }
    return tmp_crc;
}

uint8_t IoT_Packet_Validate(struct IoT_PACKET *packet){
    uint16_t tmp_crc = IoT_Packet_Generate_Checksum(packet, 0);
    if(tmp_crc == packet->checksum){
        return 1;
    }else{
        return 0;
    }
}



